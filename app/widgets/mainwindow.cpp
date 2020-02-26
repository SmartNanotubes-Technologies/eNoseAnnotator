#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../classes/datasource.h"
#include "../classes/usbdatasource.h"
#include "../classes/mvector.h"

#include "functionalisationdialog.h"
#include "generalsettings.h"
#include "classselector.h"
#include "linegraphwidget.h"
#include "sourcedialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/icons/icon"));

    createGraphWidgets();

    // init statusbar
    statusTextLabel = new QLabel(statusBar());
    statusImageLabel = new QLabel(statusBar());
    statusTextLabel->setText("Sensor status: Not connected ");
    statusImageLabel->setPixmap(QPixmap(":/icons/disconnected"));
    statusImageLabel->setScaledContents(true);
    statusImageLabel->setMaximumSize(16,16);
    statusBar()->addPermanentWidget(statusTextLabel);
    statusBar()->addPermanentWidget(statusImageLabel);

    // prepare menubar
    ui->actionStart->setEnabled(false);
    ui->actionReset->setEnabled(false);
    ui->actionStop->setEnabled(false);

    ui->actionAnnotate_selection->setEnabled(false);
    ui->actionSet_detected_class_of_selection->setEnabled(false);

    // user can only set detected class in debug mode
    #ifdef QT_NO_DEBUG
    ui->actionSet_detected_class_of_selection->setVisible(false);
    #endif


    mData = new MeasurementData();

    // this->setStyleSheet("QSplitter::handle{background: black;}"); // make splitter visible

    // relative graph: ignore limits (minVal, maxVal)
    relLineGraph->setUseLimits(false);

    // connections:
    // connect funcLineGraph, lGraph & absLGraph
    // xAxis
    connect(relLineGraph, &LineGraphWidget::xRangeChanged, absLineGraph, &LineGraphWidget::setXRange);
    connect(absLineGraph, &LineGraphWidget::xRangeChanged, relLineGraph, &LineGraphWidget::setXRange);
    connect(funcLineGraph, &LineGraphWidget::xRangeChanged, relLineGraph, &LineGraphWidget::setXRange);
    connect(relLineGraph, &LineGraphWidget::xRangeChanged, funcLineGraph, &LineGraphWidget::setXRange);

    // selection
    connect(relLineGraph, &LineGraphWidget::dataSelectionChanged, absLineGraph, &LineGraphWidget::setSelection);
    connect(relLineGraph, &LineGraphWidget::selectionCleared, absLineGraph, &LineGraphWidget::clearSelection);
    connect(absLineGraph, &LineGraphWidget::dataSelectionChanged, relLineGraph, &LineGraphWidget::setSelection);
    connect(absLineGraph, &LineGraphWidget::selectionCleared, this, [this](){
        // illegal data selection: force selectionCleared signal
        relLineGraph->setSelection(QCPDataSelection(QCPDataRange(2,1)));
    });
    connect(funcLineGraph, &LineGraphWidget::dataSelectionChanged, absLineGraph, &LineGraphWidget::setSelection);
    connect(funcLineGraph, &LineGraphWidget::selectionCleared, absLineGraph, &LineGraphWidget::clearSelection);
    connect(absLineGraph, &LineGraphWidget::dataSelectionChanged, funcLineGraph, &LineGraphWidget::setSelection);
    connect(absLineGraph, &LineGraphWidget::selectionCleared, funcLineGraph, &LineGraphWidget::clearSelection);

    // selection flow
    connect(relLineGraph, &LineGraphWidget::selectionChanged, mData, &MeasurementData::setSelection); // change selection in mData

    connect(relLineGraph, &LineGraphWidget::selectionCleared, mData, &MeasurementData::clearSelection); // clear selection in mData
    connect(relLineGraph, &LineGraphWidget::selectionCleared, vectorBarGraph, &BarGraphWidget::clearBars);  // clear vector in vectorBarGraph
    connect(relLineGraph, &LineGraphWidget::selectionCleared, funcBarGraph, &BarGraphWidget::clearBars);  // clear vector in funcBarGraph
    connect(mData, &MeasurementData::selectionVectorChanged, vectorBarGraph, &BarGraphWidget::setBars);   // plot vector in vectorBarGraph
    connect(mData, &MeasurementData::selectionVectorChanged, funcBarGraph, &BarGraphWidget::setBars);   // plot vector in funcBarGraph
    connect(mData, &MeasurementData::selectionCleared, vectorBarGraph, &BarGraphWidget::clearBars); // clear vector in vectorBarGraph
    connect(mData, &MeasurementData::selectionCleared, funcBarGraph, &BarGraphWidget::clearBars); // clear vector in funcBarGraph
    connect(mData, &MeasurementData::lgClearSelection, relLineGraph, &LineGraphWidget::clearSelection);
    connect(mData, &MeasurementData::lgClearSelection, funcLineGraph, &LineGraphWidget::clearSelection);

    connect(mData, &MeasurementData::selectionVectorChanged, this, [this](MVector selectionVector){
        if (classifier != nullptr)
        {
            auto funcVector = selectionVector.getFuncVector(mData->getFunctionalities(), mData->getSensorFailures());

            Annotation annotation = classifier->getAnnotation(funcVector.vector);
            classifierWidget->setAnnotation(annotation);
            classifierWidget->setInfoString("Class of selection");
            classifierWidget->isSelectionAnnotation = true;
        }
    }); // classify selection
    connect(mData, &MeasurementData::selectionCleared, classifierWidget, &ClassifierWidget::clearAnnotation); // clear selection classification

    connect(mData, &MeasurementData::labelsUpdated, funcLineGraph, &LineGraphWidget::labelSelection); // draw selection and classes
    connect(mData, &MeasurementData::labelsUpdated, absLineGraph, &LineGraphWidget::labelSelection); // draw selection and classes
    connect(mData, &MeasurementData::labelsUpdated, relLineGraph, &LineGraphWidget::labelSelection); // draw selection and classes

    connect(mData, &MeasurementData::selectionVectorChanged, this, [this](MVector, std::array<bool, MVector::nChannels>){
        ui->actionAnnotate_selection->setEnabled(true);
        ui->actionSet_detected_class_of_selection->setEnabled(true);
    }); // show classification actions
    connect(mData, &MeasurementData::selectionCleared, this, [this](){
        ui->actionAnnotate_selection->setEnabled(false);
        ui->actionSet_detected_class_of_selection->setEnabled(false);
    }); // hide classification actions

    // reset graphs
    connect(mData, &MeasurementData::dataReset, this, [this]{
        funcLineGraph->clearGraph();
        relLineGraph->clearGraph();
        absLineGraph->clearGraph();
        vectorBarGraph->clearBars();
        funcBarGraph->clearBars();
    }); // clear all graphs when data is reset

    // new data
    connect(mData, &MeasurementData::dataAdded, relLineGraph, &LineGraphWidget::addMeasurement);    // add new data to lGraph
    connect(mData, &MeasurementData::dataAdded, this, [this](MVector vector, uint timestamp, bool yRescale){
        MVector funcVector = vector.getFuncVector(mData->getFunctionalities(), mData->getSensorFailures());
        funcLineGraph->addMeasurement(funcVector, timestamp, yRescale);
    });    // add new data to func line graph
    connect(mData, &MeasurementData::absoluteDataAdded, absLineGraph, &LineGraphWidget::addMeasurement); // add new absolute measruement
    connect(mData, &MeasurementData::dataSet, relLineGraph, &LineGraphWidget::setData);     // set loaded data in lGraph
    connect(mData, &MeasurementData::dataSet, this, [this](QMap<uint, MVector> data){
        auto funcs = mData->getFunctionalities();
        auto failures = mData->getSensorFailures();

        // add recalculated functionalitisation averages to cleared funcLineGraph
        QMap<uint, MVector> funcData;
        for (int timestamp : data.keys())
            funcData[timestamp] = data[timestamp].getFuncVector(funcs, failures);

        funcLineGraph->setData(funcData);    });     // set loaded data in lGraph

    connect(mData, &MeasurementData::setReplotStatus, funcLineGraph, &LineGraphWidget::setReplotStatus);   // replotStatus
    connect(mData, &MeasurementData::setReplotStatus, relLineGraph, &LineGraphWidget::setReplotStatus);   // replotStatus
    connect(mData, &MeasurementData::setReplotStatus, absLineGraph, &LineGraphWidget::setReplotStatus);   // replotStatus
    connect(mData, &MeasurementData::dataAdded, this, [this](MVector vector, uint timestamp, bool){
        // no classifier loaded or no live measurement running
        // -> don't classify
        bool measRunning = source != nullptr && source->status() == DataSource::Status::RECEIVING_DATA;
        if (classifier == nullptr || (measRunning && !classifierWidget->getIsLive()))
            return;

        // get annotation from the classifier
        auto funcVector = vector.getFuncVector(mData->getFunctionalities(), mData->getSensorFailures());
        Annotation annotation = classifier->getAnnotation(funcVector.vector);

        // set annotation
        mData->setDetectedAnnotation(annotation, timestamp);

        // don't show in classifier widget if selection is made
        if (!classifierWidget->isSelectionAnnotation)
        {
            classifierWidget->setAnnotation(annotation);
            classifierWidget->setInfoString("Live classification is running...");
        }
    });     // live classification


    // measurement data changed
    connect(mData, &MeasurementData::dataChangedSet, this, [this](bool dataChanged){
        setTitle(dataChanged);
    });

    // sensor failures detected
    connect(absLineGraph, &LineGraphWidget::sensorFailure, this, [this](int channel){
        auto failures = mData->getSensorFailures();
        if (!failures[channel])
        {
            qDebug() << "sensor failure detected in channel " << QString::number(channel);
            failures[channel] = true;
            mData->setFailures(failures);
            measInfoWidget->setFailures(failures);
        }
    }); // absGraph -> mData
    connect(mData, &MeasurementData::sensorFailuresSet, relLineGraph, &LineGraphWidget::setSensorFailureFlags);    // mData: failures changed -> lGraph: update sensr failures
    connect(mData, &MeasurementData::sensorFailuresSet, absLineGraph, &LineGraphWidget::setSensorFailureFlags);    // mData: failures changed -> absLGraph: update sensor failures
    connect(mData, &MeasurementData::sensorFailuresSet, this, [this](std::array<bool, 64>){
        funcLineGraph->clearGraph();

        auto data = mData->getRelativeData();
        auto funcs = mData->getFunctionalities();
        auto failures = mData->getSensorFailures();

        // add recalculated functionalitisation averages to cleared funcLineGraph
        QMap<uint, MVector> funcData;
        for (int timestamp : data.keys())
            funcData[timestamp] = data[timestamp].getFuncVector(funcs, failures);

        funcLineGraph->setData(funcData);
    });     // update functionalisation graph

    // redraw line graphs
    connect(funcLineGraph, &LineGraphWidget::requestRedraw, this, [this]{
        // re-add data to graph
        funcLineGraph->clearGraph();

        auto data = mData->getRelativeData();
        auto funcs = mData->getFunctionalities();
        auto failures = mData->getSensorFailures();

        // add recalculated functionalitisation averages to cleared funcLineGraph
        QMap<uint, MVector> funcData;
        for (int timestamp : data.keys())
            funcData[timestamp] = data[timestamp].getFuncVector(funcs, failures);

        funcLineGraph->setData(funcData);
    }); // funcLineGraph requested redraw
    connect(relLineGraph, &LineGraphWidget::requestRedraw, this, [this]{
        // re-add data to graph
        relLineGraph->setData(mData->getRelativeData());
    }); // relLineGraph requested redraw
    connect(absLineGraph, &LineGraphWidget::requestRedraw, this, [this]{
        // re-add data to graph
        absLineGraph->setData(mData->getAbsoluteData());
    }); // absLineGraph requested redraw

    // sensor failures changed
    connect(mData, &MeasurementData::sensorFailuresSet, this, [this]{
        MVector selectionVector = mData->getSelectionVector();

        vectorBarGraph->setBars(selectionVector, mData->getSensorFailures(), mData->getFunctionalities());
        funcBarGraph->setBars(selectionVector, mData->getSensorFailures(), mData->getFunctionalities());
    }); // reset bars when sensorFailures changed

    // measurement info
    // info -> mData
    connect(measInfoWidget, &InfoWidget::mCommentChanged, mData, &MeasurementData::setComment);    // comment changed in infoWidget: change comment in mData
    connect(measInfoWidget, SIGNAL(failuresChanged(std::array<bool, 64>)), mData, SLOT(setFailures(std::array<bool, 64>)));   // failures changed in infoWidget: change failures in mData

    // mData -> info
    connect(mData, &MeasurementData::sensorIdSet, measInfoWidget, &InfoWidget::setSensor);            // mData: sensorId changed -> InfoWidget: show new sensorId
    connect(mData, &MeasurementData::startTimestempSet, measInfoWidget, &InfoWidget::setTimestamp);   // mData: timestamp changed -> InfoWidget: show new timestamp
    connect(mData, &MeasurementData::commentSet, measInfoWidget, &InfoWidget::setMComment);            // mData: comment changed -> InfoWidget: show new comment
    connect(mData, &MeasurementData::sensorFailuresSet, measInfoWidget, &InfoWidget::setFailures);    // mData: failures changed -> InfoWidget: show new failures

    // set functionalities dialog
    connect(measInfoWidget, &InfoWidget::setFunctionalities, this, [this](){
        FunctionalisationDialog dialog;

        dialog.setFunctionalities(mData->getFunctionalities());

        if (dialog.exec())
        {
            mData->setFunctionalities(dialog.getFunctionalities());
        }
    });

    // saving images of the graphs
    connect(relLineGraph, &LineGraphWidget::ImageSaveRequested, this, [this](){
        // create export folder
        if(!QDir ("./export").exists())
            QDir().mkdir("./export");

        // save file dialog
        QString selectedExtension;
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Line Graph Image"), "./export", "Image Files (*.png *.jpg *.jpeg *.bmp)", &selectedExtension);

        // save image
        if (!filename.isEmpty())
        {
            // add extension if none was set
            QStringList splitList = filename.split(".");
            if (splitList.size() < 2)
                filename += ".jpg";
            // unknown file extension
            else if (splitList.last() != "png" && splitList.last() != "jpg" && splitList.last() != "jpeg" && splitList.last() != "bmp")
                filename += ".jpg";

            relLineGraph->saveImage(filename);
        }
    });

    connect(vectorBarGraph, &BarGraphWidget::imageSaveRequested, this, [this](){
        // create export folder
        if(!QDir ("./export").exists())
            QDir().mkdir("./export");

        // save file dialog
        QString selectedExtension;
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Line Graph Image"), "./export", "Image Files (*.png *.jpg *.jpeg *.bmp)", &selectedExtension);

        // save image
        if (!filename.isEmpty())
        {
            // add extension if none was set
            QStringList splitList = filename.split(".");
            if (splitList.size() < 2)
                filename += ".jpg";
            // unknown file extension
            else if (splitList.last() != "png" && splitList.last() != "jpg" && splitList.last() != "jpeg" && splitList.last() != "bmp")
                filename += ".jpg";

            QMessageBox::StandardButton answer = QMessageBox::question(this, "Save Data", "Do you want to save the selection data that created the bar graph alongside the image?");

            if (answer == QMessageBox::StandardButton::Yes)
            {
                // dataFilemame = filename - image extension + ".csv"
                QStringList list = filename.split(".");
                QString dataFilename = list.mid(0, list.length()-1).join(".") + ".csv";
                mData->saveSelection(this, dataFilename);
            }

            vectorBarGraph->saveImage(filename);
        }
    });

    connect(funcBarGraph, &BarGraphWidget::imageSaveRequested, this, [this](){
        // create export folder
        if(!QDir ("./export").exists())
            QDir().mkdir("./export");

        // save file dialog
        QString selectedExtension;
        QString filename = QFileDialog::getSaveFileName(this, tr("Save Line Graph Image"), "./export", "Image Files (*.png *.jpg *.jpeg *.bmp)", &selectedExtension);

        // save image
        if (!filename.isEmpty())
        {
            // add extension if none was set
            QStringList splitList = filename.split(".");
            if (splitList.size() < 2)
                filename += ".jpg";
            // unknown file extension
            else if (splitList.last() != "png" && splitList.last() != "jpg" && splitList.last() != "jpeg" && splitList.last() != "bmp")
                filename += ".jpg";

            QMessageBox::StandardButton answer = QMessageBox::question(this, "Save Data", "Do you want to save the selection data that created the bar graph alongside the image?");

            if (answer == QMessageBox::StandardButton::Yes)
            {
                // dataFilemame = filename - image extension + ".csv"
                QStringList list = filename.split(".");
                QString dataFilename = list.mid(0, list.length()-1).join(".") + ".csv";
                mData->saveSelection(this, dataFilename);
            }

            funcBarGraph->saveImage(filename);
        }
    });

    // functionalisation changed
    connect(mData, &MeasurementData::functionalisationChanged, this, &MainWindow::updateFuncGraph); // recalculate func line graph
}

