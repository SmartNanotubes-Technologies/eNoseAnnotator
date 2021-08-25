#include "flashwidget.h"

#include <QtCore>
#include <QGroupBox>
#include <QLayout>
#include <QMessageBox>
#include <QFileDialog>

FlashDialog::FlashDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("Flash Firmware");

    espFlasher = new EspFlasher(this);

    // init ui elements
    pythonDirLineEdit = new QLineEdit();
    pythonCmdLineEdit = new QLineEdit();
    firmwareBinaryLineEdit = new QLineEdit();
    pythonDirButton = new QPushButton("...");
    pythonErrorLabel = new QLabel();
    pythonErrorLabel->setTextFormat(Qt::RichText);
    pythonErrorLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    pythonErrorLabel->setOpenExternalLinks(true);

    firmwareBinaryButton = new QPushButton("...");
    flashButton = new QPushButton("Flash");
    flashButton->setEnabled(false);
    textDisplay = new QPlainTextEdit();
    textDisplay->setReadOnly(true);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                        | QDialogButtonBox::Cancel
                                        | QDialogButtonBox::Reset);
    usbSettingsWidget = new USBSettingsWidget();

    // create layout
    // 1) Python settings
    QGroupBox* pythonSettingsBox = new QGroupBox("Python settings");
    QVBoxLayout* pythonSettingslayout = new QVBoxLayout();

    QHBoxLayout* pythonDirLayout = new QHBoxLayout();
    pythonDirLayout->addWidget(new QLabel("Python directory: "));
    pythonDirLayout->addWidget(pythonDirLineEdit);
    pythonDirLayout->addWidget(pythonDirButton);
    pythonSettingslayout->addLayout(pythonDirLayout);

    QHBoxLayout* pythonCmdLayout = new QHBoxLayout();
    pythonCmdLayout->addWidget(new QLabel("Python command: "));
    pythonCmdLayout->addWidget(pythonCmdLineEdit);
    pythonSettingslayout->addLayout(pythonCmdLayout);

    pythonSettingslayout->addWidget(pythonErrorLabel);

    pythonSettingsBox->setLayout(pythonSettingslayout);

    // 2) USB settings
    QGroupBox* usbSettingsGroupBox = new QGroupBox("USB settings");
    QVBoxLayout* usbSettingsLayout = new QVBoxLayout();
    usbSettingsLayout->addWidget(usbSettingsWidget);
    usbSettingsGroupBox->setLayout(usbSettingsLayout);

    // 3) firmware binary
    QGroupBox* firmwareBinaryGroupBox = new QGroupBox("Firmware");
    QHBoxLayout* firmwareBinaryLayout = new QHBoxLayout();
    firmwareBinaryLayout->addWidget(new QLabel("Firmware binary: "));
    firmwareBinaryLayout->addWidget(firmwareBinaryLineEdit);
    firmwareBinaryLayout->addWidget(firmwareBinaryButton);
    firmwareBinaryLayout->addWidget(flashButton);
    firmwareBinaryGroupBox->setLayout(firmwareBinaryLayout);

    // 4) text display
    QGroupBox* outputGroupBox = new QGroupBox("Output");
    QHBoxLayout* outputGroupBoxLayout = new QHBoxLayout();
    outputGroupBoxLayout->addWidget(textDisplay);
    outputGroupBox->setLayout(outputGroupBoxLayout);

    // main layout
    QVBoxLayout* layout = new QVBoxLayout();

    layout->addWidget(pythonSettingsBox);
    layout->addWidget(usbSettingsGroupBox);
    layout->addWidget(firmwareBinaryGroupBox);
    layout->addWidget(outputGroupBox);
    layout->addWidget(buttonBox);

    setLayout(layout);

    // connections
    #ifdef QT_DEBUG
    connect(espFlasher, &EspFlasher::debugMsg, this, &FlashDialog::addOutputMsg);
    #endif
    connect(espFlasher, &EspFlasher::outputMsg, this, &FlashDialog::addOutputMsg);
    connect(espFlasher, &EspFlasher::commandFinished, this, &FlashDialog::commandFinished);
    connect(pythonDirLineEdit, &QLineEdit::editingFinished, this, &FlashDialog::checkSettings);
    connect(pythonCmdLineEdit, &QLineEdit::editingFinished, this, &FlashDialog::checkSettings);
    connect(firmwareBinaryLineEdit, &QLineEdit::editingFinished, this, &FlashDialog::checkSettings);
    connect(pythonDirButton, &QPushButton::clicked, this, &FlashDialog::openPythonDirDialog);
    connect(firmwareBinaryButton, &QPushButton::clicked, this, &FlashDialog::openBinaryDialog);
    connect(flashButton, &QPushButton::clicked, this, &FlashDialog::flashFirmware);
    connect(buttonBox, &QDialogButtonBox::clicked, this, &FlashDialog::handleButtonBoxClick);
}

