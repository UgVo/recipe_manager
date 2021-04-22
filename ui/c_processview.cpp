#include "c_processview.h"
#include "ui_c_processview.h"

c_processView::c_processView(c_process *_process, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_processView), process(_process) {
    ui->setupUi(this);
    ui->duration->setValue(process->getDuration());
    ui->temperature->setValue(process->getTemperature());
    QSet<QString> processTypes = c_dbManager::getProcessTypes();
    QStringListModel *model = new QStringListModel(QList<QString>(processTypes.begin(),processTypes.end()));
    ui->processType->setModel(model);

    ui->processType->setCurrentText(process->getType());
}

c_processView::~c_processView()
{
    delete ui;
}
