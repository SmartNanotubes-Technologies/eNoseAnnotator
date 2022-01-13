#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include <QtCore>
#include <QDialog>

#include "bargraphwidget_old.h"

namespace Ui {
class GeneralSettings;
}

class GeneralSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GeneralSettingsDialog(QWidget *parent = nullptr);
    ~GeneralSettingsDialog();

    double getMaxVal() const;
    void setMaxVal(double value);


    double getMinVal() const;
    void setMinVal(double value);

    bool getUseLimits() const;
    void setUseLimits(bool value);

    QString getPresetDir() const;
    void setPresetDir(QString presetDir);

    bool getRunAutoSaveEnabled() const;
    void setRunAutoSaveEnabled(bool enabled);

    uint getRunAutoSaveInterval() const;
    void setRunAutoSaveInterval(uint minutes);


private slots:
    void on_buttonBox_accepted();


    void on_presetDirPushButton_clicked();

    void on_defaultPushButton_clicked();

    void onSaveCheckboxStateChanged(int state);
    void setRunAutoIntervalEnabled(bool enabled);

private:
    Ui::GeneralSettings *ui;
    double maxVal;
    double minVal;
    bool useLimits = true;
};

#endif // GENERALSETTINGS_H
