#include "c_equipementsview.h"
#include "ui_c_equipementsview.h"
#include "c_stepview.h"
#include "utils/c_dbmanager.h"

int c_equipementsView::numberMaxEquipement = 5;

c_equipementsView::c_equipementsView(QList<QString> _equipmentList, c_widget *widget, QWidget *parent) :
    c_directedWidget(parent,widget),
    ui(new Ui::c_equipementsView), equipmentList(_equipmentList) {
    ui->setupUi(this);
    ui->textEdit->setText(equipmentList.join(", "));
    ui->textEdit->setAlignment(Qt::AlignJustify);
    QSet<QString> equipmentSet = c_dbManager::getEquipments();
    equipementsListModel = QList<QString>(equipmentSet.begin(),equipmentSet.end());

    for (int i = 0; i < equipmentList.size(); ++i) {
        equipmentLabelMap[equipmentList[i]] = (new QLabel(ui->widgetEquipments));
        equipmentLabelMap[equipmentList[i]]->setText(recipe::toCapitalised(equipmentList[i]));
        static_cast<QHBoxLayout*>(ui->widgetEquipments->layout())->insertWidget(i,equipmentLabelMap[equipmentList[i]]);

        equipementsListModel.removeOne(equipmentList[i]);
        buttonList.append(new QPushButton(equipmentList[i]));
        static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->insertWidget(i,buttonList.last());
        QFontMetrics metrics =  QFontMetrics(buttonList.last()->font());
        buttonList.last()->setFixedWidth(metrics.horizontalAdvance(equipmentList[i]) + 10);
        QObject::connect(buttonList.last(),&QPushButton::clicked,this,&c_equipementsView::removeEquipment);
    }
    ui->newEquipment->installEventFilter(this);

    model = new QStringListModel(equipementsListModel);

    QCompleter* completer = new QCompleter(model);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->newEquipment->setCompleter(completer);

    QObject::connect(completer, QOverload<const QString &>::of(&QCompleter::activated),[=](const QString &text){
        addEquipment(text);
        QTimer::singleShot(0, this, [=] () {ui->newEquipment->clear();});}
    );

    ui->label->setFixedHeight(labelHeight);

    write = false;

    delete c_equipementsView::switchMode(modes::resume,false);
}

c_equipementsView::~c_equipementsView() {
    delete ui;
}

