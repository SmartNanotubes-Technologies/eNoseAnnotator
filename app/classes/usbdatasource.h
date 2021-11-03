#ifndef USBDATASOURCE_H
#define USBDATASOURCE_H

#include "datasource.h"
#include "qserialport.h"

class USBDataSource : public DataSource
{
public:
    struct Settings{
        QString portName;

        // default QSerialPort settings
        QSerialPort::BaudRate baudRate = QSerialPort::Baud115200;
        QSerialPort::Parity parity = QSerialPort::Parity::NoParity;
        QSerialPort::DataBits dataBits = QSerialPort::Data8;
        QSerialPort::StopBits stopBits = QSerialPort::StopBits::OneStop;
        QSerialPort::FlowControl flowControl = QSerialPort::FlowControl::NoFlowControl;

        // device info
        QString deviceId;
        QString firmwareVersion;
        bool hasEnvSensors;
    };

    USBDataSource(Settings settings, int sensorTimeout, int sensorNChannels);
    ~USBDataSource();

    Settings getSettings();

    Status status();

    SourceType sourceType();

    QString identifier();

    QSerialPort *getSerial() const;

    QString getMAC() const;
    QString getFirmwareVersion() const;

    bool getDeviceInfoRequested() const;

    int getNChannels() const;

    bool getHasEnvSensors() const;

public slots:
    void init();
    void start();
    void pause();
    void stop();
    void reset();
    void reconnect();

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError serialPortError);
    void handleTimeout();
    void processLine(const QByteArray &line);
    void processMeasValLine(QString &line);
    void processFanLine(QString &line);
    void processEventLine(QString &line);
    bool isDeviceInfo(QString &line);
    void requestDeviceInfo();
    void getChannelInfo(QString &ine);

private:
    void openSerialPort();
    void closeSerialPort();
    void makeConnections();
    void closeConnections();

    AbsoluteMVector getVector(QStringList, QMap<QString, double> parameterMap);

    QSerialPort *serial = nullptr;
    Settings settings;
    bool measEventFlag = false;
    bool exposureStartSet = false,  exposureEndSet = false;
    bool runningMeasFailed = false;

    bool emitData;
    int deviceInfoRequests = 0;
};

#endif // USBDATASOURCE_H
