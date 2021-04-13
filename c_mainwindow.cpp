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

    test();

    this->showMinimized();
}

c_mainWindow::~c_mainWindow()
{
    delete ui;
}

void c_mainWindow::test() {

    c_step *step = new c_step(0,"Etalez immédiatement la pâte à 3mm d'épaisseur entre 2 feuilles de papier cuisson "
                  "(sans utiliser de farine, afain de garder une pâte très friable) et réserver au réfrigirateur. "
                  "Vous pouvez utiliser deux réglettes de plexiglas pour étaler la pâte de manière uniforme. "
                  "Laisser reposer la pâte au réfrigirateur pendant 15 à 20 min avant le fonçage.",QList<QString>{":/images/step_img_1",":/images/step_img_2",":/images/step_img_3"});
    stepView = new c_stepView(step);
    stepView->show();
    stepView->setFocus();
}