void FlashDialog::setIdentifier(QString identifier)
{
    usbSettingsWidget->setPortName(identifier);
}

void FlashDialog::detectPython()
{
    // init python settings
    textDisplay->appendPlainText("--- Setting up esptool ---\n");
    pythonErrorLabel->setText("Detecting Python...");
    pythonErrorLabel->setStyleSheet("QLabel { color : yellow; }");

    pythonOk = espFlasher->checkDefaultPython();
    pythonDirLineEdit->setText(espFlasher->getPythonDir());
    pythonCmdLineEdit->setText(espFlasher->getPythonCmd());

    if (pythonOk)
    {
        settingsReset = true;
        checkSettings();
    }
    else
    {
        pythonErrorLabel->setText("No Python version >= 2.7 or 3.4 detected. \nDownload and install the latest version from <a href='https://www.python.org/downloads/'>here</a> and/ or update the python settings.");
        pythonErrorLabel->setStyleSheet("QLabel { color : red; }");
        flashButton->setEnabled(false);
    }
}

void FlashDialog::setPython(QString pythonDir, QString pythonCmd)
{
    textDisplay->appendPlainText("--- Setting up esptool ---\n");
    espFlasher->setPythonDir(pythonDir);
    pythonDirLineEdit->setText(pythonDir);

    espFlasher->setPythonCmd(pythonCmd);
    pythonCmdLineEdit->setText(pythonCmd);

    checkSettings();
}

void FlashDialog::checkSettings()
{
    if (cmdRunning)
        return;

    bool pythonSettingsChanged = settingsReset || pythonDirLineEdit->text() != espFlasher->getPythonDir() || pythonCmdLineEdit->text() != espFlasher->getPythonCmd();
    bool firmwareOk = QFile::exists(firmwareBinaryLineEdit->text());

    if (!pythonSettingsChanged && pythonOk && pipOk && esptoolOk)
    {
        if (firmwareOk)
            flashButton->setEnabled(firmwareOk);
        return;
    }

    if (pythonSettingsChanged)
    {
        pythonOk = false;
        pipOk = false;
        pipInstalled = false;
        esptoolOk = false;
        esptoolInstalled = false;
        settingsReset = false;

        espFlasher->setPythonDir(pythonDirLineEdit->text());
        espFlasher->setPythonCmd(pythonCmdLineEdit->text());
    }

    if (!pythonOk && !espFlasher->checkPython())
    {
        pythonErrorLabel->setText(espFlasher->getErrorMsg());
        pythonErrorLabel->setStyleSheet("QLabel { color : red; }");
        flashButton->setEnabled(false);
        return;
    }
    pythonOk = true;

    // check pip
    if (!pipOk && !espFlasher->checkPip())
    {
        if (pipInstalled)
        {
            pythonErrorLabel->setText("Error: Unable to install pip!");
            pythonErrorLabel->setStyleSheet("QLabel { color : red; }");
            flashButton->setEnabled(false);
            return;
        } else
        {
            pipInstalled = true;
            pythonErrorLabel->setText("Installing pip...");
            pythonErrorLabel->setStyleSheet("QLabel { color : yellow; }");
            setCmdRunning(true);
            espFlasher->installPip();
            return;
        }
    }
    pipOk = true;

    // check esptool
    if (!esptoolOk && !espFlasher->checkEsptool())
    {
        if (esptoolInstalled)
        {
            pythonErrorLabel->setText("Error: Unable to install esptool!");
            pythonErrorLabel->setStyleSheet("QLabel { color : red; }");
            flashButton->setEnabled(false);
            return;
        } else
        {
            esptoolInstalled = true;

            pythonErrorLabel->setText("Installing esptool...");
            pythonErrorLabel->setStyleSheet("QLabel { color : yellow; }");
            setCmdRunning(true);
            espFlasher->installEsptool();
            return;
        }
    }
    esptoolOk = true;

    flashButton->setEnabled(firmwareOk);

    textDisplay->appendPlainText("--> Python settings ok.");
    pythonErrorLabel->setText("Python settings ok.");
    pythonErrorLabel->setStyleSheet("QLabel { color : green; }");
}

