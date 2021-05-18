#include "c_stepview.h"
#include "ui_c_stepview.h"

QSize c_stepView::maxSizeImage((stepWidth/3)*2,(stepWidth/3)*2);
int c_stepView::interImageSpace = 5;
int c_stepView::borderMenuButton = 6;
int c_stepView::maxNumberImages = 4;

c_stepView::c_stepView(c_step *_step, QWidget *parent) :
    c_widget(parent),
    ui(new Ui::c_stepView), step(_step) {
    ui->setupUi(this);

    this->setFixedWidth(stepWidth);

    mode = modes::setup;

    ui->rankButton->setFixedSize(40,40);
    QRect rect(2,2,ui->rankButton->width()-4,ui->rankButton->height()-4);
    QRegion region(rect, QRegion::Ellipse);
    ui->rankButton->setMask(region);

    ui->showButton->setFixedWidth(stepWidth - 2*borderSize);
    QObject::connect(ui->showButton,&QPushButton::released,this,&c_stepView::triggerShowButton);

    ui->label->append(step->getDescription());
    ui->label->setAlignment(Qt::AlignJustify);
    QObject::connect(ui->label,&QTextEdit::textChanged, [=] () {
        if (getHeightText(mode) != ui->label->height()) {
            if (parent == nullptr) {
                switchMode(mode,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
            } else {
                emit animationRequired(switchMode(mode,true,500));
            }
        }
    });

    ui->rankButton->setText(QString("%1").arg(step->getRank()));
    ui->rankButton->stackUnder(ui->label);
    QObject::connect(step,&c_step::rankChanged, [=] () {
        ui->rankButton->setText(QString("%1").arg(step->getRank()));
    });

    ui->saveButton->setFixedWidth((width()/2 - 2*borderSize));
    ui->saveButton->setFixedSize(ui->saveButton->size());
    QObject::connect(ui->saveButton,&QPushButton::clicked,this,&c_stepView::editSaved);
    ui->cancelButton->setFixedSize(ui->saveButton->size());
    QObject::connect(ui->cancelButton,&QPushButton::clicked,this,&c_stepView::editCanceled);

    ui->upButton->setFixedSize(ui->upButton->size());
    QObject::connect(ui->upButton,&QPushButton::clicked, [this] () {
        emit swapRank(recipe::swapAbove);
    });
    ui->downButton->setFixedSize(ui->downButton->size());
    QObject::connect(ui->downButton,&QPushButton::clicked, [this] () {
        emit swapRank(recipe::swapBelow);
    });

    ui->displayButton->hide();
    QObject::connect(ui->displayButton,&QPushButton::clicked,[=] () {
        if (parent == nullptr) {
            switchMode(modes::display,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            emit animationRequired(switchMode(modes::display,true,500));
        }
    });

    ui->resumeButton->hide();
    QObject::connect(ui->resumeButton,&QPushButton::clicked,[=] () {
        if (parent == nullptr) {
            switchMode(modes::resume,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            emit animationRequired(switchMode(modes::resume,true,500));
        }
    });

    QList<QString> imageStringList = step->getImagesUrl();
    while (imageStringList.size() > maxNumberImages) {
        imageStringList.removeLast();
    }

    countImages = int(imageStringList.size());

    components = new c_componentView(step->getComponentsPtr(),this,ui->widget);
    QObject::connect(components,&c_componentView::resized, [=] () {
        if (parent == nullptr) {
            switchMode(mode,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            emit animationRequired(switchMode(mode,true,500));
        }
    });

    updateLimit();

    for (int i = 0; i < imageStringList.size(); ++i) {
        images.push_back(new c_image(imageStringList[i],this,ui->widget));
    }
    for (int i = int(imageStringList.size()); i < maxNumberImages; ++i) {
        images.push_back(new c_image("",this,ui->widget));
    }
    for (int i = 0; i < images.size(); ++i) {
        QObject::connect(images[i],&c_image::newImage,this,&c_stepView::imageAdded);
        QObject::connect(images[i],&c_image::resized, [=] () {
            if (parent == nullptr) {
                switchMode(mode,true,500)->start(QAbstractAnimation::DeleteWhenStopped);
            } else {
                emit animationRequired(switchMode(mode,true,500));
            }
        });
    }

    QList<c_process* > processList = step->getProcessingsPtr();
    processes = new c_processView(processList,this,ui->widget);
    processes->show();

    equipments = new c_equipementsView(step->getEquipments(),this,ui->widget);

    QMenu *menu = new QMenu();
    menu->addAction("Edit", this, [=] () {
        if (parent == nullptr) {
            switchMode(modes::edition)->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            emit animationRequired(switchMode(modes::edition));
        }
    });
    menu->addAction("Delete", this, [this] () {
        emit toDelete();
    });
    menu->addAction("Add note",this,&c_stepView::slotAddNote);

    ui->menuButton->setMenu(menu);
    ui->rankButton->move(borderSize,borderSize);

    noteDialog = new c_notesDialog(step->getNotesPtr(),this);
    noteDialog->hide();
    QObject::connect(ui->noteButton,&QPushButton::released,this,&c_stepView::slotShowNotes);

    defaultMode = (components->isEmpty() && equipments->isEmpty()) ? modes::display : modes::resume;
    mode = defaultMode;

    this->setStyleSheet("QWidget#stepWidget,QWidget#widget {"
                        "   background-color : white;"
                        "}"
                        "QWidget#stepWidget {"
                        "   border : 1px solid black;"
                        "}");

    delete c_stepView::switchMode(modes::minimal,false);
}

c_stepView::~c_stepView() {
    delete ui;
    delete noteDialog;
}

void c_stepView::triggerShowButton() {
    ui->showButton->raise();
    if (mode == modes::minimal) {
        if (parent() == nullptr)
            switchMode(defaultMode)->start(QAbstractAnimation::DeleteWhenStopped);
        else
            emit animationRequired(switchMode(defaultMode));
    } else {
        if (parent() == nullptr)
            switchMode(modes::minimal)->start(QAbstractAnimation::DeleteWhenStopped);
        else
            emit animationRequired(switchMode(modes::minimal));
    }
}

void c_stepView::editSaved() {
    imageList.clear();
    for (int i = 0; i < images.size(); ++i) {
        images[i]->save();
        if (!images[i]->isEmpty())
            imageList.push_back(images[i]->getPathImage());
    }

    step->setImagesUrl(imageList);

    processes->save();
    components->save();
    equipments->save();

    QString res;
    for (int i = 0; i < ui->label->document()->blockCount(); ++i) {
        res.append(ui->label->document()->findBlockByNumber(i).text());
        res.append("\n");
    }
    res.remove(QRegularExpression("\n$"));
    step->setDescription(res);

    QObject::disconnect(ui->saveButton,&QPushButton::released,this,&c_stepView::editSaved);
    QObject::disconnect(ui->cancelButton,&QPushButton::released,this,&c_stepView::editCanceled);

    updateLimit();

    defaultMode = (components->isEmpty() && equipments->isEmpty()) ? modes::display : modes::resume;

    if (parent() == nullptr)
        switchMode(defaultMode)->start(QAbstractAnimation::DeleteWhenStopped);
    else
        emit animationRequired(switchMode(defaultMode));

    emit saved(step);
}

void c_stepView::editCanceled() {
    ui->label->clear();
    ui->label->append(step->getDescription());
    ui->label->setAlignment(Qt::AlignJustify);

    for (int i = 0; i < images.size(); ++i) {
        images[i]->rollback();
    }

    processes->rollback();
    components->rollback();
    equipments->rollback();

    QObject::disconnect(ui->saveButton,&QPushButton::released,this,&c_stepView::editSaved);
    QObject::disconnect(ui->cancelButton,&QPushButton::released,this,&c_stepView::editCanceled);

    updateLimit();

    defaultMode = (components->isEmpty() && equipments->isEmpty()) ? modes::display : modes::resume;

    if (parent() == nullptr)
        switchMode(defaultMode)->start(QAbstractAnimation::DeleteWhenStopped);
    else
        emit animationRequired(switchMode(defaultMode));
}

void c_stepView::slotAddNote() {
    if (step->getNotes().isEmpty()) {
        ui->noteButton->hide();
    } else {
        ui->noteButton->show();
    }
    noteDialog->slotNewNote();
    noteDialog->exec();
    switchMode(mode,false);
}

void c_stepView::slotShowNotes() {
    noteDialog->exec();
}

int c_stepView::getImageCount(){
    return countImages;
}

void c_stepView::imageAdded(QAbstractAnimation * animations) {
    if (parent() == nullptr)
        switchMode(mode,true,500,animations);
    else
        emit animationRequired(switchMode(mode,true,500,animations));
    checkCount();
}

void c_stepView::checkCount() {
    countImages = 0;
    for (int i = 0; i < images.size(); ++i) {
        if (!images[i]->isEmpty())
            countImages++;
    }
}

int c_stepView::getHeightText(modes targetMode) const {
    QRect rect = ui->label->rect();
    int res = 0;
    for (int i = 0; i < ui->label->document()->blockCount(); ++i) {
        QFontMetrics metrics(ui->label->document()->findBlockByNumber(i).charFormat().font());
        res += metrics.boundingRect(rect,Qt::TextWordWrap,ui->label->document()->findBlockByNumber(i).text()).size().height();
    }
    // offset to compensate difference in height between text and QEditText
    res += 8;
    if (targetMode == modes::display || targetMode == modes::resume || targetMode == modes::minimal) {
        return res + 8;
    } else if (targetMode == modes::edition){
        QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
        if (metrics.height()*2+res > ui->rankButton->height()) {
            res = metrics.height()*2 + res;
        } else {
            res = ui->rankButton->height();
        }
    }
    return res;
}

QAnimationGroup *c_stepView::switchMode(modes target, bool animated, int time, QAbstractAnimation *childAnims) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    if (childAnims) {
        group->addAnimation(childAnims);
    }
    QSize targetSize;
    QPoint targetPos;
    QList<QPropertyAnimation *> anims;
    switch (target) {
        case modes::resume: {
            this->setFixedWidth(this->size().width());

            // label
            ui->label->setReadOnly(true);
            ui->label->setStyleSheet("QTextEdit {"
                                     "  border : 1px solid white;"
                                     "  background: transparent;"
                                     "}");
            ui->label->setFixedWidth(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton);
            targetSize = QSize(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton,getHeightText(modes::resume));
            targetPos = QPoint(ui->rankButton->width()+borderSize*2,borderSize);
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->label,targetSize,targetPos,time));
            } else {
                ui->label->setFixedSize(targetSize);
                ui->label->move(targetPos);
            }

            // rank button
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->rankButton,QPoint(borderSize,borderSize),time));
            } else {
                ui->rankButton->move(borderSize,borderSize);
            }

            // Central Widget
            targetPos = QPoint(1,1);
            targetSize = QSize(width()-2,getSize(target).height()-ui->showButton->height() - insideBorder);
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->widget,targetSize,targetPos,time));
            } else {
                ui->widget->move(targetPos);
                ui->widget->setFixedSize(targetSize);
            }

            // Process Views
            targetPos = QPoint((this->width() - 2*borderSize - processes->getSize(target).width())/2,borderSize + std::max(ui->rankButton->height(),getHeightText(target)));
            if (animated) {
                group->addAnimation(targetPositionAnimation(processes,targetPos,time));
            } else {
                processes->move(targetPos);
            }
            if ((mode == modes::edition) || !animated)
                group->addAnimation(processes->switchMode(target,animated,time));

            // Images
            QList<QPoint> posList = arrangeImages(target);
            for (int i = 0; i < images.size(); ++i) {
                if (animated) {
                    group->addAnimation(fadeAnimation(images[i],true,time));
                    group->addAnimation(targetPositionAnimation(images[i],posList[i],time));
                } else {
                    images[i]->move(posList[i]);
                }
                group->addAnimation(images[i]->switchMode(target,animated,time));
                images[i]->show();
                images[i]->raise();
            }

            // Display button
            checkCount();
            if (countImages>maxNumberImages/2) {
                int y = borderSize + std::max(getHeightText(target),ui->rankButton->height()) + processes->getSize(target).height() + interImageSpace
                        + getImagesMaxHeigth(target) + interImageSpace;
                ui->displayButton->move(QPoint((limit-borderSize-interImageSpace)/2+borderSize - ui->displayButton->width()/2,y));
                ui->displayButton->setText(QString("+%1").arg(countImages-maxNumberImages/2));
                if (animated) {
                    group->addAnimation(fadeAnimation(ui->displayButton,true,time));
                } else {
                    ui->displayButton->show();
                }
            } else {
                if (animated) {
                    group->addAnimation(fadeAnimation(ui->displayButton,false,time));
                } else {
                    ui->displayButton->hide();
                }
            }

            // Resume button
            targetPos = QPoint(interImageSpace,ui->rankButton->height() + interImageSpace);
            targetSize = ui->menuButton->size();
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->resumeButton,targetSize,targetPos,time));
                group->addAnimation(fadeAnimation(ui->resumeButton,false,time));
            } else {
                ui->resumeButton->move(targetPos);
                ui->resumeButton->setFixedSize(targetSize);
                ui->resumeButton->hide();
            }

            // Components
            targetPos = QPoint(limit + interImageSpace,borderSize+std::max(ui->rankButton->height(),getHeightText(target))
                               + processes->getSize(target).height() + interImageSpace);
            if (animated) {
                group->addAnimation(targetPositionAnimation(components,targetPos,time));
                group->addAnimation(fadeAnimation(components,true,time));
            } else {
                components->move(targetPos);
                components->show();
            }
            group->addAnimation(components->switchMode(target,animated,time));

            // Equipments
            targetPos = QPoint(limit + interImageSpace, borderSize+std::max(ui->rankButton->height(),getHeightText(target))
                               + processes->getSize(target).height() + interImageSpace
                               + components->getSize(target).height() + (components->isEmpty()?0:interImageSpace));
            if (animated) {
                group->addAnimation(targetPositionAnimation(equipments,targetPos,time));
                for (int i = 0; i < anims.size(); ++i) {
                    group->addAnimation(anims[i]);
                }
                group->addAnimation(fadeAnimation(equipments,true,time));
            } else {
                equipments->move(targetPos);
                equipments->show();
            }
            group->addAnimation(equipments->switchMode(target,animated,time));

            // Show button
            targetSize = QSize(this->width()-2*borderSize,buttonHeight);
            ui->showButton->setFixedSize(targetSize);
            ui->showButton->raise();

            // Menu button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->menuButton,targetPos,time));
                group->addAnimation(fadeAnimation(ui->menuButton,true,time));
            } else {
                ui->menuButton->move(targetPos);
            }

            // Note button
            targetPos = QPoint(this->width()- borderMenuButton- (ui->menuButton->width() + ui->noteButton->width())/2 , borderSize + ui->menuButton->height() + interImageSpace);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->noteButton,targetPos,time));
            } else {
                ui->noteButton->move(targetPos);
            }
            if (step->getNotes().isEmpty()) {
                ui->noteButton->hide();
            } else {
                ui->noteButton->show();
            }

            // Edition Buttons
            int posSaveCancelX = (width()/2-2*borderSize-ui->saveButton->width())/2 + borderSize;
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->saveButton,QPoint(posSaveCancelX,getSize(modes::edition).height()),time));
                group->addAnimation(targetPositionAnimation(ui->cancelButton,QPoint(width()/2 + posSaveCancelX,getSize(modes::edition).height()),time));
                group->addAnimation(targetPositionAnimation(ui->upButton,QPoint(this->width(),0),time));
                group->addAnimation(targetPositionAnimation(ui->downButton,QPoint(this->width(),0),time));
            } else  {
                ui->saveButton->move(QPoint(posSaveCancelX,getSize(modes::edition).height()));
                ui->cancelButton->move(QPoint(width()/2 + posSaveCancelX,getSize(modes::edition).height()));
                ui->upButton->move(QPoint(this->width(),0));
                ui->downButton->move(QPoint(this->width(),0));
            }

            QPoint targetPos = QPoint(borderSize, getSize(target).height()-ui->showButton->height());
            QSize targetSize = getSize(target);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->showButton,targetPos,time));
                group->addAnimation(targetSizeAnimation(this,targetSize,time));
            } else {
                ui->showButton->move(targetPos);
                this->setFixedSize(targetSize);
            }
            ui->showButton->raise();
        }
        break;
        case modes::display: {
            this->setFixedWidth(this->size().width());

            // label
            ui->label->setReadOnly(true);
            ui->label->setStyleSheet("QTextEdit {"
                                     "  border : 1px solid white;"
                                     "  background: transparent;"
                                     "}");
            ui->label->setFixedWidth(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton);
            targetSize = QSize(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton,getHeightText(modes::display));
            targetPos = QPoint(ui->rankButton->width()+borderSize*2,borderSize);
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->label,targetSize,targetPos,time));
            } else {
                ui->label->setFixedSize(targetSize);
                ui->label->move(targetPos);
            }

            // rank button
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->rankButton,QPoint(borderSize,borderSize),time));
            } else {
                ui->rankButton->move(borderSize,borderSize);
            }

            // Central Widget
            targetPos = QPoint(1,1);
            targetSize = QSize(width()-2,getSize(target).height()-ui->showButton->height() - insideBorder);
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->widget,targetSize,targetPos,time));
            } else {
                ui->widget->move(targetPos);
                ui->widget->setFixedSize(targetSize);
            }

            // Process Views
            targetPos = QPoint((this->width() - 2*borderSize - processes->getSize(target).width())/2,borderSize + std::max(ui->rankButton->height(),getHeightText(target)));
            if (animated) {
                group->addAnimation(targetPositionAnimation(processes,targetPos,time));
            } else {
                processes->move(targetPos);
            }
            if (mode != defaultMode)
                group->addAnimation(processes->switchMode(target,false,time));


            // Images
            QList<QPoint> posList = arrangeImages(target);
            for (int i = 0; i < images.size(); ++i) {
                if (animated) {
                    group->addAnimation(targetPositionAnimation(images[i],posList[i],time));
                } else {
                    images[i]->move(posList[i]);
                }
                group->addAnimation(images[i]->switchMode(target,animated,time));
                images[i]->show();
                images[i]->raise();
            }

            // Display button
            checkCount();
            if (animated) {
                group->addAnimation(fadeAnimation(ui->displayButton,false,time));
            } else {
                ui->displayButton->hide();
            }

            // Resume button
            if (defaultMode != display) {
                targetPos = QPoint(interImageSpace,ui->rankButton->height() + interImageSpace + borderSize);
                targetSize = ui->menuButton->size();
                if (animated) {
                    group->addAnimation(targetGeometryAnimation(ui->resumeButton,targetSize,targetPos,time));
                    group->addAnimation(fadeAnimation(ui->resumeButton,true,time));
                } else {
                    ui->resumeButton->move(targetPos);
                    ui->resumeButton->setFixedSize(targetSize);
                    ui->resumeButton->show();
                }
                ui->resumeButton->raise();
            } else {
                ui->resumeButton->hide();
            }

            // Ingredients
            if (animated) {
                group->addAnimation(fadeAnimation(components,false,time));
            } else {
                components->hide();
            }

            // Equipments
            if (animated) {
                group->addAnimation(fadeAnimation(equipments,false,time));
            } else {
                equipments->hide();
            }

            // Show button
            targetSize = QSize(this->width()-2*borderSize,buttonHeight);
            ui->showButton->setFixedSize(targetSize);
            ui->showButton->raise();

            // Menu button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->menuButton,targetPos,time));
                group->addAnimation(fadeAnimation(ui->menuButton,true,time));
            } else {
                ui->menuButton->move(targetPos);
            }

            // Note button
            targetPos = QPoint(this->width()- borderMenuButton- (ui->menuButton->width() + ui->noteButton->width())/2 , borderSize + ui->menuButton->height() + interImageSpace);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->noteButton,targetPos,time));
            } else {
                ui->noteButton->move(targetPos);
            }
            if (step->getNotes().isEmpty()) {
                ui->noteButton->hide();
            } else {
                ui->noteButton->show();
            }

            // Edition Buttons
            int posSaveCancelX = (width()/2-2*borderSize-ui->saveButton->width())/2 + borderSize;
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->saveButton,QPoint(posSaveCancelX,getSize(modes::edition).height()),time));
                group->addAnimation(targetPositionAnimation(ui->cancelButton,QPoint(width()/2 + posSaveCancelX,getSize(modes::edition).height()),time));
                group->addAnimation(targetPositionAnimation(ui->upButton,QPoint(this->width(),0),time));
                group->addAnimation(targetPositionAnimation(ui->downButton,QPoint(this->width(),0),time));
            } else  {
                ui->saveButton->move(QPoint(posSaveCancelX,getSize(modes::edition).height()));
                ui->cancelButton->move(QPoint(width()/2 + posSaveCancelX,getSize(modes::edition).height()));
                ui->upButton->move(QPoint(this->width(),0));
                ui->downButton->move(QPoint(this->width(),0));
            }

            QPoint targetPos = QPoint(borderSize, getSize(target).height()-ui->showButton->height());
            QSize targetSize = getSize(target);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->showButton,targetPos,time));
                group->addAnimation(targetSizeAnimation(this,targetSize,time));
            } else {
                ui->showButton->move(targetPos);
                this->setFixedSize(targetSize);
            }
            ui->showButton->raise();
        }
        break;
        case modes::edition: {
            this->setFixedWidth(this->size().width());

            // label
            ui->label->setReadOnly(false);
            ui->label->setStyleSheet("");

            ui->label->setFixedWidth(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton);
            targetSize = QSize(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton,getHeightText(target));
            targetPos = QPoint(ui->rankButton->width()+borderSize*2,borderSize);
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->label,targetSize,targetPos,time));
            } else {
                ui->label->setFixedSize(targetSize);
                ui->label->move(targetPos);
            }

            // rank button
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->rankButton,QPoint(borderSize,borderSize),time));
            } else {
                ui->rankButton->move(QPoint(borderSize,borderSize));
            }

            // Central Widget
            targetPos = QPoint(1,1);
            targetSize = QSize(width()-2,getSize(target).height()-std::max(ui->cancelButton->height(),ui->saveButton->height()) - 2*insideBorder);
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->widget,targetSize,targetPos,time));
            } else {
                ui->widget->move(targetPos);
                ui->widget->setFixedSize(targetSize);
            }


            // Process Views
            targetPos = QPoint(borderSize,borderSize + getHeightText(target) + interImageSpace);
            if (animated) {
                group->addAnimation(targetPositionAnimation(processes,targetPos,time));
                for (int i = 0; i < anims.size(); ++i) {
                    group->addAnimation(anims[i]);
                }
                group->addAnimation(fadeAnimation(processes,true,time));
            } else {
                processes->move(targetPos);
                processes->show();
            }
            group->addAnimation(processes->switchMode(target,animated,time));

            // Images
            QList<QPoint> posList = arrangeImages(target);
            for (int i = 0; i < images.size(); ++i) {
                if (animated) {
                    group->addAnimation(fadeAnimation(images[i],true,time));
                    group->addAnimation(targetPositionAnimation(images[i],posList[i],time));
                } else {
                    images[i]->move(posList[i]);
                    images[i]->show();
                }
                group->addAnimation(images[i]->switchMode(target,animated,time));
                images[i]->show();
                images[i]->raise();
            }

            // Display button
            checkCount();
            if (animated) {
                group->addAnimation(fadeAnimation(ui->displayButton,false,time));
            } else {
                ui->displayButton->hide();
            }

            // Resume button
            targetPos = QPoint(interImageSpace,ui->rankButton->height() + interImageSpace);
            targetSize = ui->menuButton->size();
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->resumeButton,targetSize,targetPos,time));
                group->addAnimation(fadeAnimation(ui->resumeButton,false,time));
            } else {
                ui->resumeButton->move(targetPos);
                ui->resumeButton->setFixedSize(targetSize);
                ui->resumeButton->hide();
            }

            // Components
            targetPos = QPoint(this->width()/2 + interImageSpace,borderSize + getHeightText(target) + interImageSpace);
            if (animated) {
                group->addAnimation(targetPositionAnimation(components,targetPos,time));
                group->addAnimation(fadeAnimation(components,true,time));
            } else {
                components->move(targetPos);
                components->show();
            }
            group->addAnimation(components->switchMode(target,animated,time));

            // Equipments
            targetPos = QPoint(borderSize, borderSize + getHeightText(target) + 2*interImageSpace + std::max(components->getSize(target).height(),processes->getSize(target).height()));
            if (animated) {
                group->addAnimation(targetPositionAnimation(equipments,targetPos,time));
                for (int i = 0; i < anims.size(); ++i) {
                    group->addAnimation(anims[i]);
                }
                group->addAnimation(fadeAnimation(equipments,true,time));
            } else {
                equipments->show();
                equipments->move(targetPos);
            }
            group->addAnimation(equipments->switchMode(target,animated,time));

            // Show button
            targetSize = QSize(this->width()-2*borderSize,buttonHeight);
            ui->showButton->setFixedSize(targetSize);

            // Menu button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->menuButton,targetPos,time));
                group->addAnimation(fadeAnimation(ui->menuButton,false,time));
            } else {
                ui->menuButton->move(targetPos);
            }

            // Note button
            targetPos = QPoint(this->width(), 2*borderSize + ui->menuButton->height());
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->noteButton,targetPos,time));
            } else {
                ui->noteButton->move(targetPos);
            }
            if (step->getNotes().isEmpty()) {
                ui->noteButton->hide();
            } else {
                ui->noteButton->show();
            }

            // Edition Buttons
            int posSaveCancelX = (width()/2-2*borderSize-ui->saveButton->width())/2 + borderSize;
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->saveButton,QPoint(posSaveCancelX,getSize(target).height()-ui->saveButton->height()-interImageSpace),time));
                group->addAnimation(targetPositionAnimation(ui->cancelButton,QPoint(width()/2+posSaveCancelX,getSize(target).height()-ui->cancelButton->height()-interImageSpace),time));
                int interButton = (getHeightText(modes::edition) - ui->upButton->height() - ui->downButton->height())/3;
                group->addAnimation(targetPositionAnimation(ui->upButton,QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->upButton->width())/2 - ui->upButton->width(),borderSize+interButton),time));
                group->addAnimation(targetPositionAnimation(ui->downButton,QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->downButton->width())/2 - ui->downButton->width(),borderSize+2*interButton+ui->upButton->height()),time));
            } else  {
                ui->saveButton->move(QPoint(posSaveCancelX,getSize(target).height()-ui->saveButton->height()-interImageSpace));
                ui->cancelButton->move(QPoint(width()/2+posSaveCancelX,getSize(target).height()-ui->cancelButton->height()-interImageSpace));
                int interButton = (getHeightText(modes::edition) - ui->upButton->height() - ui->downButton->height())/3;
                ui->upButton->move(QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->upButton->width())/2 - ui->upButton->width(),borderSize+interButton));
                ui->downButton->move(QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->downButton->width())/2 - ui->downButton->width(),borderSize+2*interButton+ui->upButton->height()));
            }

            QPoint targetPos = QPoint(borderSize, getSize(target).height());
            QSize targetSize = getSize(target);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->showButton,targetPos,time));
                group->addAnimation(targetSizeAnimation(this,targetSize,time));
            } else {
                ui->showButton->move(targetPos);
                this->setFixedSize(targetSize);
            }

        }
        break;
        case modes::minimal: {
            // label
            ui->label->setReadOnly(true);
            ui->label->setStyleSheet("QTextEdit {"
                                     "  border : 1px solid white;"
                                     "  background: transparent;"
                                     "}");
            ui->label->setFixedWidth(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton);
            targetSize = QSize(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton,getHeightText(modes::resume));
            targetPos = QPoint(ui->rankButton->width()+borderSize*2,borderSize);
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->label,targetSize,targetPos,time));
            } else {
                ui->label->setFixedSize(targetSize);
                ui->label->move(targetPos);
            }

            // rank button
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->rankButton,QPoint(borderSize,borderSize),time));
            } else {
                ui->rankButton->move(borderSize,borderSize);
            }

            // Central Widget
            targetPos = QPoint(1,1);
            targetSize = QSize(width()-2,getSize(target).height()-ui->showButton->height() - insideBorder);
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->widget,targetSize,targetPos,time));
            } else {
                ui->widget->move(targetPos);
                ui->widget->setFixedSize(targetSize);
            }


            // Process Views
            targetPos = QPoint((this->width() - 2*borderSize - processes->getSize(target).width())/2,borderSize + std::max(ui->rankButton->height(),getHeightText(target)));
            if (animated) {
                group->addAnimation(targetPositionAnimation(processes,targetPos,time));
            } else {
                processes->move(targetPos);
            }
            if (mode == modes::edition || !animated)
                group->addAnimation(processes->switchMode(target,animated,time));

            // Images
            QList<QPoint> posList = arrangeImages(target);
            for (int i = 0; i < images.size(); ++i) {
                if (animated) {
                    group->addAnimation(fadeAnimation(images[i],false,time));
                    group->addAnimation(targetPositionAnimation(images[i],posList[i],time));
                } else {
                    images[i]->hide();
                    images[i]->move(posList[i]);
                }
                group->addAnimation(images[i]->switchMode(target,animated,time));
            }

            // Menu Button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->menuButton,targetPos,time));
                group->addAnimation(fadeAnimation(ui->menuButton,true,time));
            } else {
                ui->menuButton->move(targetPos);
                ui->menuButton->show();
            }

            // Hide
            if (animated) {
                group->addAnimation(fadeAnimation(ui->displayButton,false,time));
                group->addAnimation(fadeAnimation(ui->resumeButton,false,time));
                group->addAnimation(fadeAnimation(components,false,time));
                group->addAnimation(fadeAnimation(equipments,false,time));
                group->addAnimation(fadeAnimation(ui->noteButton,!step->getNotes().isEmpty(),time));

            } else {
                ui->displayButton->hide();
                ui->resumeButton->hide();
                components->hide();
                equipments->hide();
                ui->noteButton->setHidden(step->getNotes().isEmpty());
            }

            // Resume Button
            targetPos = QPoint(interImageSpace,ui->rankButton->height() + interImageSpace);
            ui->resumeButton->move(targetPos);

            // Components
            targetPos = QPoint(limit + interImageSpace,borderSize+std::max(ui->rankButton->height(),getHeightText(target))
                               + processes->getSize(target).height() + interImageSpace);
            components->move(targetPos);
            group->addAnimation(components->switchMode(target,animated,time));

            //Equipments
            targetPos = QPoint(limit + interImageSpace, borderSize+std::max(ui->rankButton->height(),getHeightText(target))
                               + processes->getSize(target).height() + interImageSpace
                               + components->getSize(target).height() + (components->isEmpty()?0:interImageSpace));
            equipments->move(targetPos);


            // Edition Buttons
            int posSaveCancelX = (width()/2-2*borderSize-ui->saveButton->width())/2 + borderSize;
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->saveButton,QPoint(posSaveCancelX,getSize(modes::edition).height()),time));
                group->addAnimation(targetPositionAnimation(ui->cancelButton,QPoint(width()/2 + posSaveCancelX,getSize(modes::edition).height()),time));
                group->addAnimation(targetPositionAnimation(ui->upButton,QPoint(this->width(),0),time));
                group->addAnimation(targetPositionAnimation(ui->downButton,QPoint(this->width(),0),time));
            } else  {
                ui->saveButton->move(QPoint(posSaveCancelX,getSize(modes::edition).height()));
                ui->cancelButton->move(QPoint(width()/2 + posSaveCancelX,getSize(modes::edition).height()));
                ui->upButton->move(QPoint(this->width(),0));
                ui->downButton->move(QPoint(this->width(),0));
            }
            QPoint targetPos = QPoint(borderSize, getSize(target).height()-ui->showButton->height());
            QSize targetSize = getSize(target);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->showButton,targetPos,time));
                group->addAnimation(targetSizeAnimation(this,targetSize,time));
            } else {
                ui->showButton->move(targetPos);
                this->setFixedSize(targetSize);
            }

            break;
        }
        default:
            break;
    }

    ui->showButton->show();
    ui->showButton->raise();

    mode = target;
    return group;
}