MainWindow::~MainWindow()
{
    delete ui;

    delete mData;
    delete source;
    delete classifier;
}

void MainWindow::setTitle(bool dataChanged)
{
    QString filename = mData->getSaveFilename();

    QString title;
    QString titleExtension;

    if (filename != "")
    {
        QDir dataDir{"./data"};
        QFileInfo fileInfo(filename);
        QString extension;
        if (fileInfo.absolutePath().startsWith(dataDir.absolutePath()))
            titleExtension = " - " + dataDir.relativeFilePath(filename);
        else
            titleExtension = " - .../" + fileInfo.fileName();
    }

   if (dataChanged)
   {
        ui->actionSave->setEnabled(true);
        title = "eNoseAnnotator*";
   } else
   {
       ui->actionSave->setEnabled(false);
       title = "eNoseAnnotator";
   }
   this->setWindowTitle(title + titleExtension);
}

void MainWindow::on_actionSave_Data_triggered()
{
    mData->saveData(this);
}

void MainWindow::on_actionsave_selection_triggered()
{
    mData->saveSelection(this);
}

void MainWindow::on_actionLoad_triggered()
{
    mData->loadData(this);
    setTitle(false);

}

void MainWindow::on_actionSet_USB_Connection_triggered()
{
    SourceDialog* dialog = new SourceDialog(static_cast<QWidget*>(this->parent()));

    // source was set before
    if (source != nullptr)
    {
        // set previous settings
        dialog->setSensorId(mData->getSensorId());

        // usb specific
        if (source->sourceType() == DataSource::SourceType::USB)
        {
            USBDataSource::Settings usbSettings;
            usbSettings.portName = source->identifier();

            dialog->setSourceType(DataSource::SourceType::USB);
            dialog->setUSBSettings(usbSettings);
        }
        else
            Q_ASSERT ("Unknown source selected!" && false);
    }

    if (dialog->exec())
    {
        DataSource::SourceType sourceType = dialog->getSourceType();
        QString identifier = dialog->getIdentifier();
        QString sensorId =dialog->getSensorId();

        // source was set before
        if (source != nullptr)
        {
            // if new source type or identifier:
            if (identifier != source->identifier() || sourceType != source->sourceType())
            {
                // measurement running:
                // -> ask if measurement should be stopped
                if (source->status() == DataSource::Status::RECEIVING_DATA)
                {
                    QMessageBox::StandardButton answer = QMessageBox::question(this, "Stop Measurement", "You selected a new connection while a measurement is running.\nDo you want to stop the measurement in order to use the new connection?");

                    if (answer != QMessageBox::StandardButton::Yes)
                        return; // keep current data source

                    source->stop();
                }
                // delete old source
                delete source;
                source = nullptr;
            }
            // only sensor Id was changed
            else if (sensorId != mData->getSensorId())
            {
                mData->setSensorId(sensorId);
            }
        }

        // source not set before or just deleted:
        // -> init new source
        // usb source:
        if (source == nullptr)
        {
            if (sourceType == DataSource::SourceType::USB)
            {
                USBDataSource::Settings settings;
                settings.portName = identifier;

                source = new USBDataSource(settings);
            }

            // make connections
            makeSourceConnections();

            // update ui
            sensorConnected(sensorId);
        }
    }
}

