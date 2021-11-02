#include "clouduploader.h"
#include <QtCore>
#include "defaultSettings.h"

CloudUploader::CloudUploader(QObject *parent) :
    QObject(parent),
    process(new QProcess(this)),
    timeoutTimer(new QTimer(this)),
    syncTimer(new QTimer(this))
{
    #if defined(Q_OS_LINUX)
    uploaderCMD = "/home/pingu/Documents/eNoseUploader/bin/Debug/net5.0/eNoseUploaderCL";
    #elif defined(Q_OS_WIN)
    uploaderCMD = "eNoseUploaderCL.exe";
    #else
    errorMsg = "OS not supported.";
    #endif

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
    process->waitForFinished(60000);
    if (process->state() != QProcess::NotRunning)
        process->kill();
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

    if (runCmd(uploaderCMD + " -u '" + username + "' -p '" + password + "'", true))
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
    runCmd(uploaderCMD + " -s \"" + filepath + "\"");
}

void CloudUploader::readStandardOutput()
{
    QString standardOutput = process->readAllStandardOutput();
    cmdOutput += standardOutput;
    qDebug() << "->" << standardOutput;

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
    qDebug() << "->Error: " << errorOutput;
}

void CloudUploader::stopCmd()
{
    qDebug() << "Command timeout. Killing '" << currentCmd << "'";
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
//    // check exit of last command
//    if (currentCmd == "")
//        loggedIn = exitCode == 0 && exitStatus == QProcess::ExitStatus::NormalExit;
//    else if (loginRegex.match(currentCmd).hasMatch())
//    {
//        loggedIn = exitCode == 0 && exitStatus == QProcess::ExitStatus::NormalExit;
//        // TODO: give login feedback
//    }
    qDebug() << "Exit code: " << exitCode;

    // stop timeout timer
    if (timeoutTimer->isActive())
        timeoutTimer->stop();

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
    arguments << "-c";
    #elif defined(Q_OS_WIN)
    shell = "cmd.exe";
    arguments << "/c";
    #endif

    arguments << command;

    QString msg = "$ " + shell + " " + arguments.join(" ");
    qDebug() << msg;

    process->start(shell, arguments);
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
                                                    "Education",
                                                    "Food",
                                                    "Health care",
                                                    "Manufacturing",
                                                    "Pharmaceutical",
                                                    "Safety",
                                                    "Security",
                                                    "Services",
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
    arguments << "-u " << quoteWrap(email);
    arguments << "-p " << quoteWrap(password);
    arguments << "--firstName " << quoteWrap(firstName);
    arguments << "--lastName " << quoteWrap(lastName);
    arguments << "--company " << quoteWrap(company);
    arguments << "--industry " << quoteWrap(industry);

    return arguments.join(" ");
}

QString SignUpInfo::quoteWrap(QString string)
{
    return ('"' + string + '"');
}
