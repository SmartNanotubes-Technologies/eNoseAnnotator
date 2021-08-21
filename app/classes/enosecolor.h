#ifndef SENSORCOLOR_H
#define SENSORCOLOR_H

#include <vector>

#include <QtCore>
#include <QColor>

#include "functionalisation.h"
#include "mvector.h"
#include "annotation.h"

/*!
 * \brief The ENoseColor class implements a singleton based on https://stackoverflow.com/a/1008289
 */
class ENoseColor
{
public:
    //
    // singleton defenitions:
    //
    static ENoseColor& instance()
    {
        static ENoseColor instance; // Guaranteed to be destroyed.
                                    // Instantiated on first use.
        return instance;
    }

    ENoseColor(ENoseColor const&) = delete;
    void operator=(ENoseColor const&) = delete;

    //
    //  functionality definitions
    //
    void setFunctionalisation(const Functionalisation &value);
    void setSensorFailures(const std::vector<bool> &value);

    QColor getSensorColor(int i);
    QColor getFuncColor(int func);
    QColor getClassColor(int i, int n);
    QColor getIColor(int i, int n);

    Functionalisation functionalisation;
    std::vector<bool> sensorFailures;

private:
    ENoseColor() :
        functionalisation ( MVector::nChannels, false ),
        sensorFailures ( MVector::nChannels, false )
    {}

    QList<QColor> smallColorList {
        QColor("#006400"),  // darkgreen
        QColor("#1e90ff"),  // dodgerblue
        QColor("#ff0000"),  // red
        QColor("#ffd700"),  // gold
        QColor("#c71585"),  // mediumvioletred
        QColor("#00ff00"),  // lime
        QColor("#0000ff"),  // blue
        QColor("#7fffd4")   // aquamarine
    };

    QList<QColor> bigColorList {
        QColor("#2f4f4f"),  // darkslategray
        QColor("#6b8e23"),  // olivedrab
        QColor("#7f0000"),  // maroon2
        QColor("#4b0082"),  // indigo
        QColor("#ff0000"),  // red
        QColor("#ffa500"),  // orange
        QColor("#c71585"),  // mediumvioletred
        QColor("#00ff00"),  // lime
        QColor("#00fa9a"),  // mediumspringgreen
        QColor("#e9967a"),  // darksalmon
        QColor("#00ffff"),  // aqua
        QColor("#0000ff"),  // blue
        QColor("#b0c4de"),  // lightsteelblue
        QColor("#ff00ff"),  // fuchsia
        QColor("#1e90ff"),  // dodgerblue
        QColor("#ffff54")   // laserlemon
    };

    QList<QColor> hugeColorList { // source: https://graphicdesign.stackexchange.com/questions/3682/where-can-i-find-a-large-palette-set-of-contrasting-colors-for-coloring-many-d ("A Colour Alphabet and the Limits of Colour Coding.")
        QColor(240,163,255),
        QColor(0,117,220),
        QColor(153,63,0),
        QColor(76,0,92),
        QColor(25,25,25),
        QColor(0,92,49),
        QColor(43,206,72),
        QColor(255,204,153),
        QColor(128,128,128),
        QColor(148,255,181),
        QColor(143,124,0),
        QColor(157,204,0),
        QColor(194,0,136),
        QColor(0,51,128),
        QColor(255,164,5),
        QColor(255,168,187),
        QColor(66,102,0),
        QColor(255,0,16),
        QColor(94,241,242),
        QColor(0,153,143),
        QColor(224,255,102),
        QColor(116,10,255),
        QColor(153,0,0),
        QColor(255,255,128),
        QColor(255,255,0),
        QColor(255,80,5)
    };

public:

};

#endif // SENSORCOLOR_H
