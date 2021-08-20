#ifndef ESPFLASHER_H
#define ESPFLASHER_H

#include <QObject>
#include <QProcess>

class EspFlasher : public QObject
{
    Q_OBJECT
public:
    explicit EspFlasher(QObject *parent = nullptr);
    ~EspFlasher();


    QString getPythonDir() const;
    void setPythonDir(const QString &value);

    QString getPythonCmd() const;
    void setPythonCmd(const QString &value);

    bool checkPython(QString path="", QString cmd="");
    bool checkPythonCmds(QStringList pathList, QStringList cmds);
    bool checkDefaultPython();

    bool checkPip();
    bool checkEsptool();

    QString getErrorMsg() const;

    QString getIdentifier() const;
    void setIdentifier(const QString &value);


signals:
    void pythonDirSet(QString &pythonDir);
    void pythonCmdSet(QString &pythonCmd);
    void debugMsg(QString &msg);
    void commandFinished(int exitCode);
//    void commandFinished(int exitCode, QProcess::ExitStatus exitStatus);


public slots:
    void installPip();
    void installEsptool();
    void flashFirmware(QString port, QString filepath);

private slots:
    void readCommandData();

private:
    QString pythonDir;
    QString pythonCmd = "python";
    QString pipCmd;
    QString sys;
    QString identifier;
    QString cmdOutput, cmdError;
    QProcess *process;

    QString errorMsg;

    QString python_install_msg = "Download and install the latest version from <a href='https://www.python.org/downloads/'>here</a> and/ or update the python settings.";

    bool checkPythonVersion(QString versionString) const;
    QString getShell();
    QString getPythonFilePath(QString path, QString cmd);
    void runCommand(QString command);
};

#endif // ESPFLASHER_H
