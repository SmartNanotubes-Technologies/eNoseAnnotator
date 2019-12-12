#include "measurementdata.h"
#include <QDateTime>
#include <QVector>
#include <QtMath>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

MeasurementData::MeasurementData(QObject *parent) : QObject(parent)
{
    // zero init info
    setComment("");
    setSensorId("");
    for (int i=0; i<MVector::size; i++)
        sensorFailures[i] = false;

    for (int i=0; i<functionalisation.size(); i++)
        functionalisation[i] = 0;
}

const QMap<uint, MVector> MeasurementData::getRelativeData()
{
    QMap<uint, MVector> relativeData;

    for (uint timestamp : data.keys())
        relativeData[timestamp] = data[timestamp].toRelativeVector(getBaseLevel(timestamp));

    return relativeData;
}

const QMap<uint, MVector> MeasurementData::getAbsoluteData()
{
    return data;
}

const QMap<uint, MVector> MeasurementData::getSelectionMap()
{
    return selectedData;
}

void MeasurementData::clear()
{
    baseLevelMap.clear();
    clearSelection();
    data.clear();
    std::array<bool, 64> zeroFailures;
    for (int i=0; i<MVector::size; i++)
        zeroFailures[i] = false;
    setSensorFailures(zeroFailures);

    setComment("");
    setSensorId("");


    setDataChanged(false);
}

void MeasurementData::clearSelection()
{
    selectedData.clear();
    emit selectionCleared();
}

void MeasurementData::addMeasurement(uint timestamp, MVector vector)
{
    //  double usage of timestamps:
    if (data.contains(timestamp))
    {
        qWarning() << timestamp << ": double usage of timestamp!";
        // case 1: last timestamp was not skipped
        // -> increment current timestamp
        if (data.contains(timestamp-1))
            timestamp++;
        // case 2: last timesstamp was skipped:
        // move last measurement to skipped timestamp
        else
            data[timestamp-1] = data[timestamp];
    }

    // empty data:
    // this timestamp is startTimestamp of data
    if (data.isEmpty())
    {
        emit startTimestempSet(timestamp);
    }

    // calculate deviation in %
    MVector deviationVector;
    MVector baseLevelVector = getBaseLevel(timestamp);

    deviationVector = vector.toRelativeVector(baseLevelVector);

    // add data, update dataChanged
    data[timestamp] = vector;
    if (!dataChanged)
        setDataChanged(true);

    // save absolute vectors
    if (saveRawInput)
    {
        uint startTs = data.firstKey();

        QFile f("raw_input_" + QDateTime::fromTime_t(startTs).toString("dMMyyyy-h:mm:ss") + ".csv");
        if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
            QTextStream stream(&f);

            // write header
            if (startTs == timestamp)
            {
                QStringList headerList;
                headerList << "timestamp";

                for (int i=0; i<MVector::size; i++)
                    headerList << "channel" + QString::number(i+1);

                headerList << "user defined class" << "detected class";

                stream << headerList.join(";") << "\n";
            }

            stream << QDateTime::fromTime_t(data.firstKey()).toString("d.MM.yyyy-h:mm:ss") << ";" << vector.toString() << "\n";
            f.close();
        }
    }

    emit dataAdded(deviationVector, timestamp, true);
    emit absoluteDataAdded(vector, timestamp, true);
}

void MeasurementData::addMeasurement(uint timestamp, MVector vector, MVector baseLevel)
{
    // if new baseLevel: add to baseLevelMap
    if (baseLevelMap.isEmpty() || baseLevel != getBaseLevel(timestamp))
        baseLevelMap[timestamp] = baseLevel;

    // add vector to data
    addMeasurement(timestamp, vector);
}

MVector MeasurementData::getMeasurement(uint timestamp)
{
    Q_ASSERT(data.contains(timestamp));

    return data[timestamp];
}

QString MeasurementData::getComment()
{
    return dataComment;
}

