#include "generalsettings.h"
#include "ui_generalsettings.h"

#include "../classes/defaultSettings.h"

#include <QFileDialog>

GeneralSettingsDialog::GeneralSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralSettings)
{
    ui->setupUi(this);
    this->setWindowTitle("Settings");

    connect(ui->save_checkbox, &QCheckBox::stateChanged, this, &GeneralSettingsDialog::onSaveCheckboxStateChanged);
}

GeneralSettingsDialog::~GeneralSettingsDialog()
{
    delete ui;
}



void GeneralSettingsDialog::on_buttonBox_accepted()
{
    maxVal = ui->maxValSpinBox->value();
    minVal = ui->minValSpinBox->value();
    useLimits = ui->useLimitsCheckBox->checkState();
}


bool GeneralSettingsDialog::getUseLimits() const
{
    return useLimits;
}

void GeneralSettingsDialog::setUseLimits(bool value)
{
    if (value)
        ui->useLimitsCheckBox->setCheckState(Qt::CheckState::Checked);
    else
        ui->useLimitsCheckBox->setCheckState(Qt::CheckState::Unchecked);

    useLimits = value;
}

double GeneralSettingsDialog::getMinVal() const
{
    return minVal;
}

void GeneralSettingsDialog::setMinVal(double value)
{
    ui->minValSpinBox->setValue(value);
    minVal = value;
}

double GeneralSettingsDialog::getMaxVal() const
{
    return maxVal;
}

void GeneralSettingsDialog::setMaxVal(double value)
{
    maxVal = value;
    ui->maxValSpinBox->setValue(value);
}

QString GeneralSettingsDialog::getPresetDir() const
{
    QString presetDir = ui->presetDirlineEdit->text();
    if (!presetDir.endsWith("/"))
        presetDir = presetDir + "/";

    return presetDir;
}

void GeneralSettingsDialog::setPresetDir(QString presetDirPath)
{
    QFileInfo presetDir(presetDirPath);
    ui->presetDirlineEdit->setText(presetDir.absolutePath());
}

bool GeneralSettingsDialog::getRunAutoSaveEnabled() const
{
    return ui->save_checkbox->isChecked();
}

void GeneralSettingsDialog::setRunAutoSaveEnabled(bool enabled)
{
    ui->save_checkbox->setChecked(enabled);

    setRunAutoIntervalEnabled(enabled);
}

void GeneralSettingsDialog::onSaveCheckboxStateChanged(int state)
{
    bool enabled = state == Qt::CheckState::Checked;
    setRunAutoSaveEnabled(enabled);
}

void GeneralSettingsDialog::setRunAutoIntervalEnabled(bool enabled)
{
    ui->save_label_2->setEnabled(enabled);
    ui->save_SpinBox->setEnabled(enabled);
    ui->save_label_3->setEnabled(enabled);
}

uint GeneralSettingsDialog::getRunAutoSaveInterval() const
{
    return static_cast<uint>(ui->save_SpinBox->value());
}

void GeneralSettingsDialog::setRunAutoSaveInterval(uint minutes)
{
    ui->save_SpinBox->setValue(minutes);
}

void GeneralSettingsDialog::on_presetDirPushButton_clicked()
{
    QString presetDir = QFileDialog::getExistingDirectory(this, "Set preset folder", ui->presetDirlineEdit->text());
    if (presetDir != "")
        ui->presetDirlineEdit->setText(presetDir);
}

void GeneralSettingsDialog::on_defaultPushButton_clicked()
{
    ui->minValSpinBox->setValue(DEFAULT_LOWER_LIMIT);
    ui->maxValSpinBox->setValue(DEFAULT_UPPER_LIMIT);
    ui->useLimitsCheckBox->setCheckState(DEFAULT_USE_LIMITS ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->presetDirlineEdit->setText(QDir(DEFAULT_PRESET_DIR).absolutePath());
}
