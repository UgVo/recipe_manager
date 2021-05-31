#include "c_processelemview.h"
#include "ui_c_processelemview.h"
#include "c_processview.h"

int c_processElemView::heightProcess = 25;

c_processElemView::c_processElemView(QString _processType, c_process *_process, QWidget *parent) :
    c_widget(parent),
    ui(new Ui::c_processElemView), process(_process) {
    ui->setupUi(this);

    QSet<QString> processTypes = c_dbManager::getProcessTypes();
    if (processTypes.contains(_processType)) {
        ui->processType->setText(_processType);
        processType = _processType;
    } else {
        qErrnoWarning("Wrong processType, not contained in database, potentiel corruption of the database");
        throw 200;
    }

    if (process != nullptr) {
        QFontMetrics metrics(ui->processLabel->font());
        ui->duration->setValue(process->getDuration());
        ui->temperature->setValue(process->getTemperature());
        ui->processLabel->setFixedWidth(metrics.horizontalAdvance(process->getType())+2);
    } else {
        ui->temperature->setValue(0);
        ui->duration->setValue(0);
    }

    ui->temperature->setFixedWidth(ui->temperature->width());
    ui->duration->setFixedWidth(ui->duration->width());

    ui->label->setFixedWidth(getHorizontalAdvanceLabel(ui->label));
    ui->widget->setStyleSheet("QWidget#widget {"
                              " border : 1px solid white;"
                              " border-radius : 2px;"
                              "}");
    ui->icon->setFixedSize(20,20);

    mode = modes::resume;
    delete c_processElemView::switchMode(mode,false);

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
        ui->icon->show();
        ui->icon->setPixmap(QPixmap(recipe::processToPixmapUrl[processType]).scaled(20,20,Qt::KeepAspectRatio,Qt::SmoothTransformation));
            ui->duration->setReadOnly(false);
            ui->temperature->setReadOnly(false);
            ui->processLabel->hide();
            ui->processType->setDisabled(false);
            ui->processType->show();
            ui->label->show();
            ui->label_2->show();

            ui->temperature->show();
            ui->duration->show();

            metrics = QFontMetrics(ui->label->font());

            ui->label->setFixedWidth(metrics.boundingRect(ui->label->text()).width()+2);

            this->setFixedSize(c_processElemView::getSize(target));
            this->setFixedHeight(heightProcess);

            break;
        case modes::display:
        case modes::resume:
            ui->icon->show();
            ui->icon->setPixmap(QPixmap(recipe::processToPixmapUrl[processType]).scaled(20,20,Qt::KeepAspectRatio,Qt::SmoothTransformation));
            ui->duration->setReadOnly(true);
            ui->temperature->setReadOnly(true);
            ui->processLabel->setText(formatProcessText(target));
            ui->processLabel->setFixedWidth(getHorizontalAdvanceLabel(ui->processLabel));
            ui->processLabel->show();

            ui->processType->setDisabled(true);
            ui->processType->hide();
            ui->label->hide();
            ui->label_2->hide();

            ui->temperature->hide();
            ui->duration->hide();

            this->setFixedSize(c_processElemView::getSize(target));

            break;
        case modes::minimal: {
            ui->icon->show();
            ui->icon->setPixmap(QPixmap(recipe::processToPixmapUrl[processType]).scaled(20,20,Qt::KeepAspectRatio,Qt::SmoothTransformation));
            ui->duration->setReadOnly(true);
            ui->temperature->setReadOnly(true);
            ui->processLabel->setText(formatProcessText(target));
            ui->processLabel->setFixedWidth(getHorizontalAdvanceLabel(ui->processLabel));
            ui->processLabel->show();

            ui->processType->setDisabled(true);
            ui->processType->hide();
            ui->label->hide();
            ui->label_2->hide();

            ui->temperature->hide();
            ui->duration->hide();

            this->setFixedSize(c_processElemView::getSize(target));
            break;
        }
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
            return QSize( ui->processLabel->width() + insideBorder + ui->icon->width()
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
    if (process != nullptr) {
        if (ui->duration->value() == 0) {
            emit removeProcess(process);
            process = nullptr;
            ui->duration->setValue(0);
            ui->temperature->setValue(0);
            ui->processLabel->setText("");
        } else {
            process->setType(processType);
            process->setDuration(ui->duration->value());
            process->setTemperature(ui->temperature->value());
        }
    } else if (ui->duration->value() != 0){
        process = static_cast<c_processView *>(parent())->newProcessing();
        *process = c_process(processType,ui->duration->value(),ui->temperature->value());
        ui->processLabel->setText(formatProcessText(mode));
        ui->processLabel->setFixedWidth(getHorizontalAdvanceLabel(ui->processLabel));
    }
}

void c_processElemView::rollback() {

}

bool c_processElemView::isEmpty() const {
    return (process == nullptr || (process->getDuration() == 0 && process->getTemperature() == 0));
}

void c_processElemView::setProcess(c_process *value) {
    process = value;
    if (process != nullptr) {
        ui->duration->setValue(process->getDuration());
        ui->temperature->setValue(process->getTemperature());
        ui->processLabel->setText(formatProcessText(mode));
        ui->processLabel->setFixedWidth(getHorizontalAdvanceLabel(ui->processLabel));
    } else {
        ui->temperature->setValue(0);
        ui->duration->setValue(0);
    }
    switchMode(mode,false);
    emit resized();
}

void c_processElemView::updateView() {
    ui->duration->setValue(process->getDuration());
    ui->temperature->setValue(process->getTemperature());
    ui->processLabel->setText(formatProcessText(mode));
    ui->processLabel->setFixedWidth(getHorizontalAdvanceLabel(ui->processLabel));
    switchMode(mode,false);
    emit resized();
}

QString c_processElemView::formatProcessText(modes target) const {
    switch (target) {
    case edition:
    case resume:
    case display:
        return QString("%1%2%3").arg(processType,
                                     ui->duration->value()!=0?QString(" %1 min").arg(ui->duration->value()):QString(""),
                                     ui->temperature->value()!=0?QString(" %1°C").arg(ui->temperature->value()):QString(""));
    case minimal:
        return QString("%1").arg(ui->duration->value()!=0?QString("%1 min").arg(ui->duration->value()):QString(""));
    default:
        break;
    }
    return QString();
}