void MeasurementData::setComment(QString new_comment)
{
    if (dataComment != new_comment)
    {
        dataComment = new_comment;
        setDataChanged(true);
        emit commentSet(dataComment);
    }
}

void MeasurementData::setFailures(std::array<bool, 64> failures)
{
    if (failures != sensorFailures)
    {
        sensorFailures = failures;
        setDataChanged(true);
        emit sensorFailuresSet(failures);
    }
}

void MeasurementData::setFailures(QString failureString)
{
    Q_ASSERT(failureString.length()==64);

    std::array<bool, 64> failures;

    for (int i=0; i<MVector::size; i++)
    {
        Q_ASSERT(failureString[i]=="0" || failureString[i]=="1");
        failures[i] = failureString[i] == "1";
    }

    setFailures(failures);
}

void MeasurementData::setSensorId(QString newSensorId)
{
    if (newSensorId != sensorId)
    {
        sensorId = newSensorId;

        // changing sensor id of empty measurement data should not trigger dataChanged
        if (!data.isEmpty())
            setDataChanged(true);

        emit sensorIdSet(sensorId);
    }
}

void MeasurementData::setBaseLevel(uint timestamp, MVector baseLevel)
{
    Q_ASSERT (!baseLevelMap.contains(timestamp));

    if (baseLevelMap.isEmpty() || baseLevelMap.last() != baseLevel)
    {
        baseLevelMap.insert(timestamp, baseLevel);
        setDataChanged(true);
        qDebug() << "New baselevel at " << timestamp << ":\n" << baseLevelMap[timestamp].toString();
    }
}

QList<aClass> MeasurementData::getClassList() const
{
    return classList;
}

bool MeasurementData::getSaveRawInput() const
{
    return saveRawInput;
}

void MeasurementData::setSaveRawInput(bool value)
{
    saveRawInput = value;
}

std::array<int, MVector::size> MeasurementData::getFunctionalities() const
{
    return functionalisation;
}

void MeasurementData::setFunctionalities(const std::array<int, MVector::size> &value)
{
    if (value != functionalisation)
    {
        functionalisation = value;
        setDataChanged(true);
    }
}

bool MeasurementData::changed() const
{
    return dataChanged;
}

void MeasurementData::setDataChanged(bool newValue)
{
    if (newValue != dataChanged)
    {
        dataChanged = newValue;
        emit dataChangedSet(newValue);
    }
}

MVector MeasurementData::getBaseLevel(uint timestamp)
{
    if (baseLevelMap.isEmpty())
    {
        qWarning() << "No baselevel was set!" << "\n";
        return  MVector::zeroes();
    }

    uint bsTimestamp = 0;

    //  go through baseLevelMap and store ts in bsTimestamp until timestamp < ts
    // -> bsTimestamp is last ts
    // or until timestamp == ts
    // -> bsTimestamp = ts
    for (auto  ts: baseLevelMap.keys())
    {
        if (ts > timestamp)
            break;
        else if (ts == timestamp)
        {
            bsTimestamp = ts;
            break;
        }

        bsTimestamp = ts;
    }
    Q_ASSERT ("Error: No baselevel was set!" && bsTimestamp!=0);

    return baseLevelMap[bsTimestamp];
}

std::array<bool, 64> MeasurementData::getSensorFailures() const
{
    return sensorFailures;
}

void MeasurementData::setSensorFailures(const std::array<bool, 64> &value)
{
    if (value != sensorFailures)
    {
        sensorFailures = value;
        setDataChanged(true);
        emit sensorFailuresSet(value);
    }
}

QString MeasurementData::getSensorId() const
{
    return sensorId;
}

QString MeasurementData::getFailureString()
{
    QString failureString("");

    for (int i=0; i<MVector::size; i++)
        if (sensorFailures[i])
            failureString += "1";
        else
            failureString += "0";

    return failureString;
}

bool MeasurementData::contains(uint timestamp)
{
    return data.contains(timestamp);
}

bool MeasurementData::saveData(QWidget *widget)
{
    return saveData(widget, data);
}

