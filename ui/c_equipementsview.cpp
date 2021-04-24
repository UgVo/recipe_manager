#include "c_equipementsview.h"
#include "ui_c_equipementsview.h"
#include "c_stepview.h"
#include "utils/c_dbmanager.h"

int c_equipementsView::numberMaxEquipement = 5;

c_equipementsView::c_equipementsView(QList<QString> _equipmentList, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_equipementsView), equipmentList(_equipmentList) {
    ui->setupUi(this);
    ui->textEdit->setText(equipmentList.join(", "));
    QSet<QString> equipmentSet = c_dbManager::getEquipments();
    allEquipementsList = QList<QString>(equipmentSet.begin(),equipmentSet.end());

    for (int i = 0; i < equipmentList.size(); ++i) {
        allEquipementsList.removeOne(equipmentList[i]);
        buttonList.append(new QPushButton(equipmentList[i]));
        static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->insertWidget(i,buttonList.last());
        QFontMetrics metrics =  QFontMetrics(buttonList.last()->font());
        buttonList.last()->setFixedWidth(metrics.horizontalAdvance(equipmentList[i]) + 10);
        QObject::connect(buttonList.last(),&QPushButton::clicked,this,&c_equipementsView::removeEquipment);
    }
    switchMode(recipe::modes::resume,false);
    ui->newEquipment->installEventFilter(this);

    model = new QStringListModel(allEquipementsList);

    QCompleter* completer = new QCompleter(model);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->newEquipment->setCompleter(completer);

    QObject::connect(completer, QOverload<const QString &>::of(&QCompleter::activated),[=](const QString &text){
        addEquipment(text);
        QTimer::singleShot(0, [=] () {ui->newEquipment->clear();});}
    );

    write = false;
}

c_equipementsView::~c_equipementsView() {
    delete ui;
}

QList<QPropertyAnimation *> c_equipementsView::switchMode(int targetMode, bool animated, int time) {
    QList<QPropertyAnimation *> res;
    switch (targetMode) {
        case recipe::modes::display:
        case recipe::modes::resume: {
            if (animated) {
                res.push_back(recipe::targetSizeAnimation(this,getSize(targetMode),time));
            } else {
                this->setFixedSize(getSize(targetMode));
            }

            QFontMetrics metrics =  QFontMetrics(ui->textEdit->document()->firstBlock().charFormat().font());
            ui->textEdit->setFixedHeight(metrics.boundingRect(ui->textEdit->rect(),Qt::TextWordWrap,equipmentList.join(",")).height());

            ui->widget->setStyleSheet("QWidget#widget {"
                                      " border : 1px solid white;"
                                      " border-radius : 2px;"
                                      "}");
            ui->textEdit->setStyleSheet("QTextEdit {"
                                        "  border : 1px solid white;"
                                        "  background: transparent;"
                                        "}");
            ui->widgetEdit->hide();
        }
        break;
        case recipe::modes::edition: {
            if (animated) {
                res.push_back(recipe::targetSizeAnimation(this,getSize(targetMode),time));
            } else {
                this->setFixedSize(getSize(targetMode));
            }
            ui->textEdit->hide();
            ui->widgetEdit->show();
            ui->widgetEdit->setStyleSheet("QWidget#widgetEdit {"
                                        "  border : 1px solid black;"
                                        "  background-color: white;"
                                        "}");
            ui->newEquipment->setStyleSheet("QLineEdit {"
                                       "  border : 1px solid white;"
                                       "  background: transparent;"
                                       "}");
        }
        break;
        default:
            break;
    }
    return res;
}

QSize c_equipementsView::getSize(int mode) {
    QSize res;
    switch (mode) {
        case recipe::modes::display:
        case recipe::modes::resume: {
            QFontMetrics metrics =  QFontMetrics(ui->textEdit->document()->firstBlock().charFormat().font());
            int width = static_cast<c_stepView*>(parent())->width() - static_cast<c_stepView*>(parent())->getLimit() - c_stepView::borderSize - c_stepView::interImageSpace;
            res.setWidth(width);
            int top,bottom;
            ui->widget->layout()->getContentsMargins(nullptr,&top,nullptr,&bottom);
            res.setHeight(metrics.boundingRect(ui->textEdit->rect(),Qt::TextWordWrap,equipmentList.join(",")).height() + ui->label->height() + ui->widget->layout()->spacing() + top + bottom);
        }
        break;
        case recipe::modes::edition: {
            res.setWidth(static_cast<c_stepView*>(parent())->width() - 2*c_stepView::borderSize);
            int top,bottom;
            ui->widget->layout()->getContentsMargins(nullptr,&top,nullptr,&bottom);
            res.setHeight(ui->widgetEdit->height() + top + bottom + ui->label->height() + ui->widget->layout()->spacing());
        }
        break;
    default:
        break;
    }
    return res;
}

bool c_equipementsView::eventFilter(QObject *obj, QEvent *event) {
    if (event->type()==QEvent::KeyPress) {
           QKeyEvent* key = static_cast<QKeyEvent*>(event);
       if ( (key->key()==Qt::Key_Enter) || (key->key()==Qt::Key_Return) ) {
            addEquipment(ui->newEquipment->text());
       } else {
           return QObject::eventFilter(obj, event);
       }
       return true;
   }
    return QObject::eventFilter(obj, event);
}

void c_equipementsView::addEquipment(QString newEquipment) {
    if (!newEquipment.isEmpty() && !equipmentList.contains(newEquipment) && !equipmentList.contains(recipe::toCapitalised(newEquipment))) {
        equipmentList.push_back(recipe::toCapitalised(newEquipment));
        buttonList.append(new QPushButton(equipmentList.last()));
        static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->insertWidget(buttonList.size()-1,buttonList.last());
        QFontMetrics metrics =  QFontMetrics(buttonList.last()->font());
        buttonList.last()->setFixedWidth(metrics.horizontalAdvance(ui->newEquipment->text()) + 10);
        QObject::connect(buttonList.last(),&QPushButton::clicked,this,&c_equipementsView::removeEquipment);

        allEquipementsList.removeOne(newEquipment);
        model->setStringList(allEquipementsList);

        if (equipmentList.size() >= numberMaxEquipement) {
            ui->newEquipment->setDisabled(true);
            ui->widgetEdit->setStyleSheet("QWidget#widgetEdit {"
                                        "  border : 1px solid black;"
                                        "  background-color: rgb(240,240,240);"
                                        "}");
        } else {
            ui->newEquipment->setDisabled(false);
            ui->widgetEdit->setStyleSheet("QWidget#widgetEdit {"
                                        "  border : 1px solid black;"
                                        "  background-color: white;"
                                        "}");
        }
    }
    ui->newEquipment->clear();
}

void c_equipementsView::removeEquipment() {
    QPushButton* sender = static_cast<QPushButton *>(QObject::sender());
    buttonList.removeOne(sender);
    sender->hide();
    equipmentList.removeOne(sender->text());
    allEquipementsList.push_back(sender->text());
    model->setStringList(allEquipementsList);

    sender->deleteLater();

    if (equipmentList.size() < numberMaxEquipement) {
        ui->newEquipment->setDisabled(false);
        ui->widgetEdit->setStyleSheet("QWidget#widgetEdit {"
                                    "  border : 1px solid black;"
                                    "  background-color: white;"
                                    "}");
    }
}