void FlashDialog::addOutputMsg(QString &msg)
{
    if (printOutput)
        textDisplay->appendPlainText(msg);
}

void FlashDialog::openPythonDirDialog()
{
    QString currentDir = firmwareBinaryLineEdit->text();

    QString dir = QFileDialog::getExistingDirectory(this, "Select python directory", currentDir);
    if (!dir.isEmpty())
    {
        pythonDirLineEdit->setText(dir);
        checkSettings();
    }
}

void FlashDialog::openBinaryDialog()
{
    QString currentFile = firmwareBinaryLineEdit->text();

    QString  file = QFileDialog::getOpenFileName(this, "Select firmware binary file", currentFile, "Binary files (*.bin);; All files (*)");
    if (!file.isEmpty())
    {
        firmwareBinaryLineEdit->setText(file);
        checkSettings();
    }
}

void FlashDialog::setCmdRunning(bool value)
{
    cmdRunning = value;

    if (cmdRunning)
    {
        pythonDirButton->setEnabled(false);
        pythonDirLineEdit->setEnabled(false);
        pythonCmdLineEdit->setEnabled(false);
        firmwareBinaryButton->setEnabled(false);
        firmwareBinaryLineEdit->setEnabled(false);
        buttonBox->setEnabled(false);

        flashButton->setEnabled(false);
    }
    else
    {
        pythonDirButton->setEnabled(true);
        pythonDirLineEdit->setEnabled(true);
        pythonCmdLineEdit->setEnabled(true);
        firmwareBinaryButton->setEnabled(true);
        firmwareBinaryLineEdit->setEnabled(true);
        buttonBox->setEnabled(true);
        checkSettings();
    }
}

void FlashDialog::commandFinished(int exitCode)
{
    printOutput = true; // reset printOutput after command finished

    if (cmdRunning)
    {
        setCmdRunning(false);
        textDisplay->appendPlainText("Exit code: " + QString::number(exitCode));

        if (flashingFirmware)
        {
            if (exitCode == 0)
                textDisplay->appendPlainText("--> Firmware was flashed successfully!");
            else
                textDisplay->appendPlainText("--> Firmware flash failed!");

            flashingFirmware = false;
        }
    }
}

void FlashDialog::flashFirmware()
{
    Q_ASSERT(QFile(firmwareBinaryLineEdit->text()).exists());

    setCmdRunning(true);
    flashingFirmware = true;
    textDisplay->appendPlainText("--- Flashing Firmware ---\n");
    espFlasher->flashFirmware(usbSettingsWidget->getPortName(), firmwareBinaryLineEdit->text());
}

void FlashDialog::handleButtonBoxClick(QAbstractButton *button)
{
    if (button == buttonBox->button(QDialogButtonBox::Ok))
        accept();
    else if (button == buttonBox->button(QDialogButtonBox::Cancel))
        reject();
    else if (button == buttonBox->button(QDialogButtonBox::Reset))
    {
        detectPython();
        firmwareBinaryLineEdit->setText("");
        checkSettings();
    }
}

bool FlashDialog::getEsptoolOk() const
{
    return esptoolOk;
}

bool FlashDialog::getPipOk() const
{
    return pipOk;
}

bool FlashDialog::getPythonOk() const
{
    return pythonOk;
}

QString FlashDialog::getPythonDir() const
{
    return espFlasher->getPythonDir();
}

QString FlashDialog::getPythonCmd() const
{
    return espFlasher->getPythonCmd();
}
