#include "c_processelemview.h"
#include "ui_c_processelemview.h"
#include "c_processview.h"

int c_processElemView::heightProcess = 25;

c_processElemView::c_processElemView(c_process *_process, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_processElemView), process(_process) {
    ui->setupUi(this);

    QSet<QString> processTypes = c_dbManager::getProcessTypes();
    QStringListModel *model = new QStringListModel(QList<QString>(processTypes.begin(),processTypes.end()));
    ui->processType->setModel(model);
    ui->processType->insertItem(0,"");

    QFontMetrics metrics(ui->processTypeLabel->font());

    if (process != nullptr) {
        ui->duration->setValue(process->getDuration());
        ui->temperature->setValue(process->getTemperature());
        ui->processType->setCurrentText(process->getType());
        ui->durationLabel->setText(QString("%1 min").arg(process->getDuration()));
        ui->temperatureLabel->setText(QString("%1°C").arg(process->getTemperature()));
        ui->processTypeLabel->setText(process->getType());
        ui->processTypeLabel->setFixedWidth(metrics.horizontalAdvance(process->getType())+2);
    } else {
        ui->processType->setCurrentText("");
        ui->temperature->setValue(0);
        ui->duration->setValue(0);
    }

    ui->temperature->setFixedWidth(ui->temperature->width());
    ui->duration->setFixedWidth(ui->duration->width());
    ui->durationLabel->setFixedWidth(metrics.horizontalAdvance(ui->durationLabel->text()));
    ui->temperatureLabel->setFixedWidth(metrics.horizontalAdvance(ui->temperatureLabel->text()));

    ui->label->setFixedWidth(metrics.horizontalAdvance(ui->label->text())+2);
    ui->widget->setStyleSheet("QWidget#widget {"
                              " border : 1px solid white;"
                              " border-radius : 2px;"
                              "}");

    switchMode(recipe::modes::display);
}

c_processElemView::~c_processElemView()
{
    delete ui;
}

QList<QPropertyAnimation *> c_processElemView::switchMode(int _mode) {
    QList<QPropertyAnimation*> res;
    QFontMetrics metrics(ui->label->font());
    switch (_mode) {
        case recipe::modes::edition:
            ui->duration->setReadOnly(false);
            ui->temperature->setReadOnly(false);
            ui->processTypeLabel->hide();
            ui->processType->setDisabled(false);
            ui->processType->show();
            ui->label->show();
            ui->label_2->show();

            ui->temperature->show();
            ui->temperatureLabel->hide();
            ui->duration->show();
            ui->durationLabel->hide();

            metrics = QFontMetrics(ui->label->font());

            ui->label->setFixedWidth(metrics.boundingRect(ui->label->text()).width()+2);

            this->setFixedSize(getSize(_mode));
            this->setFixedHeight(heightProcess);

            break;
        case recipe::modes::display:
        case recipe::modes::resume:
            ui->duration->setReadOnly(true);
            ui->temperature->setReadOnly(true);
            ui->processTypeLabel->setText(ui->processType->currentText());
            ui->processTypeLabel->show();
            ui->processType->setDisabled(true);
            ui->processType->hide();
            ui->label->hide();
            ui->label_2->hide();

            ui->temperature->hide();
            if (ui->temperature->value() != 0) {
                ui->temperatureLabel->show();
            } else {
                ui->temperatureLabel->hide();
            }
            ui->duration->hide();
            ui->durationLabel->show();

            ui->durationLabel->setText(QString("%1 min").arg(ui->duration->value()));
            ui->temperatureLabel->setText(QString("%1°C").arg(ui->temperature->value()));
            ui->durationLabel->setFixedWidth(metrics.horizontalAdvance(ui->durationLabel->text()));
            ui->temperatureLabel->setFixedWidth(metrics.horizontalAdvance(ui->temperatureLabel->text()));

            this->setFixedSize(getSize(_mode));

            break;
    default:
        break;
    }
    return res;
}

QSize c_processElemView::getSize(int mode) {
    int marginLeft,marginRight;
    ui->widget->layout()->getContentsMargins(&marginLeft,nullptr,&marginRight,nullptr);
    switch (mode) {
        case recipe::modes::resume:
        case recipe::modes::display:
            if (isEmpty())
                return QSize();
            return QSize(ui->durationLabel->width() + (ui->temperature->value()?ui->temperatureLabel->width():0)
                         + ui->processTypeLabel->width()
                         + marginLeft + marginRight
                         + ui->widget->layout()->spacing()*2,heightProcess);
        case recipe::modes::edition:
            return QSize(static_cast<c_processView *>(parent())->getSize(mode).width(),heightProcess);
        default:
            break;
    }
    return QSize();
}

bool c_processElemView::isEmpty() {
    return process == nullptr;
}