bool MeasurementData::saveData(QWidget *widget, QString filename)
{
    return  saveData(widget, filename, data);
}

bool MeasurementData::saveData(QWidget* widget, QMap<uint, MVector> map)
{
    QString path = (saveFilename != "") ? saveFilename : "./data";
    QString fileName = QFileDialog::getSaveFileName(widget, QString("Save data"), path, "Data files (*.csv)");

    if (fileName.split(".").last() != "csv")
        fileName += ".csv";

    if (fileName.isEmpty())
    {
        return false;
    }

    // update saveFilename
    saveFilename = fileName;
    return saveData(widget, fileName, map);
}

bool MeasurementData::saveData(QWidget* widget, QString filename, QMap<uint, MVector> map)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::information(widget, tr("Unable to open file"),
            file.errorString());
        return false;
    }

    QTextStream out(&file);
    // write info
    // version
    out << "#measurement data v" << version << "\n";

    // sensorId
    out << "#sensorId:" << sensorId << "\n";

    // sensor failures
    out << "#failures:" << getFailureString() << "\n";
    if (!dataComment.isEmpty())
    {
        // go through dataComment line-by-line
        QTextStream commentStream(&dataComment);
        QString line;

        while (commentStream.readLineInto(&line))
            out << "#" << line << "\n";
    }

    // sensor functionalisation
    QStringList funcList;
    for (int i=0; i<functionalisation.size(); i++)
        funcList << QString::number(functionalisation[i]);
    out << "#functionalisation:" << funcList.join(";") << "\n";

    // base vector
    for (uint timestamp : baseLevelMap.keys())
    {
        out << "#baseLevel:" << getTimestampStringFromUInt(timestamp) << ";";
        QStringList valueList;
        for (int i=0; i<MVector::size; i++)
            valueList << QString::number(baseLevelMap[timestamp][i], 'g', 10);
        out <<  valueList.join(";") << "\n";
    }

    // classes
    out << "#classes:";
    QStringList classStringList;
    for (aClass c : classList)
        classStringList << c.toString();
    out << classStringList.join(";") << "\n";

    // write header
    QStringList headerList;

    headerList << "#header:timestamp";

    for (int i=0; i<MVector::size; i++)
        headerList << "ch" + QString::number(i+1);

    headerList << "user defined class";
    headerList << "detected class";

    out << headerList.join(";") << "\n";

    // write data
    auto iter = map.begin();

    while (iter != map.end())
    {
        QStringList valueList;
        valueList << getTimestampStringFromUInt(iter.key());      // timestamp

        // vector
        for (int i=0; i<MVector::size; i++)
            valueList << QString::number(iter.value()[i], 'g', 10);
        // classes
        valueList << iter.value().userDefinedClass.toString() << iter.value().detectedClass.toString();
        out <<  valueList.join(";") << "\n";

        iter++;
    }
    setDataChanged(false);
    return true;
}

bool MeasurementData::saveSelection(QWidget *widget)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());
    return saveData(widget, selectedData);
}

bool MeasurementData::saveSelection(QWidget *widget, QString filename)
{
    Q_ASSERT("Selection data is empty!" && !selectedData.isEmpty());
    return saveData(widget, filename, selectedData);
}