QSize c_stepView::getSize(modes target) const {
    QSize res(stepWidth,0);
    int rightSide, leftSide;
    if (target == modes::none)
        target = mode;
    switch (target) {
        case modes::resume: {
            rightSide = components->getSize(target).height() + (components->isEmpty()?0:interImageSpace)
                        + equipments->getSize(target).height() + (equipments->isEmpty()?0:interImageSpace)
                        + (!components->isEmpty() || !equipments->isEmpty()?interImageSpace:0);
            leftSide = getImagesMaxHeigth(target) + (countImages>maxNumberImages/2 ? ui->displayButton->height() + interImageSpace : 0) + interImageSpace;
            res.setHeight(borderSize + std::max(ui->rankButton->height(),getHeightText(target))
                            + processes->getSize(target).height() + interImageSpace
                            + std::max(leftSide,rightSide)
                            + ui->showButton->height());
        }
        break;
        case modes::display: {
            res.setHeight(std::max(ui->rankButton->height(),getHeightText(target)) + borderSize + ui->showButton->height()
                            + processes->getSize(target).height() + (processes->isEmpty()?borderSize:2*interImageSpace)
                            + getImagesMaxHeigth(target) + borderSize);
        }
        break;
        case modes::edition: {
            res.setHeight(borderSize + getHeightText(target) + 4*interImageSpace
                            + std::max(components->getSize(target).height(),processes->getSize(target).height())
                            + std::max(ui->saveButton->height(),ui->cancelButton->height()) + interImageSpace
                            + equipments->getSize(target).height()
                            + getImagesMaxHeigth(target));
        }
        break;
        case modes::minimal:
            res.setHeight(std::max(ui->rankButton->height(),getHeightText(target)) + borderSize + ui->showButton->height()
                            + processes->getSize(target).height() + interImageSpace);
        break;
    default:
        break;
    }
    return res;
}

