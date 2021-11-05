#ifndef CLOUDUPLOADER_H
#define CLOUDUPLOADER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QRegularExpression>
#include <QMutex>

class SignUpInfo
{
public:
    SignUpInfo(QString email, QString password, QString firstName, QString lastName, QString company, QString industry);

    static QString quoteWrap(QString string);

    QString getEmail() const;

    QString getPassword() const;

    QString getCompany() const;

    QString getIndustry() const;

    static QStringList industryTypes;

    QString getFirstName() const;

    QString getLastName() const;
    QString getClArgumentString();

private:
    QString email, password, firstName, lastName, company, industry;


};

class CloudUploader: public QObject
{
    Q_OBJECT
public:
    CloudUploader(QObject *parent=nullptr);
    ~CloudUploader();

    void checkLoggedIn();
    QString getLogin();
    bool isLoggedIn();
    bool loginUser(QString username, QString password);
    bool logoutUser();
    bool registerUser(SignUpInfo signUpInfo);

    void syncFile(QString filepath);

    QString getEmail() const;

    QString getErrorMsg() const;

    QString getLog() const;

    int getCmdQueueSize() const;

signals:
    void commandExecuted(QString &msg);
    void commadOutputReceived(QString &msg);

private slots:
    void onCommandFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void readStandardOutput();
    void readStandardError();
    void stopCmd();
    void requestSync();

private:
    QProcess *process;
    QString uploaderCMD, currentCmd, cmdOutput, cmdError;
    QList<QString> cmdQueue;
    bool loggedIn = false;
    QString errorMsg, email;
    QString log;
    QRegularExpression loginRegex = QRegularExpression("^-u * -p *$");

    bool runCmd(QString cmd, bool waitForFinished=false);
    void waitForCmdQueueFinished();

    QTimer *timeoutTimer, *syncTimer;
};

#endif // CLOUDUPLOADER_H