QAbstractAnimation *c_equipementsView::switchMode(modes target, bool animated, int time, QAnimationGroup *parentGroupAnimation) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QSize widgetSize = c_equipementsView::getSize(target);
    switch (target) {
        case modes::display:
        case modes::resume:
        case modes::minimal: {
            if (animated) {
                group->addAnimation(targetSizeAnimation(this,widgetSize,time));
            } else {
                this->setFixedSize(widgetSize);
            }

            if (listDirection == verticale) {
                ui->widgetEquipments->show();
                ui->textEdit->hide();
            } else {
                int left,right;
                ui->widgetEquipments->hide();
                static_cast<QVBoxLayout *>(ui->widget->layout())->getContentsMargins(&left,nullptr,&right,nullptr);
                ui->textEdit->setFixedWidth(c_equipementsView::getSize(target).width() - left - right);
                ui->textEdit->document()->setTextWidth(ui->textEdit->width());

                ui->textEdit->setFixedHeight(ui->textEdit->document()->size().toSize().height()+3);

                ui->textEdit->show();
                ui->textEdit->setReadOnly(true);
                ui->textEdit->setStyleSheet("QTextEdit {"
                                            "  border : 1px solid white;"
                                            "  background: transparent;"
                                            "}");
            }
            ui->widget->setStyleSheet("QWidget#widget {"
                                      " border : 1px solid white;"
                                      " border-radius : 2px;"
                                      "}");
            ui->widgetEdit->hide();
        }
        break;
        case modes::edition: {
            if (mode != target) {
                addedEquipment.clear();
                toDeleteEquipment.clear();
            }
            if (animated) {
                group->addAnimation(targetSizeAnimation(this,getSize(target),time));
            } else {
                this->setFixedSize(getSize(target));
            }
            ui->widgetEquipments->hide();
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
    mode = target;

    return runBehavior(animated,group,parentGroupAnimation);
}

QSize c_equipementsView::getSize(modes target) {
    QSize res;
    int width = m_parent->getEquipmentAreaWidth(target);
    int height = m_parent->getEquipmentsAreaHeight(target);
    switch (target) {
        case modes::display:
        case modes::resume:
        case modes::minimal: {
            if (isEmpty()) {
                return QSize(8,0);
            }

            res.setWidth(width);
            int top,bottom,left,right;
            ui->widget->layout()->getContentsMargins(&left,&top,&right,&bottom);
            ui->textEdit->setFixedWidth(width - left - right);
            ui->textEdit->document()->setTextWidth(ui->textEdit->width());
            int heightH = ui->textEdit->document()->size().toSize().height()+3 + ui->label->height() + ui->widget->layout()->spacing() + top + bottom;
            int topEquipment,bottomEquipment;
            ui->widgetEquipments->layout()->getContentsMargins(nullptr,&topEquipment,nullptr,&bottomEquipment);
            int heightV = topEquipment + bottomEquipment + labelHeight*int(equipmentLabelMap.count())
                    + int(equipmentLabelMap.count() - 1)*ui->widgetEquipments->layout()->spacing()
                    + ui->label->height() + ui->widget->layout()->spacing() + top + bottom;
            if (heightV > height) {
                res.setHeight(heightH);
                listDirection = horizontale;
            } else {
                res.setHeight(heightV);
                listDirection = verticale;
            }
        }
        break;
        case modes::edition: {
            res.setWidth(width);
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

void c_equipementsView::save() {
    for (int i = 0; i < equipmentList.size(); ++i) {
        if (!equipmentLabelMap.contains(equipmentList[i])) {
            equipmentLabelMap[equipmentList[i]] = (new QLabel(ui->widgetEquipments));
            equipmentLabelMap[equipmentList[i]]->setText(recipe::toCapitalised(equipmentList[i]));
            static_cast<QHBoxLayout*>(ui->widgetEquipments->layout())->insertWidget(int(equipmentLabelMap.count()-1),equipmentLabelMap[equipmentList[i]]);
        }
    }
    QList<QString> wantedList = equipmentLabelMap.keys();
    QList<QString> removeList = (QSet<QString>(wantedList.begin(),wantedList.end()) -  QSet<QString>(equipmentList.begin(),equipmentList.end())).values();
    for (int i = 0; i < removeList.size(); ++i) {
        static_cast<QHBoxLayout*>(ui->widgetEquipments->layout())->removeWidget(equipmentLabelMap[removeList[i]]);
        equipmentLabelMap[removeList[i]]->hide();
        equipmentLabelMap[removeList[i]]->deleteLater();
        equipmentLabelMap.remove(removeList[i]);
    }
    ui->textEdit->setText(equipmentList.join(", "));
    ui->newEquipment->clear();
}

void c_equipementsView::rollback() {
    QList<QString> addedEquipmentsCopy = addedEquipment;
    for (int i = 0; i < addedEquipmentsCopy.size(); ++i) {
        if (toDeleteEquipment.contains(addedEquipmentsCopy[i])) {
            toDeleteEquipment.removeOne(addedEquipmentsCopy[i]);
        }
        addedEquipment.removeOne(addedEquipmentsCopy[i]);
        equipmentList.removeOne(addedEquipmentsCopy[i]);
    }
    for (int i = 0; i < toDeleteEquipment.size(); ++i) {
        equipmentList.push_back(toDeleteEquipment[i]);
    }
    for (int i = 0; i < buttonList.size(); ++i) {
        buttonList[i]->hide();
        buttonList[i]->deleteLater();
    }
    buttonList.clear();

    for (int i = 0; i < equipmentList.size(); ++i) {
        equipementsListModel.removeOne(equipmentList[i]);
        buttonList.append(new QPushButton(equipmentList[i]));
        static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->insertWidget(i,buttonList.last());
        QFontMetrics metrics =  QFontMetrics(buttonList.last()->font());
        buttonList.last()->setFixedWidth(metrics.horizontalAdvance(equipmentList[i]) + 10);
        QObject::connect(buttonList.last(),&QPushButton::clicked,this,&c_equipementsView::removeEquipment);
    }
}

bool c_equipementsView::isEmpty() const {
    return equipmentList.isEmpty();
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
        addedEquipment.push_back(equipmentList.last());
        buttonList.append(new QPushButton(equipmentList.last()));
        static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->insertWidget(int(buttonList.size())-1,buttonList.last());
        QFontMetrics metrics =  QFontMetrics(buttonList.last()->font());
        buttonList.last()->setFixedWidth(metrics.horizontalAdvance(ui->newEquipment->text()) + 10);
        QObject::connect(buttonList.last(),&QPushButton::clicked,this,&c_equipementsView::removeEquipment);

        equipementsListModel.removeOne(newEquipment);
        model->setStringList(equipementsListModel);

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
    toDeleteEquipment.push_back(sender->text());
    equipementsListModel.push_back(sender->text());
    model->setStringList(equipementsListModel);

    sender->deleteLater();

    if (equipmentList.size() < numberMaxEquipement) {
        ui->newEquipment->setDisabled(false);
        ui->widgetEdit->setStyleSheet("QWidget#widgetEdit {"
                                    "  border : 1px solid black;"
                                    "  background-color: white;"
                                    "}");
    }
}