void MainWindow::on_actionSettings_triggered()
{
    GeneralSettings dialog;

    // set current settings
    dialog.setMaxVal(absLineGraph->getMaxVal());   // max value for absolute values
    dialog.setMinVal(absLineGraph->getMinVal());   // min value for absolute values
    dialog.setUseLimits(absLineGraph->getUseLimits());


    if (dialog.exec())
    {
        // get new settings

        // --- limits ---
        // get limits
        int newMaxVal = dialog.getMaxVal();
        int newMinVal = dialog.getMinVal();
        int oldMaxVal = absLineGraph->getMaxVal();
        int oldMinVal = absLineGraph->getMinVal();

        // get useLimits
        bool newUseLimits = dialog.getUseLimits();
        bool oldUseLimits = absLineGraph->getUseLimits();

        // recalc sensor failure flags if limits or useLimits changed
        bool limitsChanged = (newMaxVal != oldMaxVal) || (newMaxVal != oldMinVal);
        bool useLimitsChanged = newUseLimits != oldUseLimits;

        auto sensorFailureFlags = mData->getSensorFailures();

        // 4 cases for change
        // 1. useLimits: true -> false:
        //      set all sensorFailureFlags added by limit violations to false
        // 2. useLimits: false -> true:
        //      find all limit violations and set the according flags to true
        // 3. limits: minVal gets bigger or maxVal smaller
        //      find old violations that are no violations anymore and set flag to false
        // 4. limits: minVal gets smaller or maxVal bigger
        //      find new violations that were no violations and set flag  to true
        if (limitsChanged || useLimitsChanged)
        {
            auto dataMap = mData->getAbsoluteData();

            for (MVector vector : dataMap)
            {
                for (int i = 0; i<MVector::nChannels; i++)
                {
                    // case 1+2
                    if (useLimitsChanged)
                    {
                        if (vector[i] < newMinVal || vector[i] > newMaxVal)
                            sensorFailureFlags[i] = newUseLimits;   // useLimits == true -> set flags, else delete them
                    }
                    // case 3+4
                    else    // limitsChanged
                    {
                        // minVal changed
                        if (newMinVal < oldMinVal)  // case 4
                        {
                            for (int i=0; i<MVector::nChannels; i++)
                                if (vector[i] >= newMinVal && vector[i] < oldMinVal)
                                    sensorFailureFlags[i] = false;
                        } else if (newMinVal > oldMinVal)   // case 3
                        {
                            for (int i=0; i<MVector::nChannels; i++)
                                if (vector[i] < newMinVal && vector[i] >= oldMinVal)
                                    sensorFailureFlags[i] = true;
                        }

                        // maxVal changed
                        if (newMaxVal > oldMaxVal)  // case 4
                        {
                            for (int i=0; i<MVector::nChannels; i++)
                                if (vector[i] <= newMaxVal && vector[i] > oldMaxVal)
                                    sensorFailureFlags[i] = false;
                        } else if (newMaxVal < oldMaxVal)   // case 3
                        {
                            for (int i=0; i<MVector::nChannels; i++)
                                if (vector[i] > newMaxVal && vector[i] <= oldMaxVal)
                                    sensorFailureFlags[i] = true;
                        }
                    }

                }
            }

            // set new values
            absLineGraph->setMaxVal(newMaxVal);
            absLineGraph->setMinVal(newMinVal);
            mData->setSensorFailures(sensorFailureFlags);
            absLineGraph->setUseLimits(newUseLimits);
        }
    }
}

