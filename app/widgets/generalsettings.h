#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QtCore>
#include <QDialog>

#include "bargraphwidget.h"

namespace Ui {
class GeneralSettings;
}

class GeneralSettings : public QDialog
{
    Q_OBJECT

public:
    explicit GeneralSettings(QWidget *parent = nullptr);
    ~GeneralSettings();

    double getMaxVal() const;
    void setMaxVal(double value);


    double getMinVal() const;
    void setMinVal(double value);

    bool getUseLimits() const;
    void setUseLimits(bool value);

    QString getPresetDir() const;
    void setPresetDir(QString presetDir);

private slots:
    void on_buttonBox_accepted();


    void on_presetDirPushButton_clicked();

    void on_defaultPushButton_clicked();

private:
    Ui::GeneralSettings *ui;
    double maxVal;
    double minVal;
    bool useLimits = true;
};

#endif // GENERALSETTINGS_H