bool MeasurementData::loadData(QWidget* widget)
{
    bool dataSaved = true;

    // ask to save old data
    if (!data.isEmpty() && dataChanged)
    {
        if (QMessageBox::question(widget, tr("Save data"),
            "Do you want to save the current measurement before loading data?\t") == QMessageBox::Ok)
            dataSaved=saveData(widget);
    }
    if (!dataSaved)
        return false;

    // make data directory
    if (!QDir("data").exists())
        QDir().mkdir("data");

    QString path = (saveFilename != "") ? saveFilename : "./data";
    QString fileName = QFileDialog::getOpenFileName(widget, "Open data file", path, "Data files (*.csv)");

    if (fileName.isEmpty())
    {
        return false;
    } else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::information(widget, "Unable to open file",
                file.errorString());
            return false;
        }

        // clear data
        clear();
        emit lgClearSelection();
        setComment("");
        setSensorId("");
        setFailures("0000000000000000000000000000000000000000000000000000000000000000");
        emit dataReset();

        // read data from file
        QTextStream in(&file);
        QString line, comment;
        QRegExp rx("\\;");

        // first line: check for version
        QString firstLine = in.readLine();
        qDebug() << firstLine;

        if (!firstLine.startsWith("#measurement data"))
        {
            QMessageBox::warning(widget, "Can not read file", "The selected file is not a measurement file!");
            return false;
        }
        if (firstLine.startsWith("#measurement data v"))
        {
            version = firstLine.right(firstLine.length()-QString("#measurement data v").length());
            version = version.split(";").join("");
        } else
            version = "0.1";

        emit setReplotStatus(false);

        bool readOk = true;
        while (readOk && in.readLineInto(&line))
        {
            qDebug() << line;
            if (line == "")
                continue;   // ignore
            if (line[0] == '~') // file help
                continue; // ignore
            else if (line[0] == '#') // meta info
                readOk = getMetaData(line);
            else  // data
                readOk = getData(line);
        }

        emit setReplotStatus (true);

        // catch errors reading files
        if (!readOk)
        {
            QMessageBox::critical(widget, "Error reading file", "Error in line:\n" + line);
            clear();
            clearSelection();
            return false;
        }

        setDataChanged(false);

        // set filename
        saveFilename = fileName;

        emit labelsUpdated(data);   // reset labels

        return true;
    }
}

bool MeasurementData::getMetaData(QString line)
{
    bool  readOk = true;

    if (line.startsWith("#sensorId:"))
    {
        QString sensorId = line.right(line.length()-QString("#sensorId:").length());
        sensorId = sensorId.split(";").join("");
        setSensorId(sensorId);
    }
    else if (line.startsWith("#failures:"))
    {
        QString failureString = line.right(line.length()-QString("#failures:").length());
        failureString=failureString.split(";").join("");
        if (failureString.size() != MVector::size)
        {
            qWarning() << "Failure string not valid. Using empty failure string.";
            failureString = "0000000000000000000000000000000000000000000000000000000000000000";
        }
        setFailures(failureString);
    }
    else if (line.startsWith("#functionalisation:"))
    {
        QString rawString = line.right(line.length()-QString("#functionalisation:").length());
        QStringList funcList = rawString.split(";");

        for (int i=0; i<functionalisation.size(); i++)
            functionalisation[i] = funcList[i].toInt();
    }
    else if (line.startsWith("#baseLevel:"))
    {

        line = line.right(line.length()-QString("#baseLevel:").size());
        QStringList valueList = line.split(";");

        // get timestamp: uint or string
        uint timestamp;
        bool isInt;
        timestamp = valueList[0].toUInt(&isInt);

        if (!isInt)
            timestamp = getTimestampUIntfromString(valueList[0]);

        // get base level vector
        MVector baseLevel;
        for (int i=0; i<MVector::size; i++)
            baseLevel[i] = valueList[i+1].toDouble();

        setBaseLevel(timestamp, baseLevel);
    }
    else if (line.startsWith("#classes:"))
    {
        QStringList classStringList =  line.right(line.length()-QString("#classes:").length()).split(";");
        for (QString classString : classStringList)
        {
            if (classString == "")
                continue;   // ignore empty classStrings

            if (!aClass::isClassString(classString))
            {
                QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Invalid class name: " + classString, "The class name is invalid!");
                readOk = false;
                break;
            }
            aClass c = aClass::fromString(classString);

            if (!classList.contains(c))
                addClass(c);
        }
    }
    else if (line.startsWith("#header:"))
        ;   // ignore
    else    // comment
        setComment(dataComment + line.right(line.length()-1) + "\n");

    return readOk;
}

