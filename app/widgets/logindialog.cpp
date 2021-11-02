#include "logindialog.h"
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QGroupBox>
#include <QCursor>
#include <QFileDialog>
#include "../classes/defaultSettings.h"

LoginDialog::LoginDialog(CloudUploader *uploader, QWidget *parent):
    QDialog(parent),
    stackedWidget(new QStackedWidget),
    loginPage(new LoginWidget),
    signUpPage(new SignUpWidget),
    loginActivePage(new LoginActiveWidget),
    uploader(uploader)
{
    resizeWindow();

    QHBoxLayout *logoLayout = new QHBoxLayout;
    QLabel* logoLabel = new QLabel;
    logoLabel->setPixmap(QPixmap(":/logos/web_logo"));
    logoLabel->setScaledContents(true);
    logoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    logoLabel->setAlignment(Qt::AlignCenter);
//    logoLabel->setMinimumSize(0.1*QSize(2000, 354));
    logoLabel->setMaximumSize(0.2*QSize(2000, 354));
    logoLayout->addItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    logoLayout->addWidget(logoLabel, Qt::AlignCenter);
    logoLayout->addItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(signUpPage);
    stackedWidget->addWidget(loginActivePage);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    layout->addLayout(logoLayout);
    layout->addItem(new QSpacerItem(0, 50, QSizePolicy::Minimum, QSizePolicy::Minimum));
    layout->addWidget(stackedWidget);
    layout->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setLayout(layout);

    connect(loginPage, &LoginWidget::showPage, stackedWidget, &QStackedWidget::setCurrentIndex);
    connect(signUpPage, &SignUpWidget::showPage, stackedWidget, &QStackedWidget::setCurrentIndex);
    connect(loginActivePage, &LoginActiveWidget::showPage, stackedWidget, &QStackedWidget::setCurrentIndex);

    connect(loginPage, &LoginWidget::loginRequested, this, &LoginDialog::tryLogin);
    connect(loginActivePage, &LoginActiveWidget::logoutRequested, this, &LoginDialog::tryLogout);
    connect(signUpPage, &SignUpWidget::signUpRequested, this, &LoginDialog::trySignUp);

    connect(loginActivePage, &LoginActiveWidget::manualUploadRequested, this, &LoginDialog::manualUpload);


    if (uploader->isLoggedIn())
    {
        loginActivePage->setEmail(uploader->getEmail());
        stackedWidget->setCurrentWidget(loginActivePage);
    }
}

void LoginDialog::resizeWindow()
{
    QWidget* parent = static_cast<QWidget*>(this->parent());
    int nWidth = 800;
    int nHeight = 600;
    if (parent != nullptr)
        setGeometry(parent->x() + parent->width()/2 - nWidth/2,
            parent->y() + parent->height()/2 - nHeight/2,
            nWidth, nHeight);
    else
        resize(nWidth, nHeight);
}

void LoginDialog::setCursorWaiting(bool value)
{
    QCursor cursor = this->window()->cursor();
    if (value)
        cursor.setShape(Qt::CursorShape::WaitCursor);
    else
        cursor.setShape(Qt::CursorShape::ArrowCursor);

    this->window()->setCursor(cursor);
}

void LoginDialog::manualUpload(QStringList &files)
{
    if (files.isEmpty())
        return;

    setCursorWaiting(true);

    for (QString file : files)
    {
        uploader->syncFile(file);
    }
    setCursorWaiting(false);
    loginActivePage->setFeedbackLabel("Marked " + QString::number(files.size()) + " to be uploaded.");
}

void LoginDialog::setUploaderMsg(QString msg)
{

}

void LoginDialog::setSyncFinished(QString filename, bool success)
{

}

void LoginActiveWidget::showManualUpload()
{
    QStringList files = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more files to upload",
                            "", // TODO: default dir
                            "Measurement Files (*.csv)");

    emit manualUploadRequested(files);
}

