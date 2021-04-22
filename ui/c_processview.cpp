#include "c_processview.h"
#include "ui_c_processview.h"

int c_processView::heightProcess = 20;

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

    QFontMetrics metrics(ui->processTypeLabel->font());

    ui->temperature->setFixedWidth(ui->temperature->width());
    ui->duration->setFixedWidth(ui->duration->width());
    ui->durationLabel->setText(QString("%1 min").arg(process->getDuration()));
    ui->temperatureLabel->setText(QString("%1°C").arg(process->getTemperature()));
    ui->durationLabel->setFixedWidth(metrics.horizontalAdvance(ui->durationLabel->text()));
    ui->temperatureLabel->setFixedWidth(metrics.horizontalAdvance(ui->temperatureLabel->text()));

    ui->processTypeLabel->setText(process->getType());
    ui->processTypeLabel->setFixedWidth(metrics.horizontalAdvance(process->getType())+2);
    ui->label->setFixedWidth(metrics.horizontalAdvance(ui->label->text())+2);
    ui->label_2->setFixedWidth(metrics.horizontalAdvance(ui->label_2->text())+2);

    int marginLeft,marginRight;
    ui->widget->layout()->getContentsMargins(&marginLeft,nullptr,&marginRight,nullptr);

    sizes[recipe::modes::display] = QSize(ui->durationLabel->width() + (ui->temperature->value()?ui->temperatureLabel->width():0)
                                          + ui->processTypeLabel->width()
                                          + marginLeft + marginRight
                                          + ui->widget->layout()->spacing()*2,heightProcess);
    sizes[recipe::modes::resume] = QSize(ui->durationLabel->width() + (ui->temperature->value()?ui->temperatureLabel->width():0)
                                          + ui->processTypeLabel->width()
                                          + marginLeft + marginRight
                                          + ui->widget->layout()->spacing()*2,heightProcess);
    sizes[recipe::modes::edition] = QSize(ui->duration->width() + ui->temperature->width()
                                          + ui->label->width() + ui->label_2->width()
                                          + ui->processType->width()
                                          + marginLeft + marginRight
                                          + ui->widget->layout()->spacing()*4,heightProcess);
    ui->widget->setStyleSheet("QWidget#widget {"
                              " border : 1px solid white;"
                              " border-radius : 2px;"
                              "}");

    switchMode(recipe::modes::display);
}

c_processView::~c_processView()
{
    delete ui;
}

QList<QPropertyAnimation *> c_processView::switchMode(int _mode) {
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
            ui->label_2->setFixedWidth(metrics.boundingRect(ui->label_2->text()).width()+2);

            this->setFixedSize(sizes[recipe::modes::edition]);
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

            this->setFixedSize(sizes[recipe::modes::display]);

            break;
    default:
        break;
    }
    return res;
}

QSize c_processView::getSize(int mode) {
    return sizes[mode];
}

