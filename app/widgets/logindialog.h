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
#include <QRegularExpression>

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);

signals:
    void showPage(int index);

private slots:
    void tryLogin();

private:
    QLabel *emailLabel, *pwLabel, *feedbackLabel;
    QLineEdit *emailLineEdit, *pwLineEdit;
    QPushButton *loginButton, *signUpButton;
};

class SignUpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SignUpWidget(QWidget *parent = nullptr);

signals:
    void showPage(int index);

private slots:
    void checkEmail();

private:
    QLabel *emailLabel, *pw1Label, *pw2Label, *feedbackLabel, *industryLabel, *companyLabel;
    QLineEdit *emailLineEdit, *pw1LineEdit, *pw2LineEdit, *companyLineEdit;
    QPushButton *loginButton, *signUpButton;
    QComboBox *industryCombobox;
    QRegularExpression emailRegex = QRegularExpression("(?:[a-z0-9!#$%&'*+\\/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+\\/=?^_`{|}~-]+)*|\"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*\")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\\[(?:(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9]))\.){3}(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9])|[a-z0-9-]*[a-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\\])",
                                                                    QRegularExpression::CaseInsensitiveOption);
};

class LoginActiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginActiveWidget(QWidget *parent = nullptr);

signals:
    void showPage(int index);

private slots:
    void logout();

private:
    QPushButton *logoutButton;

};

class LoginDialog : public QDialog
{
   Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);

private slots:
    void resizeWindow();

private:
    QStackedWidget* stackedWidget;
    LoginWidget* loginPage;
    SignUpWidget* signUpPage;
    LoginActiveWidget* loginActivePage;
};

#endif // LOGINDIALOG_H