void MainWindow::on_actionStart_triggered()
{
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);
    Q_ASSERT("Error: Cannot start non-active connection!" && ((source->status() == DataSource::Status::CONNECTED) || (source->status() == DataSource::Status::CONNECTION_ERROR)));

    // reconnect
    if (source->status() == DataSource::Status::CONNECTION_ERROR)
    {
        qDebug() << "Reconnecting Sensor \"" << source->identifier() << "\"";
        source->reconnect();
    }

    if (mData->changed())
    {
        // save old data
        auto answer = QMessageBox::question(this, "Save measurement data", "Do you want to save the current data before starting a new measurement?");
        if (answer == QMessageBox::StandardButton::Yes)
            mData->saveData(this);
    }

    // clear data, init with sensor id, set dataChanged to false
    QString sensorId = mData->getSensorId();
    clearData();
    mData->setSensorId(sensorId);
    mData->setDataChanged(false);

    source->start();

    qDebug() << "New measurement started!";
}

void MainWindow::on_actionStop_triggered()
{
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);
    Q_ASSERT("Error: Cannot stop non-active connection!" && source->status() == DataSource::Status::RECEIVING_DATA);
    if (source->sourceType() == DataSource::SourceType::USB)
        Q_ASSERT("Error: Connection is not open!" && static_cast<USBDataSource*>(source)->getSerial()->isOpen());

    source->stop();
}

