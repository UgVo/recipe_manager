#include "c_equipementsview.h"
#include "ui_c_equipementsview.h"
#include "c_stepview.h"
#include "utils/c_dbmanager.h"

int c_equipementsView::numberMaxEquipement = 5;

c_equipementsView::c_equipementsView(QList<QString> _equipmentList, c_widget *widget, QWidget *parent) :
    c_directedWidget(parent,widget),
    ui(new Ui::c_equipementsView) {
    ui->setupUi(this);
    ui->textEdit->setAlignment(Qt::AlignJustify);
    QSet<QString> equipmentSet = c_dbManager::getEquipments();
    equipementsListModel = QList<QString>(equipmentSet.begin(),equipmentSet.end());
    ui->newEquipment->installEventFilter(this);

    model = new QStringListModel(equipementsListModel);

    setEquipmentList(QSet<QString>(_equipmentList.begin(),_equipmentList.end()));

    QCompleter* completer = new QCompleter(model);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->newEquipment->setCompleter(completer);

    QObject::connect(completer, QOverload<const QString &>::of(&QCompleter::activated),[=](const QString &text){
        addEquipment(text);
        QTimer::singleShot(0, this, [=] () {ui->newEquipment->clear();});}
    );

    ui->label->setFixedHeight(labelHeight);
    _hideTitle = false;

    delete c_equipementsView::switchMode(modes::resume,false);
}

c_equipementsView::~c_equipementsView() {
    delete ui;
}

QAbstractAnimation *c_equipementsView::switchMode(modes target, bool animated, int time, QAnimationGroup *parentGroupAnimation) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QSize widgetSize = c_equipementsView::getSize(target);
    if (target == modes::none) {
        target = mode;
    }
    ui->labelWidget->setHidden(_hideTitle);
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
    int width = m_parent->getEquipmentsAreaWidth(target);
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
                    + (!_hideTitle?ui->label->height() + ui->widget->layout()->spacing():0) + top + bottom;
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

    addedEquipment.clear();
    toDeleteEquipment.clear();
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
    foreach (QPushButton *elem, equipmentButtonMap) {
        static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->removeWidget(elem);
        equipementsListModel.push_back(recipe::toCapitalised(elem->text()));
        elem->hide();
        elem->deleteLater();
    }
    model->setStringList(equipementsListModel);
    equipmentButtonMap.clear();

    for (int i = 0; i < equipmentList.size(); ++i) {
        equipementsListModel.removeOne(equipmentList[i]);
        equipmentButtonMap[equipmentList[i]] = new QPushButton(recipe::toCapitalised(equipmentList[i]));
        static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->insertWidget(i,equipmentButtonMap[equipmentList[i]]);
        QFontMetrics metrics =  QFontMetrics(equipmentButtonMap[equipmentList[i]]->font());
        equipmentButtonMap[equipmentList[i]]->setFixedWidth(metrics.horizontalAdvance(equipmentList[i]) + 10);
        QObject::connect(equipmentButtonMap[equipmentList[i]],&QPushButton::clicked,this,&c_equipementsView::removeEquipment);
    }

    addedEquipment.clear();
    toDeleteEquipment.clear();
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

void c_equipementsView::setEquipmentList(const QSet<QString> equipments) {
    QHBoxLayout *layoutLabel = static_cast<QHBoxLayout*>(ui->widgetEquipments->layout());
    QHBoxLayout *layoutButton = static_cast<QHBoxLayout*>(ui->widgetEdit->layout());
    QSet<QString> addList = equipments - QSet<QString>(equipmentList.begin(),equipmentList.end());
    QSet<QString> removeList = QSet<QString>(equipmentList.begin(),equipmentList.end()) - equipments;

    for (const QString &elem : removeList) {
        if (equipmentButtonMap.contains(elem)) {
            layoutButton->removeWidget(equipmentButtonMap[elem]);
            equipmentButtonMap[elem]->hide();
            equipmentButtonMap[elem]->deleteLater();
        }
        layoutLabel->removeWidget(equipmentLabelMap[elem]);
        equipmentLabelMap[elem]->hide();
        equipmentLabelMap[elem]->deleteLater();

        equipementsListModel.append(recipe::toCapitalised(elem));
    }

    model->setStringList(equipementsListModel);

    for (const QString &elem : addList) {
        equipmentLabelMap[elem] = new QLabel(ui->widgetEquipments);
        equipmentLabelMap[elem]->setText(recipe::toCapitalised(elem));
        layoutLabel->insertWidget(int(equipmentLabelMap.count()-1),equipmentLabelMap[elem]);

        equipementsListModel.removeOne(recipe::toCapitalised(elem));

        if (!equipmentButtonMap.contains(elem)) {
            equipmentButtonMap[elem] = new QPushButton(recipe::toCapitalised(elem));
            QFontMetrics metrics =  QFontMetrics(equipmentButtonMap[elem]->font());
            equipmentButtonMap[elem]->setFixedWidth(metrics.horizontalAdvance(elem) + 10);
            layoutButton->insertWidget(int(equipmentButtonMap.count()-1),equipmentButtonMap[elem]);
            QObject::connect(equipmentButtonMap[elem],&QPushButton::clicked,this,&c_equipementsView::removeEquipment);
        }
    }

    equipmentList = equipments.values();

    ui->textEdit->setText(equipmentList.join(", "));
}

void c_equipementsView::hideTitle(bool hide) {
    int top , rigth, bottom;
    _hideTitle = hide;
    ui->widget->layout()->getContentsMargins(nullptr,&top,&rigth,&bottom);
    if (hide) {
        ui->widgetEquipments->layout()->setContentsMargins(0,top,rigth,bottom);
    } else {
        ui->widgetEquipments->layout()->setContentsMargins(rigth,top,rigth,bottom);
    }
    update();
    switchMode(mode,false);
}

void c_equipementsView::addEquipment(QString _newEquipment) {
    QString newEquipment = _newEquipment.toLower();
    if (!newEquipment.isEmpty() && !equipmentList.contains(newEquipment) && !equipmentList.contains(recipe::toCapitalised(newEquipment))) {
        equipmentList.push_back(recipe::toCapitalised(newEquipment));
        addedEquipment.push_back(equipmentList.last());
        equipmentButtonMap[newEquipment] = new QPushButton(equipmentList.last());
        static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->insertWidget(int(equipmentButtonMap.count())-1,equipmentButtonMap[newEquipment]);
        QFontMetrics metrics =  QFontMetrics(equipmentButtonMap[newEquipment]->font());
        equipmentButtonMap[newEquipment]->setFixedWidth(metrics.horizontalAdvance(ui->newEquipment->text()) + 10);
        QObject::connect(equipmentButtonMap[newEquipment],&QPushButton::clicked,this,&c_equipementsView::removeEquipment);

        equipementsListModel.removeOne(recipe::toCapitalised(newEquipment));
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
    equipmentButtonMap.remove(sender->text().toLower());
    equipmentLabelMap.remove(sender->text().toLower());
    sender->hide();
    equipmentList.removeOne(sender->text());
    toDeleteEquipment.push_back(sender->text().toLower());
    equipementsListModel.push_back(sender->text());
    model->setStringList(equipementsListModel);

    sender->deleteLater();

    if (equipmentList.size() + addedEquipment.size() - toDeleteEquipment.size() < numberMaxEquipement) {
        ui->newEquipment->setDisabled(false);
        ui->widgetEdit->setStyleSheet("QWidget#widgetEdit {"
                                    "  border : 1px solid black;"
                                    "  background-color: white;"
                                    "}");
    }
}
