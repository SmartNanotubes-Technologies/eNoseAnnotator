#include "usbdatasource.h"

#include <QtCore>

/*!
 * \class USBDataSource
 * \brief implements the DataSource interface for USB connections.
 */
USBDataSource::USBDataSource(USBDataSource::Settings settings, int sensorTimeout, int sensorNChannels):
    DataSource(sensorTimeout, sensorNChannels),
    settings(settings)
{
    Q_ASSERT("Invalid settings. Serial port name has to be specified!" && settings.portName != "");

    timeout = sensorTimeout;
}

USBDataSource::~USBDataSource()
{
    closeConnections();
    closeSerialPort();
    delete serial;
}

USBDataSource::Settings USBDataSource::getSettings()
{
    return settings;
}

void USBDataSource::init()
{
    serial = new QSerialPort();
    makeConnections();
    openSerialPort();
}


DataSource::Status USBDataSource::status()
{
    if (connectionStatus != Status::NOT_CONNECTED)
    {
        // check if serial connection is still open
        if (!serial->isOpen())
        {
            closeSerialPort();
            setStatus (Status::CONNECTION_ERROR);
        }
    }

    return connectionStatus;
}

USBDataSource::SourceType USBDataSource::sourceType()
{
    return DataSource::SourceType::USB;
}

/*!
 * connect USBDataSource slots to QSerialPort signals
 */
void USBDataSource::makeConnections()
{
    connect(serial, &QSerialPort::readyRead, this, &USBDataSource::handleReadyRead);
    connect(serial, &QSerialPort::errorOccurred, this, &USBDataSource::handleError);
    connect(timer, &QTimer::timeout, this, &USBDataSource::handleTimeout);
}

/*!
 * disconnect USBDataSource slots from QSerialPort signals
 */
void USBDataSource::closeConnections()
{
    disconnect(serial, &QSerialPort::readyRead, this, &USBDataSource::handleReadyRead);
    disconnect(serial, &QSerialPort::errorOccurred, this, &USBDataSource::handleError);
    disconnect(timer, &QTimer::timeout, this, &USBDataSource::handleTimeout);
}

QSerialPort *USBDataSource::getSerial() const
{
    return serial;
}

void USBDataSource::openSerialPort()
{
    serial->setPortName(settings.portName);
    serial->setBaudRate(settings.baudRate);
    serial->setDataBits(settings.dataBits);
    serial->setParity(settings.parity);
    serial->setStopBits(settings.stopBits);
    serial->setFlowControl(settings.flowControl);

    // open connection
    if (serial->open(QIODevice::ReadWrite))
    {
        serial->clear();
        setStatus (DataSource::Status::CONNECTING);

        // don't emit data until measurement is started
        emitData = false;

        // set timer for timeout errors
        timer->setSingleShot(true);
        timer->start(timeout*1000);
    } else
    {
        emit error("Cannot open USB connection on port " + settings.portName);
        setStatus (Status::CONNECTION_ERROR);
    }
}

/*!
 * \brief USBDataSource::reconnect tries to reconnect the QSerialPort
 */
void USBDataSource::reconnect()
{
    if (serial != nullptr)
    {
        closeSerialPort();
        delete serial;
    }

    serial = new QSerialPort{};

    makeConnections();
    openSerialPort();
}

void USBDataSource::closeSerialPort()
{
    if (serial->isOpen())
    {
        serial->clear();
        serial->close();

        setStatus (Status::NOT_CONNECTED);

        timer->stop();
    }
}

/*!
 * triggered every time a vector is received from the eNose sensor
 */
void USBDataSource::handleReadyRead()
{
    // process lines
    while (serial->canReadLine())
        processLine(serial->readLine());
}

void USBDataSource::handleError(QSerialPort::SerialPortError serialPortError)
{
    // ignore signals other than read errors
    if (serialPortError != QSerialPort::SerialPortError::ReadError)
        return;
    if (status() == Status::RECEIVING_DATA)
        runningMeasFailed = true;

    closeSerialPort();

    setStatus (Status::CONNECTION_ERROR);
    emit error("An I/O error occurred while reading the data from USB port " + serial->portName() + ",  error: " + serial->errorString());
}

void USBDataSource::handleTimeout()
{
    if (connectionStatus == Status::CONNECTION_ERROR)
        return; // ignore if already in error state
    if (status() == Status::RECEIVING_DATA)
        runningMeasFailed = true;

    closeSerialPort();

    setStatus (Status::CONNECTION_ERROR);
    emit error("USB connection timed out without receiving data.\nCheck the connection settings and replug the sensor. Try to reconnect by starting a new measurement.");
}

