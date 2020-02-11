#include "functionalisationdialog.h"
#include "ui_functionalisationdialog.h"

#include "QInputDialog"
#include <QMessageBox>

FunctionalisationDialog::FunctionalisationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FunctionalisationDialog)
{
    ui->setupUi(this);

    // window title
    this->setWindowTitle("Set Functionalization");

    // setup spArray
    spArray[0] = ui->sp1;
    spArray[1] = ui->sp2;
    spArray[2] = ui->sp3;
    spArray[3] = ui->sp4;
    spArray[4] = ui->sp5;
    spArray[5] = ui->sp6;
    spArray[6] = ui->sp7;
    spArray[7] = ui->sp8;
    spArray[8] = ui->sp9;
    spArray[9] = ui->sp10;
    spArray[10] = ui->sp11;
    spArray[11] = ui->sp12;
    spArray[12] = ui->sp13;
    spArray[13] = ui->sp14;
    spArray[14] = ui->sp15;
    spArray[15] = ui->sp16;
    spArray[16] = ui->sp17;
    spArray[17] = ui->sp18;
    spArray[18] = ui->sp19;
    spArray[19] = ui->sp20;
    spArray[20] = ui->sp21;
    spArray[21] = ui->sp22;
    spArray[22] = ui->sp23;
    spArray[23] = ui->sp24;
    spArray[24] = ui->sp25;
    spArray[25] = ui->sp26;
    spArray[26] = ui->sp27;
    spArray[27] = ui->sp28;
    spArray[28] = ui->sp29;
    spArray[29] = ui->sp30;
    spArray[30] = ui->sp31;
    spArray[31] = ui->sp32;
    spArray[32] = ui->sp33;
    spArray[33] = ui->sp34;
    spArray[34] = ui->sp35;
    spArray[35] = ui->sp36;
    spArray[36] = ui->sp37;
    spArray[37] = ui->sp38;
    spArray[38] = ui->sp39;
    spArray[39] = ui->sp40;
    spArray[40] = ui->sp41;
    spArray[41] = ui->sp42;
    spArray[42] = ui->sp43;
    spArray[43] = ui->sp44;
    spArray[44] = ui->sp45;
    spArray[45] = ui->sp46;
    spArray[46] = ui->sp47;
    spArray[47] = ui->sp48;
    spArray[48] = ui->sp49;
    spArray[49] = ui->sp50;
    spArray[50] = ui->sp51;
    spArray[51] = ui->sp52;
    spArray[52] = ui->sp53;
    spArray[53] = ui->sp54;
    spArray[54] = ui->sp55;
    spArray[55] = ui->sp56;
    spArray[56] = ui->sp57;
    spArray[57] = ui->sp58;
    spArray[58] = ui->sp59;
    spArray[59] = ui->sp60;
    spArray[60] = ui->sp61;
    spArray[61] = ui->sp62;
    spArray[62] = ui->sp63;
    spArray[63] = ui->sp64;

    // presets
    loadPresets();
}

FunctionalisationDialog::~FunctionalisationDialog()
{
    delete ui;
}

void FunctionalisationDialog::setFunctionalities(std::array<int, 64> funcs)
{
    for (int i=0; i<64; i++)
        spArray[i]->setValue(funcs[i]);
}

std::array<int, 64> FunctionalisationDialog::getFunctionalities()
{
    std::array<int, 64> funcs;

    for (int i=0; i<64; i++)
        funcs[i] = spArray[i]->value();

    return funcs;
}

void FunctionalisationDialog::loadPresets()
{
    QDir directory("./presets");
    QStringList presets = directory.entryList(QStringList() << "*.preset",QDir::Files);

    for (QString presetFileName : presets)
    {
        auto list = presetFileName.split(".");
        list.removeLast();
        QString preset = list.join(".");

        ui->comboBox->addItem(preset);
    }
}

void FunctionalisationDialog::on_comboBox_currentTextChanged(const QString &arg1)
{
    if (arg1 == "")
        ui->pushButton->setEnabled(false);
    else
        ui->pushButton->setEnabled(true);
}

void FunctionalisationDialog::on_pushButton_clicked()
{
    QString preset = ui->comboBox->currentText();
    QString presetFileName = preset + ".preset";

    QFile file("./presets/" + presetFileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, "Unable to load preset " + presetFileName,
            file.errorString());
    } else
    {
        QTextStream in(&file);

        QString line;
        std::array<int, 64> presetArray;
        bool readOk = true;
        for (int i = 0; i<spArray.size(); i++)
        {
            // load line & convert to integer
            readOk = in.readLineInto(&line);
            if (readOk)
                presetArray[i] = line.toInt(&readOk);
            if (!readOk)
            {
                QMessageBox::information(this, "Unable to load preset " + presetFileName,
                    file.errorString());
                break;
            }
        }

        if (readOk)
        {
            for (int i = 0; i<spArray.size(); i++)
                spArray[i]->setValue(presetArray[i]);
        }
    }
}

void FunctionalisationDialog::on_pushButton_2_clicked()
{
    for (int i=0; i<spArray.size(); i++)
        spArray[i]->setValue(0);
}

void FunctionalisationDialog::on_pushButton_3_clicked()
{
    // create preset folder
    if(!QDir ("./presets").exists())
        QDir().mkdir("./presets");

    // get preset name
    QString input = QInputDialog::getText(this, "Save Preset", "Preset Name: ");

    if (input == "")
    {
        QMessageBox::critical(this, "Unable to save preset " + input, "Invalid preset name! ");
        return;
    }

    QString presetFileName = input + ".preset";

    QFile file("./presets/" + presetFileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Unable to save preset " + presetFileName,
            file.errorString());
    } else
    {
        QTextStream out(&file);

        for (int i = 0; i<spArray.size(); i++)
            out << QString::number(spArray[i]->value()) << "\n";
    }
    // update preset combo box
    ui->comboBox->addItem(input);
}