void MainWindow::on_actionReset_triggered()
{
    Q_ASSERT("Error: No connection was specified!" && source!=nullptr);
    Q_ASSERT("Error: Cannot stop non-active connection!" && source->status() == DataSource::Status::RECEIVING_DATA);
    if (source->sourceType() == DataSource::SourceType::USB)
        Q_ASSERT("Error: Connection is not open!" && static_cast<USBDataSource*>(source)->getSerial()->isOpen());

    source->reset();
}

void MainWindow::on_actionAnnotate_selection_triggered()
{
    Q_ASSERT(!mData->getSelectionMap().isEmpty());

    ClassSelector* dialog = new ClassSelector(this);
    dialog->setWindowTitle("Select class of selection");
    dialog->setSelectedAnnotation(mData->getSelectionMap().first().userAnnotation);

    // connections
    connect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    connect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    connect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);

    if (dialog->exec())
    {
        Annotation annotation = dialog->getAnnotation();
        mData->setUserAnnotationOfSelection(annotation);
    }

    // disconnect
    disconnect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    disconnect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    disconnect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);
}

void MainWindow::on_actionSet_detected_class_of_selection_triggered()
{
    Q_ASSERT(!mData->getSelectionMap().isEmpty());

    ClassSelector* dialog = new ClassSelector(this);
    dialog->setWindowTitle("[Debug] Select class of detected selection");
    dialog->setSelectedAnnotation(mData->getSelectionMap().first().userAnnotation);

    // connections
    connect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    connect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    connect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);

    if (dialog->exec())
    {
        Annotation annotation = dialog->getAnnotation();
        mData->setDetectedAnnotationOfSelection(annotation);
    }

    // disconnect
    disconnect(dialog, &ClassSelector::addClass, mData, &MeasurementData::addClass);
    disconnect(dialog, &ClassSelector::removeClass, mData, &MeasurementData::removeClass);
    disconnect(dialog, &ClassSelector::changeClass, mData, &MeasurementData::changeClass);
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if (mData->changed())
    {
        QMessageBox::StandardButton resBtn = QMessageBox::question( this, "eNoseAnnotator",
                                                                    tr("The measurement data was changed without saving.\nDo you want to save the measurement before leaving?\n"),
                                                                   QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes)
        {
            mData->saveData(this);
            event->accept();
        } else if (resBtn == QMessageBox::Cancel)
        {
            event->ignore();
        } else
        {
            event->accept();
        }
    }
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::on_actionSave_triggered()
{
    QString filename = mData->getSaveFilename();
    if (filename == "")
        mData->saveData(this);
    else
        mData->saveData(this, filename);
}

