#ifndef FUNCTIONALISATIONDIALOG_H
#define FUNCTIONALISATIONDIALOG_H

#include <QtCore>
#include <QDialog>
#include <QSpinBox>

#include "../classes/mvector.h"

namespace Ui {
class FunctionalisationDialog;
}

class FunctionalisationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FunctionalisationDialog(QWidget *parent = nullptr);
    ~FunctionalisationDialog();

    void setFunctionalities(std::vector<int> funcs);

    std::vector<int> getFunctionalities();

    QString presetName = "None";


private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void valueChanged(int);

private:
    Ui::FunctionalisationDialog *ui;

    // stores spinboxes
    std::vector<QSpinBox*> spinBoxes;

    void loadPresets();
};

#endif // FUNCTIONALISATIONDIALOG_H
