#include "widgets/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("eNoseAnnotator");
    MainWindow w;
    w.show();
    return a.exec();
}
