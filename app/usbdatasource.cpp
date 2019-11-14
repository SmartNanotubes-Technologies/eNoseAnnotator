#include "usbdatasource.h"
#include "usbsettingsdialog.h"


#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

USBDataSource::USBDataSource(QObject *parent, USBSettingsDialog::Settings usbSettings) : QObject(parent)
{
    Q_ASSERT("Parent has to be specified!" && parent != nullptr);

    serial = new QSerialPort();

    // no usb settings specified
    if (usbSettings.portName == "default")
        {
        settings = new USBSettingsDialog(static_cast<QWidget*>(this->parent()));
        if (settings->exec())
        {
            makeConnections();

            timer.setSingleShot(true);
            timer.start(5000);
        }
    } else  // usb settings specified
    {
        settings = new USBSettingsDialog(static_cast<QWidget*>(this->parent()));
        if (!settings->setSettings(usbSettings))
        {
            QString errorString = "Serial port " + usbSettings.portName + " does not exist.";
            QMessageBox::warning(static_cast<QWidget*>(this->parent()), "USB error", errorString);
            return;
        }

        makeConnections();
    }
}


USBDataSource::~USBDataSource()
{
    closeSerialPort();
}

void USBDataSource::makeConnections()
{
    connect(serial, &QSerialPort::readyRead, this, &USBDataSource::handleReadyRead);
    connect(serial, &QSerialPort::errorOccurred, this, &USBDataSource::handleError);
    connect(&timer, &QTimer::timeout, this, &USBDataSource::handleTimeout);
}

bool USBDataSource::getPaused() const
{
    return paused;
}

QSerialPort *USBDataSource::getSerial() const
{
    return serial;
}

void USBDataSource::changeSettings()
{
    closeSerialPort();

    USBSettingsDialog::Settings s = settings->getSettings();
    delete  settings;

    settings = new USBSettingsDialog(static_cast<QWidget*>(this->parent()));
    settings->setSettings(s);

    if (settings->exec())
    {
        timer.setSingleShot(true);
        timer.start(5000);

        openSerialPort();
    }
}

void USBDataSource::openSerialPort()
{
    Q_ASSERT (settings != nullptr);

    USBSettingsDialog::Settings s = settings->getSettings();

    emit newMeasurement(s.sensorId);

    serial->setPortName(s.portName);
    serial->setBaudRate(dBaudRate);
    serial->setDataBits(dDataBits);
    serial->setParity(dParity);
    serial->setStopBits(dStopBits);
    serial->setFlowControl(dFlowControl);



    // open connection
    if (serial->open(QIODevice::ReadOnly))
    {
        serial->clear();
        status = Status::OPEN;

        timer.setSingleShot(true);
        timer.start(5000);
    } else
    {
        QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error: Cannot open USB connection", serial->errorString());
        status = Status::ERR;
    }
}

void USBDataSource::closeSerialPort()
{
    if (serial->isOpen())
    {
        serial->clear();
        serial->close();

        status = Status::CLOSED;

        timer.stop();
    }
}

void USBDataSource::handleReadyRead()
{
    // reset timer
    timer.start(5000);

    // process lines
    while (serial->canReadLine())
        processLine(serial->readLine());

}

void USBDataSource::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError) {
        QString errorString("An I/O error occurred while reading the data from port " + serial->portName() + ",  error: " + serial->errorString());
        QMessageBox::warning(static_cast<QWidget*>(this->parent()), "USB Error", errorString);

        closeSerialPort();

        status = Status::ERR;
        emit serialError();
    }
}

void USBDataSource::handleTimeout()
{
    closeSerialPort();
    QMessageBox::warning(static_cast<QWidget*>(parent()), "USB timeout", "The usb connection has timed out. Try replugging the sensor and connect again.");
    status = Status::ERR;
    emit serialError();
}

void USBDataSource::processLine(const QByteArray &data)
{
    if (paused)
        return;

    QString line(data);

//    qDebug() << line;
    if (line.startsWith("count"))
    {
        uint timestamp = QDateTime::currentDateTime().toTime_t();

        qDebug() << timestamp << ": Received new data";

        // line looks like: count=___,var1=____._,var2=____._,....
        QList<QString> dataList = line.split(',');
        QList<QString> valueList;

        for (auto element : dataList)
            valueList << element.split('=')[1];

        // extract values
        int count = valueList[0].toInt();

        if (startCount == 0 || count == 1)    // first count
        {            
            startCount = count;

            // reset base level
            for (uint i=0; i<MVector::size; i++)
                baselevelVector.array[i] = valueList[i+1].toDouble() / nBaseLevel;

            emit beginSetBaseLevel();
        }
        else if (count < startCount + nBaseLevel -1) // prepare baselevel
        {
            for (uint i=0; i<MVector::size; i++)
                baselevelVector.array[i] += valueList[i+1].toDouble() / nBaseLevel;
        } else if (count == startCount + nBaseLevel -1) // set baselevel
        {
            for (uint i=0; i<MVector::size; i++)
                baselevelVector.array[i] += valueList[i+1].toDouble() / nBaseLevel;
           emit baseLevelSet(timestamp, baselevelVector);
        }
        else // get vector & emit
        {
            if (status != Status::OPEN)
                status = Status::OPEN;

            MVector vector;
            for (uint i=0; i<MVector::size; i++)
                vector.array[i] = valueList[i+1].toDouble();

//            qDebug() << "Vector Received: \n" << vector.toString();
            emit vectorReceived(timestamp, vector);
        }
    }
}

void USBDataSource::setPaused(bool value)
{
    paused = value;

    if (value)
        status = Status::PAUSED;
    else
        status = Status::CLOSED;
}

void USBDataSource::reset()
{
    startCount = 0;
}