bool MeasurementData::getData(QString line)
{
    uint timestamp;
    MVector vector;
    bool readOk = true;

    if (line == "") // ignore empty lines
        return true;

    // support old format:
    // - measurement values stored as relative vectors
    if (version == "0.1")
    {
        QStringList query = line.split(";");

        // line has to contain vector + [user defined and detected class]
        if ((query.size() < MVector::size+1) || (query.size() > MVector::size+3))
        {
            QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "Data format is not compatible (len=" + QString::number(query.size()) + "):\n" + line);
            return false;
        } else if (baseLevelMap.isEmpty())
        {
            QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "No baseLevel in data");
            return false;
        }
        // prepare vector
        timestamp = query[0].toUInt(&readOk);
        vector;
        for (int i=0; i<MVector::size; i++)
        {
            vector[i] = query[i+1].toDouble(&readOk);
        }
        if (query.size() > MVector::size+1)
        {
            if (aClass::isClassString(query[MVector::size+1]))
                vector.userDefinedClass = aClass::fromString(query[MVector::size+1]);
            else
            {
                return false;
            }
        }
        if (query.size() > MVector::size+2)
        {
            if (aClass::isClassString(query[MVector::size+2]))
                vector.detectedClass = aClass::fromString(query[MVector::size+2]);
            else
            {
                return false;
            }
        }
        vector = vector.getAbsoluteVector(getBaseLevel(timestamp));
    }
    else //if (version == "1.0")   // current version -> default
    {
        QStringList query = line.split(";");

        // line has to contain vector + [user defined and detected class]
        if ((query.size() < MVector::size+1) || (query.size() > MVector::size+3))
        {
            QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "Data format is not compatible (len=" + QString::number(query.size()) + "):\n" + line);
            return false;
        } else if (baseLevelMap.isEmpty())
        {
            QMessageBox::critical(static_cast<QWidget*>(this->parent()), "Error loading measurement data", "No baseLevel in data");
            return false;
        }
        // prepare vector
        // get timestamp: uint or string
        timestamp;
        bool isInt;
        timestamp = query[0].toUInt(&isInt);

        if(!isInt)
            timestamp = getTimestampUIntfromString(query[0]);

        for (int i=0; i<MVector::size; i++)
        {
            vector[i] = query[i+1].toDouble(&readOk);
        }
        if (query.size() > MVector::size+1)
        {
            QString userString = query[MVector::size+1];
            if (aClass::isClassString(userString))
                vector.userDefinedClass = aClass::fromString(userString);
            else
            {
                qWarning() << "Warning: User defined class invalid: \"" << userString << "\"";
            }
        }
        if (query.size() > MVector::size+2)
        {
            QString classString = query[MVector::size+2];
            if (aClass::isClassString(classString))
                vector.detectedClass = aClass::fromString(classString);
            else
            {
                qWarning() << "Warning: Detected class invalid: \"" << classString << "\"";
            }
        }

    }

    // ignore zero vectors
    if (vector != MVector::zeroes())
        addMeasurement(timestamp, vector, getBaseLevel(timestamp));

    return readOk;
}

void MeasurementData::generateRandomWalk()
{
    clear();

    uint now = QDateTime::currentDateTime().toTime_t();
    int n = 100;

    MVector baseLevel;
    for (int i=0; i<n; i++)
    {
        MVector measurement;
        for(int j=0; j<64; j++)
        {
            if (i == 0)
                measurement.array[j] = 1500.0 + (300.0)*(rand()/(double)RAND_MAX-0.5);

            else
                measurement.array[j] = (1+data[now+i-1].array[j]/100.0) * baseLevel.array[j] + 100.0 * (rand()/(double)RAND_MAX-0.5) + j/64.0;
        }

        if (i == 0)
        {
            emit startTimestempSet(now);
            baseLevel = measurement;
            setBaseLevel(now-1, baseLevel);
        }
        addMeasurement(now+i, measurement);
    }

    setSensorId("Randomly generated data");
}

