#include "c_processelemview.h"
#include "ui_c_processelemview.h"
#include "c_processview.h"

int c_processElemView::heightProcess = 25;

c_processElemView::c_processElemView(c_process *_process, QWidget *parent) :
    c_widget(parent),
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

    mode = modes::resume;
    switchMode(mode);

}

c_processElemView::~c_processElemView()
{
    delete ui;
}

QAbstractAnimation *c_processElemView::switchMode(modes target, bool, int) {
    QParallelAnimationGroup *res = nullptr;
    QFontMetrics metrics(ui->label->font());
    switch (target) {
        case modes::edition:
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

            this->setFixedSize(getSize(target));
            this->setFixedHeight(heightProcess);

            break;
        case modes::display:
        case modes::resume:
        case modes::minimal:
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

            this->setFixedSize(getSize(target));

            break;
    default:
        break;
    }
    mode = target;
    return res;
}

QSize c_processElemView::getSize(modes target) const {
    int marginLeft,marginRight;
    ui->widget->layout()->getContentsMargins(&marginLeft,nullptr,&marginRight,nullptr);
    switch (target) {
        case modes::resume:
        case modes::display:
        case modes::minimal:
            if (isEmpty())
                return QSize(0,0);
            return QSize(ui->durationLabel->width() + (ui->temperature->value()?ui->temperatureLabel->width():0)
                         + ui->processTypeLabel->width()
                         + marginLeft + marginRight
                         + ui->widget->layout()->spacing()*2,heightProcess);
        case modes::edition:
            return QSize(static_cast<c_processView *>(parent())->getWidth(target),heightProcess);
        default:
            break;
    }
    return QSize();
}

void c_processElemView::save() {
    QFontMetrics metrics(ui->processTypeLabel->font());
    if (ui->processType->currentText() != "") {
        process = static_cast<c_processView *>(parent())->newProcessing();
        *process = c_process(ui->processType->currentText(),ui->duration->value(),ui->duration->value());
        ui->durationLabel->setText(QString("%1 min").arg(process->getDuration()));
        ui->temperatureLabel->setText(QString("%1°C").arg(process->getTemperature()));
        ui->processTypeLabel->setText(process->getType());
        ui->processTypeLabel->setFixedWidth(metrics.horizontalAdvance(process->getType())+2);
    } else {
        if (ui->processType->currentText() == "") {
            emit removeProcess(process);
            process = nullptr;
            ui->duration->setValue(0);
            ui->temperature->setValue(0);
            ui->processTypeLabel->setText("");
        } else {
            if (ui->duration->value() != process->getDuration()) {
                process->setDuration(ui->duration->value());
                ui->durationLabel->setText(QString("%1 min").arg(process->getDuration()));
            }
            if (ui->temperature->value() != process->getTemperature()) {
                process->setTemperature(ui->temperature->value());
                ui->temperatureLabel->setText(QString("%1°C").arg(process->getTemperature()));
            }
            if (!ui->processType->currentText().compare(process->getType())) {
                process->setType(ui->processType->currentText());
                ui->processTypeLabel->setText(process->getType());
                ui->processTypeLabel->setFixedWidth(metrics.horizontalAdvance(process->getType())+2);
            }
        }
    }
}

void c_processElemView::rollback() {

}

bool c_processElemView::isEmpty() const {
    return process == nullptr;
}

void c_processElemView::setProcess(c_process *value) {
    process = value;
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
}

