#ifndef FLASHWIDGET_H
#define FLASHWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QLabel>

#include "usbsettingswidget.h"
#include "../classes/espflasher.h"

class FlashDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FlashDialog(QWidget *parent = nullptr);

    bool getPythonOk() const;

    bool getPipOk() const;

    bool getEsptoolOk() const;

    QString getPythonDir () const;

    QString getPythonCmd () const;

signals:

public slots:
    void setIdentifier(QString identifier);
    void setPython(QString pythonDir, QString pythonCmd);
    void detectPython();
    void checkSettings();
    void addOutputMsg(QString &msg);

private slots:
    void openPythonDirDialog();
    void openBinaryDialog();
    void setCmdRunning(bool cmdRunning);
    void commandFinished(int exidCode);
    void flashFirmware();
    void handleButtonBoxClick(QAbstractButton *button);

protected:
    void showEvent(QShowEvent *) override;

private:
    QLineEdit *pythonDirLineEdit, *pythonCmdLineEdit, *firmwareBinaryLineEdit;
    QLabel *pythonErrorLabel;
    QPushButton *pythonDirButton, *firmwareBinaryButton, *flashButton;
    QPlainTextEdit *textDisplay;
    QDialogButtonBox *buttonBox;
    USBSettingsWidget *usbSettingsWidget;
    EspFlasher *espFlasher;
    bool pythonOk = false,  pipOk = false,  esptoolOk = false,  pipInstalled = false, esptoolInstalled = false;
    bool cmdRunning = false, printOutput = true;
    bool flashingFirmware = false;
    bool settingsReset = false;
    bool pythonSet = false;
};

#endif // FLASHWIDGET_H