void MeasurementData::setSelection(int lower, int upper)
{
    // clear selectedData
    clearSelection();

    // selection deselected
    if (upper < lower)
        return;

    MVector vector;

    qDebug() << "Selection requested: " << lower << ", " << upper;

    // single selection:
    if (lower == upper)
    {
        // update selectData
        selectedData[lower] = data[lower];
    }
    // multi selection:
    else
    {
        QMap<uint, MVector>::iterator beginIter, endIter;
        bool beginIterSet = false, endIterSet = false;

        // find lowest beginTimestamp >= lower & endTimestamp >= higher respectively
        for (uint timestamp : data.keys())
        {
            if (!beginIterSet && timestamp >= lower)
            {
                 beginIter = data.find(timestamp);
                beginIterSet = true;
            }
            if (!endIterSet && timestamp > upper)
            {
                endIter = data.find((timestamp));
                endIterSet = true;
                // increment endIter in order to point one element further than upper
//                endIter++;
                break;
            }
        }

        // upper > timestamp of last vector
        if (!endIterSet)
            endIter = data.end();

        // no vector in the interval [lower; upper]
        if (!beginIterSet || (beginIter != data.end() && beginIter.key()>upper) || (endIter != data.end() && endIter.key()<lower))
            return;

        // add selected vectors to seletedData
        for (auto iter=beginIter; iter != endIter; iter++)
            selectedData[iter.key()] = iter.value();
    }

    // calculate average vector
    if (selectedData.isEmpty())
        return;

    vector = getSelectionVector();

    qDebug() << "Selection made: " << selectedData.firstKey() << ", " << selectedData.lastKey() << "\n" << vector.toString() << "\n";

    emit selectionVectorChanged(vector, sensorFailures, functionalisation);
}

//const MVector MeasurementData::getSelectionVector(QMap<uint, MVector>::iterator begin, QMap<uint, MVector>::iterator end, uint endTimestamp, MultiMode mode)
//{
//    // init vector
//    MVector vector;
//    for (int i=0; i<MVector::size; i++)
//        vector[i] = 0.0;

//    if (mode == MultiMode::Average)
//    {
//    auto iter = begin;
//    int n = 0;

//    // iterate until end; if endTimestamp was set (!= 0) also check for endTimestamp
//    while (iter != end && (endTimestamp==0 || iter.key() <= endTimestamp))
//    {
//        for (int i=0; i<MVector::size; i++)
//            vector[i] += iter.value()[i];

//        iter++;
//        n++;
//    }


//    for (int i=0; i<MVector::size; i++)
//        vector[i] = vector[i] / n;
//    }
//    else
//        Q_ASSERT ("Error: Invalid MultiMode." && false);

//    return vector;
//}

const MVector MeasurementData::getSelectionVector(MultiMode mode)
{
    // only average supported
    Q_ASSERT(mode == MultiMode::Average);

    MVector selectionVector;
    // zero init
    for (int i=0; i<MVector::size; i++)
        selectionVector[i] = 0.0;

    for (auto timestamp : selectedData.keys())
    {
        // ignore zero vectors
        if (selectedData[timestamp] == MVector::zeroes())
            continue;

        // get relative selection vector
        MVector vector = selectedData[timestamp].toRelativeVector(getBaseLevel(timestamp));

        // calculate average
        if (mode == MultiMode::Average)
            for (int i=0; i<MVector::size; i++)
                selectionVector[i] += vector[i] / selectedData.size();
    }

    return selectionVector;
}

QString MeasurementData::sensorFailureString(std::array<bool, 64> failureBits)
{
    QString failureString;

    for (int hex=0; hex<MVector::size/4; hex++)
    {
        QString hexadecimal;
        int failureInt = 0;

        for (int bit=0; bit<4; bit++)
            if (failureBits[4*hex+bit])
                failureInt |= 1UL << bit;
        hexadecimal.setNum(failureInt,16);

        failureString = hexadecimal + failureString;
    }
    return failureString;
}

