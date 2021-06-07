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
    ui->textEdit->setFocusPolicy(Qt::StrongFocus);
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
    if (target == modes::none) {
        target = mode;
    }
    QSize widgetSize = c_equipementsView::getSize(target);
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
    QList<QString> newList = equipmentButtonMap.keys();
    for (int i = 0; i < newList.size(); ++i) {
        if (!equipmentLabelMap.contains(newList[i])) {
            equipmentLabelMap[newList[i]] = new QLabel(ui->widgetEquipments);
            equipmentLabelMap[newList[i]]->setText(" • "+recipe::toCapitalised(newList[i]));
            static_cast<QHBoxLayout*>(ui->widgetEquipments->layout())->addWidget(equipmentLabelMap[newList[i]]);
        }
    }
    for (int i = 0; i < equipmentList.size(); ++i) {
        if (!newList.contains(equipmentList[i])) {
            static_cast<QHBoxLayout*>(ui->widgetEquipments->layout())->removeWidget(equipmentLabelMap[equipmentList[i]]);
            equipmentLabelMap[equipmentList[i]]->hide();
            equipmentLabelMap[equipmentList[i]]->deleteLater();
            equipmentLabelMap.remove(equipmentList[i]);
        }
    }
    equipmentList = newList;
}

void c_equipementsView::rollback() {
    QList<QString> oldList = equipmentButtonMap.keys();
    for (int i = 0; i < equipmentList.size(); ++i) {
        if (!equipmentButtonMap.contains(equipmentList[i])) {
            QString key = equipmentList[i].toLower();
            equipmentButtonMap[key] = new QPushButton(recipe::toCapitalised(key));
            equipmentButtonMap[key]->setFixedWidth(QFontMetrics(equipmentButtonMap[key]->font()).horizontalAdvance(key) + 10);
            static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->insertWidget(int(equipmentButtonMap.count()-1),equipmentButtonMap[key]);
            QObject::connect(equipmentButtonMap[key],&QPushButton::clicked,this,&c_equipementsView::removeEquipment);

            equipementsListModel.removeOne(recipe::toCapitalised(key));
        }
    }

    for (int i = 0; i < oldList.size(); ++i) {
        if (!equipmentList.contains(oldList[i])) {
            QString key = oldList[i].toLower();
            static_cast<QHBoxLayout*>(ui->widgetEdit->layout())->removeWidget(equipmentButtonMap[key]);
            equipmentButtonMap[key]->hide();
            equipmentButtonMap[key]->deleteLater();
            equipmentButtonMap.remove(key);
            equipementsListModel.push_back(recipe::toCapitalised(key));
        }
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

void c_equipementsView::setEquipmentList(const QSet<QString> _equipments) {
    QSet<QString> equipments;
    foreach (QString elem, _equipments) {
        equipments.insert(elem.toLower());
    }
    QHBoxLayout *layoutLabel = static_cast<QHBoxLayout*>(ui->widgetEquipments->layout());
    QHBoxLayout *layoutButton = static_cast<QHBoxLayout*>(ui->widgetEdit->layout());
    QSet<QString> addList = equipments - QSet<QString>(equipmentList.begin(),equipmentList.end());
    QSet<QString> removeList = QSet<QString>(equipmentList.begin(),equipmentList.end()) - equipments;

    for (const QString &elem : removeList) {
        if (equipmentButtonMap.contains(elem)) {
            layoutButton->removeWidget(equipmentButtonMap[elem]);
            equipmentButtonMap[elem]->hide();
            equipmentButtonMap[elem]->deleteLater();
            equipmentButtonMap.remove(elem);
        }
        layoutLabel->removeWidget(equipmentLabelMap[elem]);
        if (equipmentLabelMap[elem] != nullptr) {
            equipmentLabelMap[elem]->hide();
            equipmentLabelMap[elem]->deleteLater();
        }
        equipmentLabelMap.remove(elem);

        equipementsListModel.append(recipe::toCapitalised(elem));
    }

    model->setStringList(equipementsListModel);

    for (const QString &elem : addList) {
        QString key = elem.toLower();
        equipmentLabelMap[key] = new QLabel(ui->widgetEquipments);
        equipmentLabelMap[key]->setText(" • "+recipe::toCapitalised(key));
        layoutLabel->addWidget(equipmentLabelMap[key]);

        equipementsListModel.removeOne(recipe::toCapitalised(key));

        if (!equipmentButtonMap.contains(key)) {
            equipmentButtonMap[key] = new QPushButton(recipe::toCapitalised(key));
            QFontMetrics metrics =  QFontMetrics(equipmentButtonMap[key]->font());
            equipmentButtonMap[key]->setFixedWidth(metrics.horizontalAdvance(key) + 10);
            layoutButton->insertWidget(int(equipmentButtonMap.count()-1),equipmentButtonMap[key]);
            QObject::connect(equipmentButtonMap[key],&QPushButton::clicked,this,&c_equipementsView::removeEquipment);
        }
    }

    for (const QString &elem : addList) {
        equipmentList.push_back(elem.toLower());
    }

    ui->textEdit->setText(equipmentList.join(", "));
}

QList<QString> c_equipementsView::getEquipmentList() {
    return equipmentList;
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
    if (!equipmentButtonMap.contains(newEquipment)) {
        equipmentButtonMap[newEquipment] = new QPushButton(recipe::toCapitalised(newEquipment));
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
    QString key = sender->text().toLower();

    ui->widgetEdit->layout()->removeWidget(sender);
    equipmentButtonMap[key]->hide();
    equipmentButtonMap[key]->deleteLater();
    equipmentButtonMap.remove(key);

    equipementsListModel.push_back(recipe::toCapitalised(key));
    model->setStringList(equipementsListModel);


    if (equipmentButtonMap.count() < numberMaxEquipement) {
        ui->newEquipment->setDisabled(false);
        ui->widgetEdit->setStyleSheet("QWidget#widgetEdit {"
                                    "  border : 1px solid black;"
                                    "  background-color: white;"
                                    "}");
    }
}