void MainWindow::on_actionAbout_triggered()
{
    QString iconCredits = "Application Icon made by: Timo Land<br>USB icons from <a href='https://www.icons8.de'>Icons8</a><br>All other icons made by <a href='https://smashicons.com'>SmashIcons</a> from <a href='https://www.flaticon.com'>www.flaticon.com</a>";

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About eNoseAnnotator");
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    msgBox.setText(iconCredits);
    msgBox.exec();
}

void MainWindow::clearData()
{
    mData->clear();
    relLineGraph->clearGraph();
    absLineGraph->clearGraph();
}

void MainWindow::sensorConnected(QString sensorId)
{
    // mData
    mData->setSensorId(sensorId);

    // info widget
    measInfoWidget->setSensor(sensorId);
    measInfoWidget->setStatus(DataSource::Status::CONNECTING);

    // tool bar
    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(false);
    ui->actionReset->setEnabled(false);
    ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));

    // status bar
    statusTextLabel->setText("Sensor Status: Connecting...");
    statusImageLabel->setPixmap(QPixmap(":/icons/baseVector"));
}

void MainWindow::makeSourceConnections()
{
    connect(source, &DataSource::baseVectorSet, mData, &MeasurementData::setBaseLevel);
    connect(source, &DataSource::vectorReceived, mData, [this] (uint timestamp, MVector vector) {
        mData->addMeasurement(timestamp, vector);

        if (measInfoWidget->statusSet != DataSource::Status::RECEIVING_DATA)
        {
            statusTextLabel->setText("Sensor Status: Receiving Data");
            statusImageLabel->setPixmap(QPixmap(":/icons/recording"));
        }
    } );    // new measurement

    connect(source, &DataSource::error, this, [this] (QString errorString) {
        QMessageBox::critical(this, "Connection Error", errorString);
    }); // error

    connect(source, &DataSource::statusSet, measInfoWidget, &InfoWidget::setStatus);
    connect(source, &DataSource::statusSet, this, [this](DataSource::Status status){
        switch (status) {
            case DataSource::Status::NOT_CONNECTED:
                ui->actionStart->setEnabled(false);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));
                statusTextLabel->setText("Sensor Status: Not Connected");
                statusImageLabel->setPixmap(QPixmap(":/icons/disconnected"));
                break;
            case DataSource::Status::CONNECTING:
                ui->actionStart->setEnabled(false);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));
                statusTextLabel->setText("Sensor Status: Connecting...");
                statusImageLabel->setPixmap(QPixmap(":/icons/baseVector"));
                break;
            case DataSource::Status::CONNECTED:
                ui->actionStart->setEnabled(true);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
                statusTextLabel->setText("Sensor Status: Connected");
                statusImageLabel->setPixmap(QPixmap(":/icons/connected"));
                break;
            case DataSource::Status::SET_BASEVECTOR:
                ui->actionStart->setEnabled(false);
                ui->actionStop->setEnabled(true);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
                statusTextLabel->setText("Sensor Status: Setting Base Vector (R0)...");
                statusImageLabel->setPixmap(QPixmap(":/icons/baseVector"));
                break;
            case DataSource::Status::RECEIVING_DATA:
                ui->actionStart->setEnabled(false);
                ui->actionStop->setEnabled(true);
                ui->actionReset->setEnabled(true);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/connected"));
                statusTextLabel->setText("Sensor Status: Receiving Data");
                statusImageLabel->setPixmap(QPixmap(":/icons/recording"));
                break;
            case DataSource::Status::CONNECTION_ERROR:
                ui->actionStart->setEnabled(true);
                ui->actionStop->setEnabled(false);
                ui->actionReset->setEnabled(false);
                ui->actionSet_USB_Connection->setIcon(QIcon(":/icons/disconnected"));
                statusTextLabel->setText("Sensor Status: Error");
                statusImageLabel->setPixmap(QPixmap(":/icons/error"));
                break;
            default:
                Q_ASSERT("Unknown Sensor Status!" && false);
        }
    });
}

