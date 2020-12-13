#include "c_mainwindow.h"
#include "ui_c_mainwindow.h"

c_mainWindow::c_mainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::c_mainWindow)
{
    ui->setupUi(this);
}

c_mainWindow::~c_mainWindow()
{
    delete ui;
}