void c_stepView::resizeEvent(QResizeEvent *) {
    repaint();
}

QList<QPoint> c_stepView::arrangeImages(modes target) {
    QList<QPoint> res;
    int totalWidth = 0;
    int countImage, resumeImagesCount;
    QPoint point;
    if (target == modes::minimal) {
        target = defaultMode;
    }
    switch (target) {
        case modes::display:
                countImage = 0;
                for (int i = 0; i < images.size(); ++i) {
                    if (!images[i]->isEmpty()) {
                        totalWidth += images[i]->getSize(target).width();
                        countImage++;
                    }
                }
                point = QPoint( (this->width() - 2*borderSize - totalWidth - ((countImage-1)*interImageSpace))/2 + borderSize,
                                borderSize + std::max(ui->rankButton->height(),getHeightText(target))
                                + processes->getSize(target).height() + insideBorder);
                for (int i = 0; i < images.size(); ++i) {
                    res.push_back(point);
                    point += QPoint(images[i]->getSize(target).width() + interImageSpace,0);
                }
            break;
        case modes::resume:
                countImage = 0;
                for (int i = 0; i < images.size(); ++i) {
                    if (!images[i]->isEmpty() && countImage<maxNumberImages/2) {
                        totalWidth += images[i]->getSize(target).width();
                        countImage++;
                    }
                }
                point = QPoint( (limit - borderSize - totalWidth - ((countImage)*interImageSpace))/2 + borderSize,
                                borderSize + std::max(ui->rankButton->height(),getHeightText(target)) +
                                    + processes->getSize(target).height() + insideBorder);
                resumeImagesCount = 0;
                for (int i = 0; i < images.size(); ++i) {
                    if ((resumeImagesCount < maxNumberImages/2) && !images[i]->isEmpty()) {
                        res.push_back(point);
                        point += QPoint(images[i]->getSize(target).width() + interImageSpace,0);
                        ++resumeImagesCount;
                    } else {
                        res.push_back(QPoint(this->width(),0));
                    }
                }
            break;
        case modes::edition:
            for (int i = 0; i < images.size(); ++i) {
                totalWidth += images[i]->getSize(target).width();
            }
            point = QPoint((this->width() - 2*borderSize - totalWidth - ((maxNumberImages-1)*interImageSpace))/2 + borderSize,
                           borderSize + getHeightText(modes::edition) + 3*interImageSpace + equipments->getSize(modes::edition).height()
                           + std::max(components->getSize(target).height(),processes->getSize(target).height()));
            for (int i = 0; i < images.size(); ++i) {
                res.push_back(point);
                point += QPoint(images[i]->getSize(target).width() + interImageSpace,0);
            }
            break;
    default:
        break;
    }
    return res;
}

