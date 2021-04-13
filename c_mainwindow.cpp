#include "c_mainwindow.h"
#include "ui_c_mainwindow.h"

c_mainWindow::c_mainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::c_mainWindow)
{
    ui->setupUi(this);
    iomanager = new c_ioManager();
    dbmanager = new c_dbManager(iomanager);
    dbmanager->initDatabase();
    dbmanager->enablePragma(true);
}

c_mainWindow::~c_mainWindow()
{
    delete ui;
}


