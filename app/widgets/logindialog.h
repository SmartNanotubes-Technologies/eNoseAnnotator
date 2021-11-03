#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QObject>
#include <QDialog>
#include <QWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QRegularExpression>
#include "../classes/clouduploader.h"

class IntroWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IntroWidget(QWidget *parent = nullptr);

signals:
    void nextRequested();

private:
    QLabel *textLabel;
    QPushButton *nextButton;
};

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);

signals:
    void loginRequested(QString username, QString password);
    void showSignUpRequested();

public slots:
    void setFeedbackLabel(QString msg, bool isError=true);
    void clear();
    void setUILocked(bool);

private slots:
    void tryLogin();

private:
    QLabel *emailLabel, *pwLabel, *feedbackLabel, *signUpLabel;
    QLineEdit *emailLineEdit, *pwLineEdit;
    QPushButton *loginButton, *signUpButton;
};

class SignUpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SignUpWidget(QWidget *parent = nullptr);

signals:
    void signUpRequested(SignUpInfo signUpInfo);
    void showLoginRequested();

public slots:
    void clear();
    void setUILocked(bool);
    void setFeedback(QString);

private slots:
    void trySignUp();
    void onTOCLinkClicked(const QString &link);

private:
    QLabel *emailLabel, *pw1Label, *pw2Label, *feedbackLabel, *industryLabel, *companyLabel, *firstNameLabel, *lastNameLabel, *loginlabel, *toclabel;
    QLineEdit *emailLineEdit, *pw1LineEdit, *pw2LineEdit, *companyLineEdit, *firstNameLineEdit, *lastNameLineEdit;
    QCheckBox *tocCheckBox;
    QPushButton *loginButton, *signUpButton;
    QComboBox *industryCombobox;
    QRegularExpression emailRegex = QRegularExpression("(?:[a-z0-9!#$%&'*+\\/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+\\/=?^_`{|}~-]+)*|\"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*\")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\\[(?:(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9]))\.){3}(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9])|[a-z0-9-]*[a-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\\])",
                                                       QRegularExpression::CaseInsensitiveOption);

    bool isEmailValid(QString email);
    bool isPasswordValid(QString pw1, QString pw2);
    bool isCompanyValid(QString company);
    bool isIndustryValid(QString industry);
    bool isFirstNameValid(QString firstName);
    bool isLastNameValid(QString lastName);
    bool isTOCValid(bool tocAccepted);
};

class LoginActiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginActiveWidget(QWidget *parent = nullptr);

signals:
    void logoutRequested();
    void manualUploadRequested(QStringList &files);

public slots:
    void setFeedbackLabel(QString msg);
    void setEmail(QString email);
    void setUILocked(bool);

private slots:
    void logout();
    void showManualUpload();

private:
    QPushButton *logoutButton, *uploadButton;
    QLabel *uploadLabel, *statuslabel, *feedbackLabel;
};

class LoginDialog : public QDialog
{
   Q_OBJECT

public:
    explicit LoginDialog(CloudUploader *uploader, QWidget *parent = nullptr);

private slots:
    void tryLogin(QString username, QString password);
    void tryLogout();
    void trySignUp(SignUpInfo signUpInfo);
    void resizeWindow();
    void setCursorWaiting(bool);
    void manualUpload(QStringList &files);
    void onDialogShown();

private:
    QStackedWidget* stackedWidget;
    IntroWidget* introPage;
    LoginWidget* loginPage;
    SignUpWidget* signUpPage;
    LoginActiveWidget* loginActivePage;
    CloudUploader* uploader;

protected:
    void showEvent(QShowEvent *) override;
};

class TextDisplayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextDisplayDialog(QString boxText = "", QWidget *parent = nullptr);

public slots:
    void loadHtml(const QString &filepath);

private slots:
    void resizeWindow();

private:
    QDialogButtonBox *buttonBox;
    QTextEdit *textBox;
};

#endif // LOGINDIALOG_H