int c_stepView::getImagesMaxHeigth(modes target) const {
    int max = 0;
    if (target ==  modes::minimal) {
        target = defaultMode;
    }
    switch (target) {
        case modes::display:
            for (int i = 0; i < images.size(); ++i) {
                if (images[i]->height() > max && !images[i]->isEmpty()) {
                    max = images[i]->getSize(target).height();
                }
            }
            break;
        case modes::resume:
            for (int i = 0; i < images.size(); ++i) {
                if (images[i]->height() > max && !images[i]->isEmpty()) {
                    max = images[i]->getSize(target).height();
                }
            }
            break;
        case modes::edition:
            for (int i = 0; i < images.size(); ++i) {
                if (images[i]->getSize(target).height() > max) {
                    max = images[i]->getSize(target).height();
                }
            }
            break;
    default:
        break;
    }
    return max;
}

int c_stepView::getLimit() const {
    return limit;
}

void c_stepView::updateLimit() {
    if (mode != modes::setup) {
        checkCount();
    }
    if (countImages != 0) {
        limit = this->width() - borderSize - interImageSpace - components->getSize(modes::resume).width();
        limit = limit>(this->width()/3)*2?(this->width()/3)*2:limit;
    } else {
        limit = 0;
    }
}

c_step *c_stepView::getStep() const {
    return step;
}
