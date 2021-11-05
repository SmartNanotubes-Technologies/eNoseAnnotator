#include "clouduploader.h"
#include <QtCore>
#include <QMessageBox>
#include "defaultSettings.h"

CloudUploader::CloudUploader(QObject *parent) :
    QObject(parent),
    process(new QProcess(this)),
    timeoutTimer(new QTimer(this)),
    syncTimer(new QTimer(this))
{
    QFileInfo uploaderCMDFileInfo;
    #if defined(Q_OS_LINUX)
    uploaderCMDFileInfo.setFile("/home/pingu/Documents/eNoseUploader/bin/Debug/net5.0/eNoseUploaderCL");
    #elif defined(Q_OS_WIN)
    uploaderCMDFileInfo.setFile(QCoreApplication::applicationDirPath() + "/eNoseUploader/eNoseUploaderCL.exe");
    #else
    errorMsg = "OS not supported.";
    #endif

    uploaderCMD = SignUpInfo::quoteWrap(uploaderCMDFileInfo.absoluteFilePath());

    connect(process, &QProcess::readyReadStandardOutput, this, &CloudUploader::readStandardOutput);
    connect(process, &QProcess::readyReadStandardError, this, &CloudUploader::readStandardError);
    connect(process , QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &CloudUploader::onCommandFinished);
    connect(timeoutTimer, &QTimer::timeout, this, &CloudUploader::stopCmd);
    connect(syncTimer, &QTimer::timeout, this, &CloudUploader::requestSync);

    checkLoggedIn();

    timeoutTimer->setSingleShot(true);
    syncTimer->setSingleShot(true);
    requestSync();
}

CloudUploader::~CloudUploader()
{
    // wait for process to finish and cmdQueue to be empty
    while (!process->waitForFinished(60000) && !cmdQueue.isEmpty())
        continue;
}

void CloudUploader::checkLoggedIn()
{
    loggedIn = runCmd(uploaderCMD, true);
}

bool CloudUploader::isLoggedIn()
{
    return loggedIn;
}

bool CloudUploader::loginUser(QString username, QString password)
{
    Q_ASSERT(!loggedIn);

    if (runCmd(uploaderCMD + " -u " + SignUpInfo:: quoteWrap(username) + " -p " + SignUpInfo:: quoteWrap(password), true))
    {
        loggedIn = true;
        requestSync();
        return true;
    }
    return false;
}

bool CloudUploader::logoutUser()
{
    Q_ASSERT(loggedIn);

    if (runCmd(uploaderCMD + " --logout", true))
    {
        loggedIn = false;
        return true;
    }
    return false;
}

bool CloudUploader::registerUser(SignUpInfo signUpInfo)
{
    return runCmd(uploaderCMD + " -r " + signUpInfo.getClArgumentString(), true);
}

void CloudUploader::syncFile(QString filepath)
{
    Q_ASSERT(loggedIn && "User has to be logged in in order to sync data!");
    runCmd(uploaderCMD + " -s " + SignUpInfo:: quoteWrap(filepath));
}

void CloudUploader::readStandardOutput()
{   
    QString standardOutput = process->readAllStandardOutput();

    cmdOutput += standardOutput;
    QString msg = "->" + standardOutput;
    qDebug() << msg;
    log += msg;
    emit commadOutputReceived(msg);

    for (QString line : standardOutput.split("\n"))
    {
        if (line.startsWith("Logged in as "))
            email = line.mid(QString("Logged in as ").size());
        if (line == "AccountNameInUse: name already in use")
            errorMsg = "Account name is already in use.";
    }
}

void CloudUploader::readStandardError()
{
    QString errorOutput = process->readAllStandardError();
    cmdError += errorOutput;
    QString msg  = "->Error: " + errorOutput;
    qDebug() << msg;
    log += msg;
    emit commadOutputReceived(msg);
}

void CloudUploader::stopCmd()
{
    qDebug() << "Command timeout. Killing " << SignUpInfo:: quoteWrap(currentCmd);
    QString msg = "xx Timeout. Killing current command. xx";
    emit commadOutputReceived(msg);
    if (process->state() != QProcess::NotRunning)
    {
        process->kill();
    }
}

void CloudUploader::requestSync()
{
    if (!loggedIn)
        return;

    QString syncCommand = uploaderCMD + " --sync";
    if (!cmdQueue.contains(syncCommand))
        runCmd(syncCommand);

    syncTimer->start(UPLOADER_CMD_SYNC_PERIOD);
}