/*!
 * \brief USBDataSource::processLine extracts a MVector from the message received.\n
 * if no measurement running: do nothing\n
 * if reset was triggered: use extracted MVector to calculate base vector. Emits base vector if no further vectors needed. \n
 * if measurement is running: emit extracted MVector
 */
void USBDataSource::processLine(const QByteArray &data)
{
    QString line(data);

    if (connectionStatus == DataSource::Status::CONNECTING)
    {
        if (runningMeasFailed)
        {
            runningMeasFailed = false;
            setStatus(DataSource::Status::PAUSED);
        }
        else
        {
            if (isDeviceInfo(line) || deviceInfoRequests == 2)
                setStatus(DataSource::Status::CONNECTED);
            else
            {
                getChannelInfo(line);
                requestDeviceInfo();
            }
            return;
        }
    }
    // reset timer
    timer->start(timeout*1000);

//    qDebug() << line;

    if (isMeasValLine(line))
        processMeasValLine(line);
    else if (isFanLine(line))
        processFanLine(line);
    else if (isEventLine(line))
        processEventLine(line);

}

bool USBDataSource::isMeasValLine(QString &line)
{
    if (settings.firmwareVersion < "2")
        return line.startsWith("count") || line.startsWith("start;") ;
    else
        return line.startsWith("start;");
}

bool USBDataSource::isFanLine(QString &line)
{
    return line.startsWith("fan");
}

bool USBDataSource::isEventLine(QString &line)
{
    return line.startsWith("event");
}

void USBDataSource::processMeasValLine(QString &line)
{
    if (!emitData)
        return;

    uint timestamp = QDateTime::currentDateTime().toTime_t();

    QStringList valueList;
    QMap<QString, double> parameterMap;
    count ++;

//        qDebug() << timestamp << ": Received new vector";

    if (settings.firmwareVersion < "2" && line.startsWith("count"))
    {
        // line looks like: count=___,var1=____,var2=____,...,var64=____[,sensorAttribute=____]*
        QStringList dataList = line.split(',');

        for (auto element : dataList)
        {
            if (element.startsWith("count"))
            {
                continue;
            }
            else if (element.startsWith("var"))
            {
                valueList << element.split('=')[1];
            }
            else
            {
                QStringList vals = element.split('=');
                QString parameterName = vals[0];

                if (parameterName == "humidity")
                    parameterName += "[%]";
                else if (parameterName == "temperature")
                    parameterName += "[??C]";

                parameterMap[parameterName] = vals[1].toDouble();
            }
        }
    }
    else    // firmware version >= 2
    {
        // line looks like: start;___,____,____,...,____[,____]*
        QStringList dataList = line.split(';');

        for (int i=1; i<dataList.size(); i++)
        {
            auto element = dataList[i];
            if (i < dataList.size() - 2)
                valueList << element;
            else if (i == dataList.size() - 1)
                parameterMap["humidity[%]"] = element.toDouble();
            else
                parameterMap["temperature[??C]"] = element.toDouble();
        }
    }

    // extract values
    AbsoluteMVector vector = getVector(valueList, parameterMap);
    if (measEventFlag)
    {
        measEventFlag = false;

        QString eventName;
        if (!exposureStartSet)
        {
            eventName = "exposure start";
            exposureStartSet = true;
        }
        else if (!exposureEndSet)
        {
            eventName = "exposure end";
            exposureEndSet = true;
        }
        else
        {
            eventName = "measurement event";
        }
        aClass measEventClass = aClass(eventName);
        aClass::staticClassSet.insert(measEventClass);
        vector.userAnnotation = Annotation({measEventClass});
    }

    if (startCount == 0)    // first count
    {
        // reset baselevel vector
        baselevelVectorMap.clear();

        setStatus (Status::SET_BASEVECTOR);
        startCount = count;
        baselevelVectorMap[timestamp] = vector;
    }
    if (status() == Status::SET_BASEVECTOR && count < startCount + nBaseVectors) // prepare baselevel
    {
        baselevelVectorMap[timestamp] = vector;
    } else if (status() == Status::SET_BASEVECTOR && count == startCount + nBaseVectors) // set baselevel
    {
        baselevelVectorMap[timestamp] = vector;

        MVector baselevelVector;

        for (uint ts : baselevelVectorMap.keys())
            baselevelVector = baselevelVector + baselevelVectorMap[ts] / baselevelVectorMap.size();

        // set base vector
        emit baseVectorSet(baselevelVectorMap.firstKey(), baselevelVector);
    }
    else // get vector & emit
    {
        if (connectionStatus != Status::RECEIVING_DATA)
            setStatus (Status::RECEIVING_DATA);

        emit vectorReceived(timestamp, vector);
    }
}