std::array<bool, MVector::size> MeasurementData::sensorFailureArray(QString failureString)
{
    std::array<bool, MVector::size> failureArray;
    for (int i=0; i<MVector::size; i++)
        failureArray[i] = 0;

    if (failureString == "None")
        return failureArray;

    for (int hex=0; hex < failureString.length(); hex++)
    {
        int failureInt = QString(failureString[failureString.length()-hex-1]).toInt(nullptr, 16);

        for (int bit=0; bit<4; bit++)
        {
            failureArray[4*hex+bit] |= failureInt & (1UL << bit);
        }
    }

    return failureArray;
}

void MeasurementData::setUserDefinedClassOfSelection(QString className, QString classBrief)
{
    for (auto timestamp : selectedData.keys())
    {
        // data and selectedData
        data[timestamp].userDefinedClass = aClass(className, classBrief);

        selectedData[timestamp].userDefinedClass = aClass(className, classBrief);
    }

    setDataChanged(true);
    emit labelsUpdated(selectedData);
}

void MeasurementData::setDetectedClassOfSelection(QString className, QString classBrief)
{
    for (auto timestamp : selectedData.keys())
    {
        // data and selectedData
        data[timestamp].detectedClass = aClass(className, classBrief);

        selectedData[timestamp].detectedClass = aClass(className, classBrief);
    }

    setDataChanged(true);
    emit labelsUpdated(selectedData);
}

void MeasurementData::addClass(aClass newClass)
{
    Q_ASSERT("Trying to add class that already exists!" && !classList.contains(newClass));

    classList.append(newClass);
    setDataChanged(true);
}

void MeasurementData::removeClass(aClass oldClass)
{
    Q_ASSERT("Trying to remove class that does not exist!" && classList.contains(oldClass));

    int index = classList.indexOf(oldClass);
    classList.removeAt(index);

    // update measurement data
    QMap<uint, MVector> updatedVectors;

    for (uint timestamp: data.keys())
    {
        bool updated = false;   // flag for updated class

        if (data[timestamp].userDefinedClass == oldClass)
        {
            data[timestamp].userDefinedClass = aClass{"", ""};
            updated = true;
        }
        if (data[timestamp].detectedClass == oldClass)
        {
            data[timestamp].detectedClass = aClass{"", ""};
            updated = true;
        }

        if (updated)
            updatedVectors[timestamp] = data[timestamp];
    }

    setDataChanged(true);
    emit labelsUpdated(updatedVectors);
}

void MeasurementData::changeClass(aClass oldClass, aClass newClass)
{
    Q_ASSERT("Trying to change class that does not exist!" && classList.contains(oldClass));
    Q_ASSERT("Trying to change class to new class that already exists!" && !classList.contains(newClass));

    int index = classList.indexOf(oldClass);
    classList[index] = newClass;

    // update measurement data
    QMap<uint, MVector> updatedVectors;

    for (uint timestamp: data.keys())
    {
        bool updated = false;   // flag for updated class

        if (data[timestamp].userDefinedClass == oldClass)
        {
            data[timestamp].userDefinedClass = newClass;
            updated = true;
        }
        if (data[timestamp].detectedClass == oldClass)
        {
            data[timestamp].detectedClass = newClass;
            updated = true;
        }

        if (updated)
            updatedVectors[timestamp] = data[timestamp];
    }

    setDataChanged(true);
    emit labelsUpdated(updatedVectors);
}

QString MeasurementData::getSaveFilename() const
{
    return saveFilename;
}

QString MeasurementData::getTimestampStringFromUInt(uint timestamp)
{
    QDateTime dateTime = QDateTime::fromTime_t(timestamp);
    return dateTime.toString("d.M.yyyy - h:mm:ss");
}

uint MeasurementData::getTimestampUIntfromString(QString string)
{
    QDateTime dateTime = QDateTime::fromString(string, "d.M.yyyy - h:mm:ss");
    return dateTime.toTime_t();
}