void MainWindow::createGraphWidgets()
{
    setDockNestingEnabled(true);

    // create graph widgets & their docks
    // func line graph
    QDockWidget *flgdock = ui->dock1;
    funcLineGraph = new LineGraphWidget;  // init func line Graph
    flgdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    flgdock->setWidget(funcLineGraph);
    addDockWidget(Qt::LeftDockWidgetArea, flgdock);
    leftDocks << flgdock;


    // relative line graph
    QDockWidget *rlgdock = new QDockWidget(tr("Relative Line Graph"), this);
    relLineGraph = new LineGraphWidget;
    rlgdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    rlgdock->setWidget(relLineGraph);
    addDockWidget(Qt::LeftDockWidgetArea, rlgdock);
    leftDocks << rlgdock;
    rlgdock->hide();

    // absolute line graph
    QDockWidget *algdock = new QDockWidget(tr("Absolute Line Graph"), this);
    absLineGraph = new LineGraphWidget;
    absLineGraph->setIsAbsolute(true);
    algdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    algdock->setWidget(absLineGraph);
    addDockWidget(Qt::LeftDockWidgetArea, algdock);
    leftDocks << algdock;
    algdock->hide();

    // vector bar graph
    QDockWidget *vbgdock = ui->dock2;
    vectorBarGraph = new BarGraphWidget;
    vectorBarGraph->setMode(BarGraphWidget::Mode::showAll);
    vbgdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    vbgdock->setWidget(vectorBarGraph);
    addDockWidget(Qt::LeftDockWidgetArea, vbgdock);
    leftDocks << vbgdock;
    vbgdock->hide();

    // functionalisation bar graph
    QDockWidget *fbgdock = new QDockWidget(tr("Functionalisation Bar Graph"), this);
    funcBarGraph = new BarGraphWidget;
    funcBarGraph->setMode(BarGraphWidget::Mode::showFunc);
    fbgdock->setAllowedAreas(Qt::LeftDockWidgetArea);
    fbgdock->setWidget(funcBarGraph);
    addDockWidget(Qt::LeftDockWidgetArea, fbgdock);
    leftDocks << fbgdock;

    // add actions to view menu
    ui->menuView->addAction(flgdock->toggleViewAction());
    ui->menuView->addAction(fbgdock->toggleViewAction());
    ui->menuView->addAction(rlgdock->toggleViewAction());
    ui->menuView->addAction(algdock->toggleViewAction());
    ui->menuView->addAction(vbgdock->toggleViewAction());

    // create tabs
    tabifyDockWidget(algdock, rlgdock);
    tabifyDockWidget(rlgdock, flgdock);
    tabifyDockWidget(fbgdock, vbgdock);

    // right widgets
    measInfoWidget = static_cast<InfoWidget*>(ui->infoWidget);
    classifierWidget = static_cast<ClassifierWidget*>(ui->classifierInfoWidget);

    int dockWidth = 0.7 * window()->size().width();
    for (auto dock : leftDocks)
        dock->resize(dockWidth, dock->size().height());

}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    auto resizedDock = static_cast<QDockWidget*>(obj);

  if (event->type() == QEvent::Resize && leftDocks.contains(resizedDock))
  {
        auto resizeEvent = static_cast<QResizeEvent*>(event);
        int newWidth = window()->size().width() - resizeEvent->size().width() - 5;
//        measInfoWidget->resize(newWidth, measInfoWidget->size().width());
//        classifierWidget->resize(newWidth, classifierWidget->size().width());
  }
  return QWidget::eventFilter(obj, event);
}