void LoginDialog::tryLogin(QString username, QString password)
{

    loginPage->setUILocked(true);
    setCursorWaiting(true);

    bool success = uploader->loginUser(username, password);

    setCursorWaiting(false);
    loginPage->setUILocked(false);

    if (success)
    {
        loginPage->clear();
        loginActivePage->setEmail(uploader->getEmail());
        stackedWidget->setCurrentWidget(loginActivePage);
    } else
    {
        loginPage->setFeedbackLabel("Invalid username or password. Please try again.");
    }
}

void LoginDialog::tryLogout()
{
    setCursorWaiting(true);
    loginActivePage->setUILocked(true);

    bool success = uploader->logoutUser();

    setCursorWaiting(false);
    loginActivePage->setUILocked(false);

    if (success)
    {
        stackedWidget->setCurrentWidget(loginPage);
    }
    else
    {
        loginActivePage->setFeedbackLabel("Error: unable to log out");
    }
}

void LoginDialog::trySignUp(SignUpInfo signUpInfo)
{
    setCursorWaiting(true);
    signUpPage->setUILocked(true);

    bool success = uploader->registerUser(signUpInfo);

    setCursorWaiting(false);
    signUpPage->setUILocked(false);

    if (success)
    {
        stackedWidget->setCurrentWidget(loginPage);
        signUpPage->clear();

        loginPage->setFeedbackLabel("Registration successfull. You can log in now.", false);
        stackedWidget->setCurrentWidget(loginPage);
    }
    else
    {
        QString errorMsg = "Unable to register new account. ";
        errorMsg += uploader->getErrorMsg();
        signUpPage->setFeedback(errorMsg);
    }
}

