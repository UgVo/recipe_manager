#include "c_mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    c_mainWindow w;
    w.show();
    return a.exec();
}
