#include "espflasher.h"
#include <QRegularExpression>
#include <QtCore>

EspFlasher::EspFlasher(QObject *parent) :
    QObject(parent),
    process(new QProcess(this))
{
    connect(process, &QProcess::readyReadStandardOutput, this, &EspFlasher::readCommandData);
    connect(process, &QProcess::readyReadStandardError, this, &EspFlasher::readCommandData);
    connect(process , SIGNAL(finished(int)), this, SIGNAL(commandFinished(int)));

    #if defined(Q_OS_LINUX)
    sys = "linux";
    #elif defined(Q_OS_WIN)
    sys = "win";
    #else
    errorMsg = "OS not supported.";
    #endif
}

EspFlasher::~EspFlasher()
{

}

QString EspFlasher::getPythonDir() const
{
    return pythonDir;
}

void EspFlasher::setPythonDir(const QString &value)
{
    pythonDir = value;
}

QString EspFlasher::getPythonCmd() const
{
    return pythonCmd;
}

void EspFlasher::setPythonCmd(const QString &value)
{
    pythonCmd = value;
}

bool EspFlasher::checkPython(QString path, QString cmd)
{
    Q_ASSERT(process->atEnd());

    if (path == "")
        path = pythonDir;
    if (cmd == "")
        cmd = pythonCmd;

    QString shell;
    QString command;

    runCommand(getPythonFilePath(path, cmd) + " --version");
    process->waitForFinished();
    int exitCode = process->exitCode();

    // handle weird bug where output of successfull execution is sent to the standardError channel
    if (exitCode == 0 && cmdOutput.isEmpty())
    {
        cmdOutput = cmdError;
        cmdError.clear();
    }

    // command executed successfully:
    // extract and check python version (>= 2.7 || >= 3.4 )
    if (exitCode == 0)
    {
        QString versionString = cmdOutput;

        if (checkPythonVersion(versionString))
        {
            QString msg = "-> Valid python version detected.";
            emit debugMsg(msg);
            return true;
        }
        else
        {
            QString msg = "-> Invalid python version detected.";
            emit debugMsg(msg);
            errorMsg = versionString + " was found.\nPython version >= 2.7 or 3.4 is needed.\n" + python_install_msg;
            return false;
        }
    }

    // command executed with error:
    // no python found
    errorMsg = "No Python Interpreter was found with the current settings.\nPython version >= 2.7 or 3.4 is needed.\n" + python_install_msg;
    return false;
}

QString EspFlasher::getErrorMsg() const
{
    return errorMsg;
}

bool EspFlasher::checkPythonVersion(QString versionString) const
{
    QRegularExpression re("Python (\\d+)\\.(\\d+)");

    auto match = re.match(versionString);

    if (match.hasMatch())
    {
        int majorVersion = match.captured(1).toInt();
        int minorVersion = match.captured(2).toInt();

        return ((majorVersion == 2 && minorVersion >= 7) || (majorVersion == 3 && minorVersion >= 4));
    }

    return false;
}

bool EspFlasher::checkPythonCmds(QStringList pathList, QStringList cmds)
{
    for (auto pythonDirPath : pathList)
    {
        for (QString cmd : cmds)
        {
            if (checkPython(pythonDirPath, cmd))
            {
                pythonDir = pythonDirPath;
                pythonCmd = cmd;
                return true;
            }
        }
    }

    return false;
}

bool EspFlasher::checkPip()
{
    runCommand(pythonCmd + " -m pip install --upgrade pip");
    process->waitForFinished(60000);
    runCommand(pythonCmd + " -m pip --version");
    process->waitForFinished();

    int exitCode = process->exitCode();

    // handle weird bug where output of successfull execution is sent to the standardError channel
    if (exitCode == 0 && cmdOutput.isEmpty())
    {
        cmdOutput = cmdError;
        cmdError.clear();
    }

    if (exitCode == 0)
    {
        QString msg = "-> Pip is installed.";
        emit debugMsg(msg);
        return true;
    }

    errorMsg = "Pip is not installed!";
    return false;
}

bool EspFlasher::checkEsptool()
{
    runCommand(pythonCmd + " -m esptool version");
    process->waitForFinished();

    int exitCode = process->exitCode();

    if (exitCode == 0)
    {
        QString msg = "-> esptool is installed.";
        emit debugMsg(msg);
        return true;
    }

    errorMsg = "esptool is not installed!";
    return false;
}

void EspFlasher::installPip()
{
    runCommand(pythonCmd + " -m ensurepip --upgrade");
}

void EspFlasher::installEsptool()
{
    runCommand(pythonCmd + " -m pip install esptool");
}

void EspFlasher::flashFirmware(QString port, QString filepath)
{
    if (sys == "linux")
        port = "/dev/" + port;

    runCommand(pythonCmd + " -m esptool --port " + port + " write_flash 0x10000 " + filepath);
}

QString EspFlasher::getIdentifier() const
{
    return identifier;
}

void EspFlasher::setIdentifier(const QString &value)
{
    identifier = value;
}

bool EspFlasher::checkDefaultPython()
{
    QString msg = "Checking for default python installation...";
    emit debugMsg(msg);

    QStringList cmds, pythonDirs;


    if (sys == "win")
    {
        cmds << "python.exe" << "python" << "py";

        QStringList defaultDirs;
        defaultDirs << "" << "C:/Program Files" << "C:/Program Files (x86)" << QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);

        for (auto dirPath : defaultDirs)
        {
            QDir dir = QDir(dirPath);
            pythonDirs << dir.entryList(QStringList() << "Python *", QDir::Dirs);
        }
    }
    else if (sys == "linux")
    {
        cmds << "python3" << "python";
        pythonDirs << "" << "/usr/bin";
    }

    if (checkPythonCmds(pythonDirs, cmds))
        return true;

    return false;
}

QString EspFlasher::getShell()
{
    Q_ASSERT(((sys == "linux") || (sys == "win")) && "Unsupported os");
    if (sys == "linux")
        return "/bin/sh";
    else if (sys == "win")
        return  "cmd.exe";

    return "";
}

void EspFlasher::runCommand(QString command)
{
    Q_ASSERT(process->atEnd() && "Previous command was not finished!");

    // clear saved command output
    cmdOutput.clear();
    cmdError.clear();

    QString shell = getShell();
    QStringList arguments;

    if(sys == "linux")
        arguments << "-c";
    else if (sys == "win")
        arguments << "/c";
    arguments << command;

    QString msg = "$ " + shell + " " + arguments.join(" ");
    emit debugMsg(msg);

    process->start(shell, arguments);

}

QString EspFlasher::getPythonFilePath(QString path, QString cmd)
{
    if (path == "")
        return cmd;
    else
        return path + "/" + cmd;
}

void EspFlasher::readCommandData()
{
    QString standardOutput = process->readAllStandardOutput();
    QString errorOutput = process->readAllStandardError();

    // save output
    cmdOutput += standardOutput;
    cmdError += errorOutput;

    // emit message
    QString msg;
    msg += standardOutput;
    msg += errorOutput;
    emit debugMsg(msg);
}