void MainWindow::on_actionLoadClassifier_triggered()
{
    // make data directory
    if (!QDir("classifiers").exists())
        QDir().mkdir("classifiers");

    // get path to classifier
    QString filename = QFileDialog::getOpenFileName(this, tr("Load smell classifier"), "./classifiers/", "TorchScript Files (*.pt)");
    if (filename == "")
        return;

    if (classifier != nullptr)
        delete classifier;

    bool loadOk;
    QString errorString;
    classifier = new TorchClassifier(this, filename, &loadOk, &errorString);

    if (!loadOk)
    {
        QMessageBox::critical(this, "Error loading model", errorString);
        delete classifier;
        classifier = nullptr;
        return;
    }

    classifierWidget->setClassifier(classifier->getName(), classifier->getClassNames(), classifier->getIsInputAbsolute());

    if (!mData->getAbsoluteData().empty())
    {
        auto ans = QMessageBox::question(this, "Classify data", "Do you want to classify the current measurement data?");

        if (ans == QMessageBox::StandardButton::Yes)
        {
            // get measurement data
            QMap<uint, MVector> measDataMap;
            if (classifier->getIsInputAbsolute())
                measDataMap = mData->getAbsoluteData();
            else
                measDataMap = mData->getRelativeData();

            // classify
            for (uint timestamp : measDataMap.keys())
            {
                auto funcVector = measDataMap[timestamp].getFuncVector(mData->getFunctionalities(), mData->getSensorFailures());
                Annotation annotation = classifier->getAnnotation(funcVector.vector);
                mData->setDetectedAnnotation(annotation, timestamp);
            }
        }
    }

    // add classifier classes to mData
    for (QString className : classifier->getClassNames())
    {
        aClass c{className};

        if (!aClass::staticClassSet.contains(c))
            mData->addClass(c);
    }

    // TODO
    // classify data
    // start live classification
}

void MainWindow::updateFuncGraph()
{
    auto data = mData->getRelativeData();
    auto fails = mData->getSensorFailures();
    auto funcs = mData->getFunctionalities();

    // get number of funcs
    auto funcMap = MeasurementData::getFuncMap(funcs, fails);
    auto keyList = funcMap.keys();
    int maxFunc = *std::max_element(keyList.begin(), keyList.end());

    // else: reset graph
    delete funcLineGraph;
    funcLineGraph = new LineGraphWidget(this, maxFunc+1);
    leftDocks[0]->setWidget(funcLineGraph);
    connectFLGraph();   // reconnect funcLineGraph

    // add func vectors to func vector

    for (uint timestamp : data.keys())
    {
        MVector funcVector = data[timestamp].getFuncVector(funcs, fails);
        funcLineGraph->addMeasurement(funcVector, timestamp);
    }

}

/*!
 * \brief MainWindow::connectFLGraph makes connections for funcLineGraph. has to be called each time funcLineGraph is recreated.
 */
void MainWindow::connectFLGraph()
{
    // xRange
    connect(funcLineGraph, &LineGraphWidget::xRangeChanged, relLineGraph, &LineGraphWidget::setXRange);
    connect(relLineGraph, &LineGraphWidget::xRangeChanged, funcLineGraph, &LineGraphWidget::setXRange);

    // selection
    connect(funcLineGraph, &LineGraphWidget::dataSelectionChanged, absLineGraph, &LineGraphWidget::setSelection);
    connect(funcLineGraph, &LineGraphWidget::selectionCleared, absLineGraph, &LineGraphWidget::clearSelection);
    connect(absLineGraph, &LineGraphWidget::dataSelectionChanged, funcLineGraph, &LineGraphWidget::setSelection);
    connect(absLineGraph, &LineGraphWidget::selectionCleared, funcLineGraph, &LineGraphWidget::clearSelection);
    connect(mData, &MeasurementData::lgClearSelection, funcLineGraph, &LineGraphWidget::clearSelection);

    // labels
    connect(mData, &MeasurementData::labelsUpdated, funcLineGraph, &LineGraphWidget::labelSelection); // draw selection and classes

    // replot status
    connect(mData, &MeasurementData::setReplotStatus, funcLineGraph, &LineGraphWidget::setReplotStatus);   // replotStatus

    // redraw line graphs
    connect(funcLineGraph, &LineGraphWidget::requestRedraw, this, [this]{
        // re-add data to graph
        funcLineGraph->clearGraph();

        auto data = mData->getRelativeData();
        auto funcs = mData->getFunctionalities();
        auto failures = mData->getSensorFailures();

        // add recalculated functionalitisation averages to cleared funcLineGraph
        QMap<uint, MVector> funcData;
        for (int timestamp : data.keys())
            funcData[timestamp] = data[timestamp].getFuncVector(funcs, failures);

        funcLineGraph->setData(funcData);
    }); // funcLineGraph requested redraw
}
