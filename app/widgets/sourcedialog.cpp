#include "sourcedialog.h"
#include "ui_sourcedialog.h"
#include "usbsettingswidget.h"
#include <QSerialPortInfo>
#include "../classes/usbdatasource.h"


SourceDialog::SourceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SourceDialog)
{
    ui->setupUi(this);

    #ifdef QT_DEBUG
    ui->sourceTypeComboBox->addItem("Debug: Fake Data Source");
    #else
    ui->sourceTypeWidget->setHidden(true);
    #endif

    ui->firmwareUpdateLabel->setStyleSheet("QLabel { color: red;}");
    ui->firmwareUpdateLabel->setHidden(true);
    ui->applyButton->setEnabled(false);
}

SourceDialog::~SourceDialog()
{
    delete ui;
}

void SourceDialog::scanUSBDevices()
{
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        if (info.portName() == defaultSettings.portName)
        {
            addSource(defaultSettings, nChannels);
            portScansStarted++;
            portScansCompleted++;
            continue;
        }

        QThread *sourceThread = new QThread();

        USBDataSource::Settings scanSettings;
        scanSettings.portName = info.portName();

        USBDataSource *source = new USBDataSource(scanSettings, 10, MVector::nChannels);

        // run source in separate thread
        source->moveToThread(sourceThread);

        connect(sourceThread, SIGNAL(started()), source, SLOT(started())); // init source when thread was started
        connect(source, SIGNAL(destroyed()), sourceThread, SLOT(quit()));   // end thread when source is deleted
        connect(sourceThread, SIGNAL(finished()), sourceThread, SLOT(deleteLater())); // delete thread when finishes
        connect(source, &USBDataSource::statusSet, this, &SourceDialog::onSourceStatusSet);
        sourceThread->start();
        portScansStarted++;
    }
}

void SourceDialog::onSourceStatusSet(DataSource::Status newStatus)
{
       USBDataSource* source = static_cast<USBDataSource*>(sender());

       switch (newStatus)
       {
       // starting to connect:
       // -> ignore
       case DataSource::Status::NOT_CONNECTED:
       case DataSource::Status::CONNECTING:
           break;
        // connection error:
        // -> no supported device
       case DataSource::Status::CONNECTION_ERROR:
           source->deleteLater();
           portScansCompleted++;
           break;
        // connected succesfully
        // -> compatible device, add device info
        case DataSource::Status::CONNECTED:
            addSource(source->getSettings(), source->getNChannels());
            source->deleteLater();
            portScansCompleted++;
           break;
        default:
           throw std::runtime_error("Unexpected status!");
       }

       QString msg;
       if (portScansStarted == portScansCompleted)
       {
           msg = "Scan complete. ";
           int nDevices = ui->portListComboBox->count();
           if (nDevices == 1)
                msg += "1 device detected.";
           else
               msg += QString::number(nDevices) + " Smell Inspector devices found.";
       } else
       {
            msg = "Scanning for devices (" + QString::number(portScansCompleted) + "/" + QString::number(portScansStarted) + ")...";
       }
       ui->scanLabel->setText(msg);
}
void SourceDialog::addSource(USBDataSource::Settings sourceSettings, int nChannels)
{
    // prepare device info
    QStringList sourceInfoList;
    sourceInfoList << sourceSettings.portName << sourceSettings.deviceId << sourceSettings.firmwareVersion << QString::number(nChannels) << QString::number(sourceSettings.hasEnvSensors);

    // add info to port selection comboBox
    ui->portListComboBox->addItem(sourceSettings.portName, sourceInfoList);
}

void SourceDialog::setUSBSettings(USBDataSource::Settings usbSettings, int nChannels)
{
    defaultSettings = usbSettings;
    this->nChannels = nChannels;
}

USBDataSource::Settings SourceDialog::getUSBSettings() const
{
    return usbSettings;
}

void SourceDialog::on_cancelButton_clicked()
{
    this->reject();
}

void SourceDialog::on_applyButton_clicked()
{
    if (ui->sourceTypeComboBox->currentText() == "USB")
    {
        sourceType = DataSource::SourceType::USB;
        identifier = ui->portListComboBox->currentText();
        nChannels = ui->portListComboBox->currentData().toStringList().at(3).toInt();
    }
    else if (ui->sourceTypeComboBox->currentText() == "Debug: Fake Data Source")
    {
        sourceType = DataSource::SourceType::FAKE;
        identifier = "Fake";
        nChannels = MVector::nChannels;
    }
    else
        Q_ASSERT("Unknown source type selected!" && false);

    this->accept();
}

int SourceDialog::getNChannels() const
{
    return nChannels;
}

DataSource::SourceType SourceDialog::getSourceType() const
{
    return sourceType;
}

void SourceDialog::setSourceType(const DataSource::SourceType &sourceType)
{
    if (sourceType == DataSource::SourceType::USB)
        ui->sourceTypeComboBox->setCurrentText("USB");
    else if (sourceType == DataSource::SourceType::FAKE)
        ui->sourceTypeComboBox->setCurrentText("Debug: Fake Data Source");
}

void SourceDialog::on_portListComboBox_currentIndexChanged(int index)
{
    if (index == -1)
        return;

    const QStringList list = ui->portListComboBox->itemData(index).toStringList();
    QString port = list.at(0);
    QString deviceId = !list.at(1).isEmpty() ? list.at(1) : "Unknown";
    QString firmwareVersion = !list.at(2).isEmpty() ? list.at(2) : "1.2.0" ;
    QString nChannels = list.at(3);
    bool hasEnvSensors = list.at(4) == "1";

    ui->portLabel->setText(tr("Port: %1").arg(port));
    ui->idLabel->setText(tr("Device Id: %1").arg(deviceId));
    ui->firmwareVersionLabel->setText(tr("Firmware Version: %1").arg(firmwareVersion));
    ui->channelLabel->setText(tr("Channels: %1").arg(nChannels));

    // firmware < 1.3.0 and device has environment sensors (temperature/ humidity)
    // -> firmware has to be updated
    if (firmwareVersion < "1.3.0" & hasEnvSensors)
    {
        ui->firmwareUpdateLabel->setHidden(false);
        ui->applyButton->setEnabled(false);
    } else
    {
        ui->firmwareUpdateLabel->setHidden(true);
        ui->applyButton->setEnabled(true);
    }

    // update usbSettings
    usbSettings.portName = port;
    usbSettings.deviceId = deviceId;
    usbSettings.firmwareVersion = firmwareVersion;
    usbSettings.hasEnvSensors = hasEnvSensors;
}

void SourceDialog::showEvent(QShowEvent *)
{
    scanUSBDevices();
}
