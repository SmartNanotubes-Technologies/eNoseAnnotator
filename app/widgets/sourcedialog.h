#ifndef SETSOURCEDIALOG_H
#define SETSOURCEDIALOG_H

#include <QDialog>
#include "../classes/datasource.h"
#include "../classes/usbdatasource.h"
#include "usbsettingswidget.h"

namespace Ui {
class SourceDialog;
}

class SourceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SourceDialog(QWidget *parent = nullptr);
    ~SourceDialog();

    DataSource::SourceType getSourceType() const;
    void setSourceType(const DataSource::SourceType &value);

    void setUSBSettings(USBDataSource::Settings usbSettings, int nChannels);
    USBDataSource::Settings getUSBSettings() const;

    int getNChannels() const;

    int getTimeout() const;
    void setTimeout(int value);

protected:
    void showEvent(QShowEvent *) override;

private slots:
    void on_cancelButton_clicked();

    void on_applyButton_clicked();

    void scanUSBDevices();

    void onSourceStatusSet(DataSource::Status newStatus);

    void addSource(USBDataSource::Settings sourceSettings, int nChannels);

    void on_portListComboBox_currentIndexChanged(int index);

private:
    Ui::SourceDialog *ui;

    DataSource::SourceType sourceType;
    QString identifier;
    QString sensorId;
    int portScansStarted = 0;
    int portScansCompleted = 0;
    USBDataSource::Settings usbSettings, defaultSettings;
    int nChannels = 0;
};

#endif // SETSOURCEDIALOG_H