QString CloudUploader::getLog() const
{
    return log;
}

int CloudUploader::getCmdQueueSize() const
{
    return cmdQueue.size();
}


QString CloudUploader::getErrorMsg() const
{
    return errorMsg;
}

QString CloudUploader::getEmail() const
{
    return email;
}

void CloudUploader::onCommandFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString msg = "Exit code: " + QString::number(exitCode);
    if (exitCode != 0)
        msg = "Command finished with error!";
    else
        msg = "Command finished successfully!";

    emit commandExecuted(msg);
    qDebug() << "Exit code: " << exitCode;

    // stop timeout timer
    if (timeoutTimer->isActive())
        timeoutTimer->stop();
    QEventLoop loop;
    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();

    // start next command
    if (!cmdQueue.isEmpty())
        runCmd(cmdQueue.takeFirst(), false);
}

bool CloudUploader::runCmd(QString command, bool waitForFinished)
{
    // TODO block upload ui when cmd is running
    if (waitForFinished)
        waitForCmdQueueFinished();

    if (process->state() != QProcess::NotRunning)
    {
        if (!cmdQueue.contains(command))
            cmdQueue.push_back(command);
        return false;
    }

    // clear saved command output
    cmdOutput.clear();
    cmdError.clear();

    QString shell;
    QStringList arguments;

    #if defined(Q_OS_LINUX)
    shell = "/bin/sh";
    arguments << "-c" << command; // Linux: "-c" and command separate elements of list
    process->setProgram(shell);
    process->setArguments(arguments);
    #elif defined(Q_OS_WIN)
    shell = "cmd.exe";
    arguments << "/c" << SignUpInfo::quoteWrap(command);
    process->setProgram(shell);
    process->setNativeArguments(arguments.join(" "));
    #endif

    QString msg = "$ " + process->program() + " " + process->arguments().join(" ");
    qDebug() << msg;
    log += msg;
    emit commandExecuted(msg);

    process->start();
    currentCmd = command;
    timeoutTimer->start(UPLOADER_CMD_TIMEOUT);

    if (waitForFinished)
    {
        if (process->waitForFinished())
        {
            return process->exitCode() == 0;
        }
    }
    return false;
}

void CloudUploader::waitForCmdQueueFinished()
{
    while(process->state() != QProcess::NotRunning || !cmdQueue.isEmpty())
    {
        // non-blocking sleep
        QEventLoop loop;
        QTimer::singleShot(500, &loop, SLOT(quit()));
        loop.exec();
    }
}

QStringList SignUpInfo::industryTypes =QStringList{ "Agriculture",
                                                    "Automotive",
                                                    "Chemistry/Materials",
                                                    "Education",
                                                    "Food",
                                                    "Health care",
                                                    "Home Appliances",
                                                    "Manufacturing",
                                                    "Pharmaceutical",
                                                    "Safety",
                                                    "Security",
                                                    "Services",
                                                    "Smart Living",
                                                    "Other"};

SignUpInfo::SignUpInfo(QString email, QString password, QString firstName, QString lastName, QString company, QString industry)
{
    this->email = email;
    this->password = password;
    this->firstName = firstName;
    this->lastName = lastName;
    this->company = company;
    this->industry = industry;
}

QString SignUpInfo::getEmail() const
{
    return email;
}

QString SignUpInfo::getPassword() const
{
    return password;
}

QString SignUpInfo::getCompany() const
{
    return company;
}

QString SignUpInfo::getIndustry() const
{
    return industry;
}

QString SignUpInfo::getFirstName() const
{
    return firstName;
}

QString SignUpInfo::getLastName() const
{
    return lastName;
}

QString SignUpInfo::getClArgumentString()
{
    QStringList arguments;
    arguments << "-u " <<  quoteWrap(email);
    arguments << "-p " <<  quoteWrap(password);
    arguments << "--firstName " <<  quoteWrap(firstName);
    arguments << "--lastName " <<  quoteWrap(lastName);
    arguments << "--company " <<  quoteWrap(company);
    arguments << "--industry " <<  quoteWrap(industry);

    return arguments.join(" ");
}

QString SignUpInfo::quoteWrap(QString string)
{
    return ("\"" + string + "\"");
}