LoginWidget::LoginWidget(QWidget *parent):
    QWidget(parent),
    emailLabel(new QLabel("Email: ")),
    pwLabel(new QLabel("Password: ")),
    feedbackLabel(new QLabel),
    emailLineEdit(new QLineEdit),
    pwLineEdit(new QLineEdit),
    loginButton(new QPushButton("Login")),
    signUpButton(new QPushButton("Sign up"))
{
    QHBoxLayout *emailLayout = new QHBoxLayout;
    emailLayout->addWidget(emailLabel);
    emailLayout->addWidget(emailLineEdit);
    emailLabel->setMinimumSize(80, 25);

    QHBoxLayout *pwLayout = new QHBoxLayout;
    pwLayout->addWidget(pwLabel);
    pwLayout->addWidget(pwLineEdit);
    pwLabel->setMinimumSize(80, 25);
    pwLineEdit->setEchoMode(QLineEdit::Password);

    QHBoxLayout *signUpLayout = new QHBoxLayout;
    signUpLayout->addItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    signUpLayout->addWidget(signUpButton);
    signUpButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QGroupBox* loginBox = new QGroupBox("Log In");
    QVBoxLayout *loginFormlayout = new QVBoxLayout;
    loginFormlayout->addLayout(emailLayout);
    loginFormlayout->addLayout(pwLayout);
    loginFormlayout->addWidget(feedbackLabel);
    loginFormlayout->addWidget(loginButton);
    loginFormlayout->addLayout(signUpLayout);
    loginBox->setLayout(loginFormlayout);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addItem(new QSpacerItem(70, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    hLayout->addWidget(loginBox);
    hLayout->addItem(new QSpacerItem(70, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(hLayout);
    layout->addItem(new QSpacerItem(10,10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setLayout(layout);

    connect(signUpButton, &QPushButton::clicked, this, [this](){
        emit showPage(1);
    });
    connect(loginButton, &QPushButton::clicked, this, &LoginWidget::tryLogin);
}

void LoginWidget::setFeedbackLabel(QString msg, bool isError)
{
    feedbackLabel->setText(msg);

    if (isError)
        feedbackLabel->setStyleSheet("QLabel { color: red;}");
    else
        feedbackLabel->setStyleSheet("QLabel { color: black;}");
}

void LoginWidget::setUILocked(bool value)
{
    emailLineEdit->setEnabled(!value);
    pwLineEdit->setEnabled(!value);
    loginButton->setEnabled(!value);
    signUpButton->setEnabled(!value);
}

void LoginWidget::clear()
{
    emailLineEdit->clear();
    pwLineEdit->clear();
    feedbackLabel->clear();
}

void LoginWidget::tryLogin()
{
    QString email = emailLineEdit->text();
    QString password = pwLineEdit->text();
    if (email.isEmpty() || password.isEmpty())
    {
        feedbackLabel->setText("Please provide email and password.");
        return;
    }

    feedbackLabel->clear();
    emit loginRequested(email, password);
}

SignUpWidget::SignUpWidget(QWidget *parent):
    QWidget(parent),
    emailLabel(new QLabel("Email: ")),
    pw1Label(new QLabel("Password: ")),
    pw2Label(new QLabel("Repeat password: ")),
    feedbackLabel(new QLabel),
    industryLabel(new QLabel("Industry: ")),
    companyLabel(new QLabel("Company: ")),
    firstNameLabel(new QLabel("First Name: ")),
    lastNameLabel(new QLabel("Last Name: ")),
    emailLineEdit(new QLineEdit),
    pw1LineEdit(new QLineEdit),
    pw2LineEdit(new QLineEdit),
    companyLineEdit(new QLineEdit),
    firstNameLineEdit(new QLineEdit),
    lastNameLineEdit(new QLineEdit),
    loginButton(new QPushButton("Login")),
    signUpButton(new QPushButton("Sign up")),
    industryCombobox(new QComboBox)
{
    feedbackLabel->setStyleSheet("QLabel { color: red;}");

    QHBoxLayout *emailLayout = new QHBoxLayout;
    emailLayout->addWidget(emailLabel);
    emailLayout->addWidget(emailLineEdit);
    emailLabel->setMinimumSize(130, 25);

    QHBoxLayout *industryLayout = new QHBoxLayout;
    industryLayout->addWidget(industryLabel);
    industryLayout->addWidget(industryCombobox);
    industryLabel->setMinimumSize(130, 25);

    industryCombobox->addItem(INDUSTRY_SELECTION_TEXT);
    for (QString industry : SignUpInfo::industryTypes)
        industryCombobox->addItem(industry);
    industryCombobox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QHBoxLayout *companyLayout = new QHBoxLayout;
    companyLayout->addWidget(companyLabel);
    companyLayout->addWidget(companyLineEdit);
    companyLabel->setMinimumSize(130, 25);

    QHBoxLayout *pw1Layout = new QHBoxLayout;
    pw1Layout->addWidget(pw1Label);
    pw1Layout->addWidget(pw1LineEdit);
    pw1LineEdit->setEchoMode(QLineEdit::Password);
    pw1Label->setMinimumSize(130, 25);
    pw1LineEdit->setEchoMode(QLineEdit::Password);

    QHBoxLayout *pw2Layout = new QHBoxLayout;
    pw2Layout->addWidget(pw2Label);
    pw2Layout->addWidget(pw2LineEdit);
    pw2LineEdit->setEchoMode(QLineEdit::Password);
    pw2Label->setMinimumSize(130, 25);
    pw2LineEdit->setEchoMode(QLineEdit::Password);

    QHBoxLayout *firstNameLayout = new QHBoxLayout;
    firstNameLayout->addWidget(firstNameLabel);
    firstNameLayout->addWidget(firstNameLineEdit);
    firstNameLabel->setMinimumSize(130, 25);

    QHBoxLayout *lastNameLayout = new QHBoxLayout;
    lastNameLayout->addWidget(lastNameLabel);
    lastNameLayout->addWidget(lastNameLineEdit);
    lastNameLabel->setMinimumSize(130, 25);

    QHBoxLayout *loginLayout = new QHBoxLayout;
    loginLayout->addItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    loginLayout->addWidget(loginButton);
    loginButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QGroupBox* signUpBox = new QGroupBox("Sign Up");
    QVBoxLayout *signUpFormlayout = new QVBoxLayout;
    signUpFormlayout->addLayout(emailLayout);
    signUpFormlayout->addLayout(firstNameLayout);
    signUpFormlayout->addLayout(lastNameLayout);
    signUpFormlayout->addLayout(companyLayout);
    signUpFormlayout->addLayout(industryLayout);
    signUpFormlayout->addLayout(pw1Layout);
    signUpFormlayout->addLayout(pw2Layout);
    signUpFormlayout->addWidget(feedbackLabel);
    signUpFormlayout->addWidget(feedbackLabel);
    signUpFormlayout->addWidget(signUpButton);
    signUpFormlayout->addLayout(loginLayout);
    signUpBox->setLayout(signUpFormlayout);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addItem(new QSpacerItem(70, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    hLayout->addWidget(signUpBox);
    hLayout->addItem(new QSpacerItem(70, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addItem(new QSpacerItem(10,10, QSizePolicy::Minimum, QSizePolicy::Expanding));
    layout->addLayout(hLayout);
    layout->addItem(new QSpacerItem(10,10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setLayout(layout);

    connect(signUpButton, &QPushButton::clicked, this, &SignUpWidget::trySignUp);
    connect(loginButton, &QPushButton::clicked, this, [this](){
        emit showPage(0);
    });
}

void SignUpWidget::clear()
{
    emailLineEdit->clear();
    pw1LineEdit->clear();
    pw2LineEdit->clear();
    feedbackLabel->clear();
    companyLineEdit->clear();
    industryCombobox->setCurrentIndex(0);
}

void SignUpWidget::setUILocked(bool value)
{
    emailLineEdit->setEnabled(!value);
    pw1LineEdit->setEnabled(!value);
    pw2LineEdit->setEnabled(!value);
    companyLineEdit->setEnabled(!value);
    firstNameLineEdit->setEnabled(!value);
    lastNameLineEdit->setEnabled(!value);
    loginButton->setEnabled(!value);
    signUpButton->setEnabled(!value);
    industryCombobox->setEnabled(!value);
}

void SignUpWidget::setFeedback(QString msg)
{
    feedbackLabel->setText(msg);
}

void SignUpWidget::trySignUp()
{
    QString email = emailLineEdit->text();
    if (!isEmailValid(email))
        return;

    QString firstName = firstNameLineEdit->text();
    if (!isFirstNameValid(firstName))
        return;

    QString lastName = lastNameLineEdit->text();
    if (!isLastNameValid(lastName))
        return;

    QString company = companyLineEdit->text();
    if (!isCompanyValid(company))
        return;

    QString industry = industryCombobox->currentText();
    if (!isIndustryValid(industry))
        return;

    QString password1 = pw1LineEdit->text();
    QString password2 = pw2LineEdit->text();
    if (!isPasswordValid(password1, password2))
        return;

    emit signUpRequested(SignUpInfo(email, password1, firstName, lastName, company, industry));
}

bool SignUpWidget::isEmailValid(QString email)
{
    auto emailMatch = emailRegex.match(email);

    if(!emailMatch.hasMatch())
    {
        emailLabel->setStyleSheet("QLabel { color: red;}");
        feedbackLabel->setText("Please provide a valid email addresse.");
        return false;
    }

    emailLabel->setStyleSheet("QLabel { color: black;}");
    feedbackLabel->setText("");

    return true;
}

bool SignUpWidget::isPasswordValid(QString pw1, QString pw2)
{
    if (pw1.isEmpty())
    {
        pw1Label->setStyleSheet("QLabel { color: red;}");
        feedbackLabel->setText("Please enter a password.");
        return false;
    }
    if (pw1.size() < 6)
    {
        pw1Label->setStyleSheet("QLabel { color: red;}");
        feedbackLabel->setText("Invalid password. The password has to contain at least 6 letters.");
        return false;
    }
    pw1Label->setStyleSheet("QLabel { color: black;}");

    if (pw1 != pw2)
    {
        pw2Label->setStyleSheet("QLabel { color: red;}");
        feedbackLabel->setText("The passwords are not matching. Please provide matching passwords.");
        return false;
    }
    pw2Label->setStyleSheet("QLabel { color: black;}");
    feedbackLabel->setText("");
    return true;
}

bool SignUpWidget::isCompanyValid(QString company)
{
    companyLabel->setStyleSheet("QLabel { color: black;}");
    feedbackLabel->setText("");
    return true;
}

bool SignUpWidget::isIndustryValid(QString industry)
{
    if (industry == INDUSTRY_SELECTION_TEXT)
    {
        industryLabel->setStyleSheet("QLabel { color: red;}");
        feedbackLabel->setText("Please select an industry.");
        return false;
    }
    industryLabel->setStyleSheet("QLabel { color: black;}");
    feedbackLabel->setText("");
    return true;
}

bool SignUpWidget::isFirstNameValid(QString firstName)
{
    if (firstName.isEmpty())
    {
        firstNameLabel->setStyleSheet("QLabel { color: red;}");
        feedbackLabel->setText("Please enter your name.");
        return false;
    }
    firstNameLabel->setStyleSheet("QLabel { color: black;}");
    feedbackLabel->setText("");
    return true;
}

bool SignUpWidget::isLastNameValid(QString lastName)
{
    if (lastName.isEmpty())
    {
        lastNameLabel->setStyleSheet("QLabel { color: red;}");
        feedbackLabel->setText("Please enter your name.");
        return false;
    }
    lastNameLabel->setStyleSheet("QLabel { color: black;}");
    feedbackLabel->setText("");
    return true;
}

LoginActiveWidget::LoginActiveWidget(QWidget *parent):
    QWidget(parent),
    logoutButton(new QPushButton("Logout")),
    uploadButton(new QPushButton("Manual Upload")),
    uploadLabel(new QLabel("")),
    statuslabel(new QLabel("")),
    feedbackLabel(new QLabel())
{   
    QHBoxLayout *uploadLayout = new QHBoxLayout();
    uploadLayout->addWidget(uploadLabel);
    uploadLayout->addWidget(uploadButton);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(statuslabel);
    vlayout->addItem(new QSpacerItem(10,10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    vlayout->addLayout(uploadLayout);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hLayout->addLayout(vlayout);
    hLayout->addItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addItem(new QSpacerItem(10,10, QSizePolicy::Minimum, QSizePolicy::Expanding));
    layout->addLayout(hLayout);
    layout->addItem(new QSpacerItem(10,10, QSizePolicy::Minimum, QSizePolicy::Expanding));
    vlayout->addWidget(feedbackLabel);
    layout->addWidget(logoutButton);

    setLayout(layout);

    connect(logoutButton, &QPushButton::clicked, this, &LoginActiveWidget::logout);
    connect(uploadButton, &QPushButton::clicked, this, &LoginActiveWidget::showManualUpload);
}

void LoginActiveWidget::setFeedbackLabel(QString msg)
{
    feedbackLabel->setText(msg);
}

void LoginActiveWidget::setEmail(QString email)
{
    QString text = "You are logged in as " + email;
    text += "\nAll new measurements are uploaded automatically.";
    text += "\nUse the 'Manual Upload' to upload existing measurements";
    statuslabel->setText(text);
}

void LoginActiveWidget::setUILocked(bool value)
{
    logoutButton->setEnabled(!value);
    uploadButton->setEnabled(!value);
}

void LoginActiveWidget::logout()
{
    emit logoutRequested();
}

