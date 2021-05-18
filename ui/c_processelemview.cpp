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

    QFontMetrics metrics(ui->processLabel->font());

    if (process != nullptr) {
        ui->duration->setValue(process->getDuration());
        ui->temperature->setValue(process->getTemperature());
        ui->processType->setCurrentText(process->getType());
        ui->processLabel->setText(formatProcessText());
        ui->processLabel->setFixedWidth(metrics.horizontalAdvance(process->getType())+2);
    } else {
        ui->processType->setCurrentText("");
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
        case modes::minimal:
            ui->duration->setReadOnly(true);
            ui->temperature->setReadOnly(true);
            ui->processLabel->setText(formatProcessText());
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
            ui->processLabel->setText(formatProcessText());
            return QSize( ui->processLabel->width()
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
        if (ui->processType->currentText().isEmpty()) {
            emit removeProcess(process);
            process = nullptr;
            ui->duration->setValue(0);
            ui->temperature->setValue(0);
            ui->processLabel->setText("");
        } else {
            process->setType(ui->processType->currentText());
            process->setDuration(ui->duration->value());
            process->setTemperature(ui->temperature->value());
        }
    } else if (!ui->processType->currentText().isEmpty()){
        process = static_cast<c_processView *>(parent())->newProcessing();
        *process = c_process(ui->processType->currentText(),ui->duration->value(),ui->temperature->value());
        ui->processLabel->setText(formatProcessText());
        ui->processLabel->setFixedWidth(getHorizontalAdvanceLabel(ui->processLabel));
    }
}

void c_processElemView::rollback() {

}

bool c_processElemView::isEmpty() const {
    return process == nullptr;
}

void c_processElemView::setProcess(c_process *value) {
    process = value;
    if (process != nullptr) {
        ui->duration->setValue(process->getDuration());
        ui->temperature->setValue(process->getTemperature());
        ui->processType->setCurrentText(process->getType());
        ui->processLabel->setText(formatProcessText());
        ui->processLabel->setFixedWidth(getHorizontalAdvanceLabel(ui->processLabel));
    } else {
        ui->processType->setCurrentText("");
        ui->temperature->setValue(0);
        ui->duration->setValue(0);
    }
    delete switchMode(mode,false);
    emit resized();
}

QString c_processElemView::formatProcessText() const {
    return QString("%1%2%3").arg(ui->processType->currentText(),
                                 ui->duration->value()!=0?QString(" %1 min").arg(ui->duration->value()):QString(""),
                                 ui->temperature->value()!=0?QString(" %1°C").arg(ui->temperature->value()):QString(""));
}