void USBDataSource::processFanLine(QString &line)
{
    QString valueString = line.split(":")[1];

    int fanLevel;
    if (valueString.contains("off"))
        fanLevel = 0;
    else
        fanLevel = valueString.toInt();

    emit fanLevelSet(fanLevel);
}

void USBDataSource::processEventLine(QString &line)
{
    qDebug() << line;
    // set measEventFlag
    measEventFlag = true;
}

bool USBDataSource::isDeviceInfo(QString &line)
{
    qDebug() << line;
    QRegularExpression infoRegex("^V(\\d+\\.\\d+\\.\\d+);(([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2}))$");
    auto infoMatch = infoRegex.match(line);

    if (!infoMatch.hasMatch())
        return false;

    settings.firmwareVersion = infoMatch.captured(1);
    settings.deviceId = infoMatch.captured(2);

    return true;
}

void USBDataSource::requestDeviceInfo()
{
    if (deviceInfoRequests == 0)
    {
        QString command = "GET_INFO\n";
        serial->write(command.toStdString().c_str(), command.size());
    }
    deviceInfoRequests++;
}

void USBDataSource::getChannelInfo(QString &line)
{
    QRegularExpression varRegex("^var(\\d+)=.*$");

    QStringList elements = line.split(',');
    for (QString element : elements)
    {
        auto varMatch = varRegex.match(element);
        if (varMatch.hasMatch() && varMatch.captured(1).toInt() > nChannels)
        {
            nChannels = varMatch.captured(1).toInt();
        }
        else if (element.startsWith("temperature") || element.startsWith("humidity"))
            settings.hasEnvSensors = true;
    }
}

/*!
 * triggered to start emition of measurements
 */
void USBDataSource::start()
{
    Q_ASSERT("Usb connection was already started!" && connectionStatus != Status::RECEIVING_DATA);
    Q_ASSERT("Usb connection is not connected!" && connectionStatus != Status::NOT_CONNECTED);

    // start new measurement
    if (status() != Status::PAUSED && !baselevelVectorMap.isEmpty())
    {
        // start meas
        startCount = 0;
        setStatus(Status::SET_BASEVECTOR);
    }
    // resume existing measurement
    else
        setStatus(Status::RECEIVING_DATA);

    emitData = true;
}

/*!
 * triggered to pause emition of measurementsgitk
 */
void USBDataSource::pause()
{
    Q_ASSERT("Usb connection was already started!" && connectionStatus == Status::RECEIVING_DATA || connectionStatus == Status::SET_BASEVECTOR);

    if (connectionStatus == Status::SET_BASEVECTOR)
        baselevelVectorMap.clear();

    emitData = false;
    setStatus (Status::PAUSED);

}

/*!
 * triggers the end of vector emition
 */
void USBDataSource::stop()
{
    Q_ASSERT("Trying to stop connection that is not receiving data!" && (connectionStatus == Status::RECEIVING_DATA ||  connectionStatus == Status::PAUSED));

    emitData = false;
    runningMeasFailed = false;
    setStatus (Status::CONNECTED);
}

/*!
 * \brief USBDataSource::reset triggers a reset of the base vector
 */
void USBDataSource::reset()
{
    Q_ASSERT("Trying to reset base vector without receiving data!" && connectionStatus == Status::RECEIVING_DATA || connectionStatus == Status::PAUSED);

    emitData = true;
    startCount = 0;
    setStatus (Status::SET_BASEVECTOR);
}

/*!
 * \brief USBDataSource::identifier returns the port name of the current QSerialPort
 */
QString USBDataSource::identifier()
{
    return settings.portName;
}

/*!
 * \brief USBDataSource::getVector return MVector with values contained in line.
 * Set infinite for negative values
 * \return
 */
AbsoluteMVector USBDataSource::getVector(QStringList vectorList, QMap<QString, double> parameterMap)
{
    AbsoluteMVector vector;

    for (int i=0; i<MVector::nChannels; i++)
    {
        // get values
        vector[i] = vectorList[i].toDouble(); // ignore first entry in vectorList (count)

//        // values < 0 or value == 1.0:
//        // huge resistances on sensor
//        if (vector[i] < 0 || qFuzzyCompare(vector[i], 1.0))
//            vector[i] = qInf();
    }

    vector.sensorAttributes = parameterMap;

    return vector;
}

bool USBDataSource::getHasEnvSensors() const
{
    return settings.hasEnvSensors;
}

QString USBDataSource::getMAC() const
{
    return settings.deviceId;
}

QString USBDataSource::getFirmwareVersion() const
{
    return settings.firmwareVersion;
}

int USBDataSource::getNChannels() const
{
    return nChannels;
}

bool USBDataSource::getDeviceInfoRequested() const
{
    return deviceInfoRequests > 0;
}
