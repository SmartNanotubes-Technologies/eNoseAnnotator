#include "logindialog.h"
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QGroupBox>

LoginDialog::LoginDialog(QWidget *parent):
    QDialog(parent),
    stackedWidget(new QStackedWidget),
    loginPage(new LoginWidget),
    signUpPage(new SignUpWidget),
    loginActivePage(new LoginActiveWidget)
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
    feedbackLabel->setStyleSheet("QLabel { color: red;}");

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

void LoginWidget::tryLogin()
{
    if (emailLineEdit->text().isEmpty() || pwLineEdit->text().isEmpty())
    {
        feedbackLabel->setText("Please provide email and password.");
        return;
    }

    // TODO: check login
    bool loginSuccess = true;
    if (loginSuccess)
        emit showPage(2);
}

SignUpWidget::SignUpWidget(QWidget *parent):
    QWidget(parent),
    emailLabel(new QLabel("Email: ")),
    pw1Label(new QLabel("Password: ")),
    pw2Label(new QLabel("Repeat password: ")),
    feedbackLabel(new QLabel),
    industryLabel(new QLabel("Industry: ")),
    companyLabel(new QLabel("Company: ")),
    emailLineEdit(new QLineEdit),
    pw1LineEdit(new QLineEdit),
    pw2LineEdit(new QLineEdit),
    companyLineEdit(new QLineEdit),
    loginButton(new QPushButton("Login")),
    signUpButton(new QPushButton("Sign up")),
    industryCombobox(new QComboBox)
{
    feedbackLabel->setStyleSheet("QLabel { color: red;}");

    QHBoxLayout *emailLayout = new QHBoxLayout;
    emailLayout->addWidget(emailLabel);
    emailLayout->addWidget(emailLineEdit);
    emailLabel->setMinimumSize(130, 25);
    QObject::connect(emailLineEdit, &QLineEdit::editingFinished, this, &SignUpWidget::checkEmail);

    QHBoxLayout *industryLayout = new QHBoxLayout;
    industryLayout->addWidget(industryLabel);
    industryLayout->addWidget(industryCombobox);
    industryLabel->setMinimumSize(130, 25);
    QStringList industryTypes({"Industry A", "Industry B"});
    for (QString industry : industryTypes)
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

    QHBoxLayout *loginLayout = new QHBoxLayout;
    loginLayout->addItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    loginLayout->addWidget(loginButton);
    loginButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QGroupBox* signUpBox = new QGroupBox("Sign Up");
    QVBoxLayout *signUpFormlayout = new QVBoxLayout;
    signUpFormlayout->addLayout(emailLayout);
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

    connect(loginButton, &QPushButton::clicked, this, [this](){
        emit showPage(0);
    });
}

void SignUpWidget::checkEmail()
{
    auto emailMatch = emailRegex.match(emailLineEdit->text());

    if(!emailMatch.hasMatch())
    {
        emailLabel->setStyleSheet("QLabel { color: red;}");
        emailLineEdit->setStyleSheet("QLineEdit { color: red;}");
        feedbackLabel->setText("Please provide a valid email addresse.");
    }
    else
    {
        emailLabel->setStyleSheet("QLabel { color: black;}");
        emailLineEdit->setStyleSheet("QLineEdit { color: black;}");
        feedbackLabel->setText("");
    }

}

LoginActiveWidget::LoginActiveWidget(QWidget *parent):
    QWidget(parent),
    logoutButton(new QPushButton("Logout"))
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(logoutButton);

    setLayout(layout);

    connect(logoutButton, &QPushButton::clicked, this, &LoginActiveWidget::logout);
}

void LoginActiveWidget::logout()
{
    // TODO
    emit showPage(0);

}
