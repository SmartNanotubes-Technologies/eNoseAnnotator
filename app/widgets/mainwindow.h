#ifndef MAINWINDOW_H
#define MAINWINDOW_H
// for windows build
#define DLPACK_EXPORTS

#include <QtCore>
#include <QMainWindow>
#include <QCloseEvent>
#include <QLabel>

#include "../classes/measurementdata.h"
#include "../classes/datasource.h"
#include "../classes/mvector.h"
#include "../classes/torchclassifier.h"
#include "linegraphwidget.h"
#include "bargraphwidget.h"
#include "infowidget.h"
#include "classifierwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionSave_Data_triggered();

    void on_actionsave_selection_triggered();

    void on_actionLoad_triggered();

    void on_actionSet_USB_Connection_triggered();

    void on_actionSettings_triggered();

    void on_actionStart_triggered();

    void on_actionStop_triggered();

    void on_actionReset_triggered();

    void on_actionAnnotate_selection_triggered();

    void on_actionSet_detected_class_of_selection_triggered();

    void on_actionSave_triggered();

    void on_actionAbout_triggered();

    void on_actionLoadClassifier_triggered();

private:
    Ui::MainWindow *ui;

    LineGraphWidget* funcLineGraph;
    LineGraphWidget* absLineGraph;
    LineGraphWidget* relLineGraph;
    BarGraphWidget* vectorBarGraph;
    BarGraphWidget* funcBarGraph;
    InfoWidget* measInfoWidget;
    ClassifierWidget* classifierWidget;
    QList<QDockWidget*> leftDocks;
    QList<QDockWidget*> rightDocks;

    QLabel *statusTextLabel;
    QLabel *statusImageLabel;

    MeasurementData *mData = nullptr;
    DataSource *source = nullptr;
    TorchClassifier *classifier = nullptr;

    void closeEvent (QCloseEvent *event);

    void createGraphWidgets();

    void createStatusBar();

    void clearData();

    void makeSourceConnections();

    void sensorConnected(QString sensorId);

    void setTitle(bool);

    void updateFuncGraph();

    void connectFLGraph();

protected:
  bool eventFilter(QObject *obj, QEvent *event);
};

#endif // MAINWINDOW_H
