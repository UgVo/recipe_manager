#include "c_stepview.h"
#include "ui_c_step_view.h"

QSize c_stepView::maxSizeImage((stepWidth/3)*2,(stepWidth/3)*2);
int c_stepView::interImageSpace = 5;
int c_stepView::borderSize = 9;
int c_stepView::showButtonHeight = 21;
int c_stepView::borderMenuButton = 6;
int c_stepView::maxNumberImages = 4;

c_stepView::c_stepView(c_step *_step, QWidget *parent) :
    c_widget(parent),
    ui(new Ui::c_stepView), step(_step) {
    ui->setupUi(this);

    mode = modes::setup;

    QRect rect(2,2,46,46);
    QRegion region(rect, QRegion::Ellipse);
    ui->rankButton->setMask(region);
    QObject::connect(ui->showButton,&QPushButton::released,this,&c_stepView::triggerShowButton);

    ui->label->append(step->getDescription());
    ui->label->setAlignment(Qt::AlignJustify);
    QObject::connect(ui->label,&QTextEdit::textChanged, [=] () {
        if (getHeightText(mode) != ui->label->height()) {
            switchMode(mode,true,500);
        }
    });

    ui->rankButton->setText(QString("%1").arg(step->getRank()));
    rankEdit = step->getRank();

    ui->saveButton->setFixedWidth(ui->rankButton->width());
    ui->saveButton->setFixedSize(ui->saveButton->size());
    QObject::connect(ui->saveButton,&QPushButton::clicked,this,&c_stepView::editSaved);
    ui->cancelButton->setFixedSize(ui->saveButton->size());
    QObject::connect(ui->cancelButton,&QPushButton::clicked,this,&c_stepView::editCanceled);

    ui->upButton->setFixedSize(ui->upButton->size());
    QObject::connect(ui->upButton,&QPushButton::clicked, [this] () {
        emit upRank();
    });
    ui->downButton->setFixedSize(ui->downButton->size());
    QObject::connect(ui->downButton,&QPushButton::clicked, [this] () {
        emit downRank();
    });

    ui->displayButton->hide();
    QObject::connect(ui->displayButton,&QPushButton::clicked,[=] () {
        switchMode(modes::display);
    });

    ui->resumeButton->hide();
    QObject::connect(ui->resumeButton,&QPushButton::clicked,[=] () {
        switchMode(modes::resume);
    });

    QList<QString> imageStringList = step->getImagesUrl();
    while (imageStringList.size() > maxNumberImages) {
        imageStringList.removeLast();
    }

    countImages = imageStringList.size();

    components = new c_componentView(step->getComponentsPtr(),this);
    QObject::connect(components,&c_componentView::resized, [=] () {
        switchMode(mode,true,500);
    });

    updateLimit();

    for (int i = 0; i < imageStringList.size(); ++i) {
        images.push_back(new c_image(imageStringList[i],this));
    }
    for (int i = imageStringList.size(); i < maxNumberImages; ++i) {
        images.push_back(new c_image("",this));       
    }
    for (int i = 0; i < images.size(); ++i) {
        QObject::connect(images[i],&c_image::newImage,this,&c_stepView::imageAdded);
        QObject::connect(images[i],&c_image::resized, [=] () {
            switchMode(mode,true,500);
        });
    }

    QList<c_process* > processList = step->getProcessingsPtr();
    processes = new c_processView(processList,this);
    processes->show();

    equipments = new c_equipementsView(step->getEquipments(),this);
    equipments->lower();

    QMenu *menu = new QMenu();
    menu->addAction("Edit",[this] () {
        if (state == states::retracted) {
            QAbstractAnimation *stateAnime = switchState(states::opened);
            QObject::connect(stateAnime,&QAbstractAnimation::finished, [=] () {
                switchMode(modes::edition);
            });
            stateAnime->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            switchMode(modes::edition);
        }
    });
    menu->addAction("Delete", [this] () {
        emit toDelete(this);
    });
    menu->addAction("Add note",this,&c_stepView::slotAddNote);

    ui->menuButton->setMenu(menu);
    ui->rankButton->move(borderSize,borderSize);

    noteDialog = new c_notesDialog(step->getNotesPtr(),this);
    noteDialog->hide();
    QObject::connect(ui->noteButton,&QPushButton::released,this,&c_stepView::slotShowNotes);

    state = states::retracted;
    defaultMode = (components->isEmpty() && equipments->isEmpty()) ? modes::display : modes::resume;
    mode = defaultMode;

    switchMode(mode,false);
}

c_stepView::~c_stepView() {
    delete ui;
    delete noteDialog;
}

void c_stepView::setRank(int rank) {
    rankEdit = rank;
    step->setRank(rank);
}

void c_stepView::triggerShowButton() {
    ui->showButton->raise();
    switch (state) {
        case states::retracted: {
            QParallelAnimationGroup *group = new QParallelAnimationGroup;
            group->addAnimation(switchState(states::opened));
            group->start(QAbstractAnimation::DeleteWhenStopped);
        }
        break;
        case states::opened: {
            QParallelAnimationGroup *group = new QParallelAnimationGroup;
            group->addAnimation(switchState(states::retracted));
            group->start(QAbstractAnimation::DeleteWhenStopped);
        }
        break;
        default:
            break;
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
    res.remove(QRegExp("\n$"));
    step->setDescription(res);

    QObject::disconnect(ui->saveButton,&QPushButton::released,this,&c_stepView::editSaved);
    QObject::disconnect(ui->cancelButton,&QPushButton::released,this,&c_stepView::editCanceled);

    updateLimit();

    defaultMode = (components->isEmpty() && equipments->isEmpty()) ? modes::display : modes::resume;
    switchMode(defaultMode);

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
    switchMode(defaultMode);
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
    switchMode(mode,true,500,animations);
    checkCount();
}

c_note *c_stepView::addNoteToStep(c_note* newNote) {
    c_note *note = step->newNote();
    *note = *newNote;
    return note;
}

void c_stepView::deleteNote(c_note *note) {
    step->deleteNote(note);
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
    if (targetMode == modes::display || targetMode == modes::resume) {
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
            QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
            ui->label->setReadOnly(true);
            ui->label->setStyleSheet("QTextEdit {"
                                     "  border : 1px solid white;"
                                     "  background: transparent;"
                                     "}");
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

            // Process Views
            targetPos = QPoint((this->width() - 2*borderSize - processes->getSize(target).width())/2,borderSize + interImageSpace + std::max(ui->rankButton->height(),getHeightText(target)));
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
                    group->addAnimation(targetPositionAnimation(images[i],posList[i],time));
                } else {
                    images[i]->move(posList[i]);
                }
                group->addAnimation(images[i]->switchMode(target,animated,time));
                if (state == states::retracted) {
                    images[i]->hide();
                } else {
                    images[i]->show();
                }
            }

            // Display button
            checkCount();
            if (countImages>maxNumberImages/2) {
                int y = borderSize + std::max(getHeightText(target),ui->rankButton->height()) + processes->getSize(target).height() + (processes->isEmpty()?borderSize:2*interImageSpace)
                        + getImagesMaxHeigth(target) + interImageSpace;
                ui->displayButton->move(QPoint((limit-borderSize-interImageSpace)/2+borderSize - ui->displayButton->width()/2,y));
                ui->displayButton->setText(QString("+%1").arg(countImages-maxNumberImages/2));
                if (animated) {
                    QPropertyAnimation* anim = fadeAnimation(ui->displayButton,true,time);
                    if (anim != nullptr)
                        group->addAnimation(anim);
                } else {
                    ui->displayButton->show();
                }
                ui->displayButton->lower();
            } else {
                if (animated) {
                    QPropertyAnimation* anim = fadeAnimation(ui->displayButton,false,time);
                    if (anim != nullptr)
                        group->addAnimation(anim);
                } else {
                    ui->displayButton->hide();
                }
            }

            // Resume button
            targetPos = QPoint(interImageSpace,ui->rankButton->height() + interImageSpace);
            targetSize = ui->menuButton->size();
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->resumeButton,targetSize,targetPos,time));
                QPropertyAnimation* anim = fadeAnimation(ui->resumeButton,false,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->resumeButton->move(targetPos);
                ui->resumeButton->setFixedSize(targetSize);
                ui->resumeButton->hide();
            }

            // Components
            targetPos = QPoint(limit + interImageSpace,borderSize+std::max(ui->rankButton->height(),getHeightText(target))
                               + processes->getSize(target).height() + (processes->isEmpty()?borderSize:2*interImageSpace));
            if (animated) {
                group->addAnimation(targetPositionAnimation(components,targetPos,time));
                QPropertyAnimation* anim = fadeAnimation(components,true,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                components->move(targetPos);
                components->show();
            }
            group->addAnimation(components->switchMode(target,animated,time));

            // Equipments
            targetPos = QPoint(limit + interImageSpace, borderSize+std::max(ui->rankButton->height(),getHeightText(target))
                               + processes->getSize(target).height() + (processes->isEmpty()?borderSize:2*interImageSpace)
                               + components->getSize(target).height() + (components->isEmpty()?0:interImageSpace));
            if (animated) {
                group->addAnimation(targetPositionAnimation(equipments,targetPos,time));
                for (int i = 0; i < anims.size(); ++i) {
                    group->addAnimation(anims[i]);
                }
                QPropertyAnimation* anim = fadeAnimation(equipments,true,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                equipments->move(targetPos);
                equipments->show();
            }
            group->addAnimation(equipments->switchMode(target,animated,time));

            // Show button
            targetSize = QSize(this->width()-2*borderSize,showButtonHeight);
            ui->showButton->setFixedSize(targetSize);
            ui->showButton->raise();

            // Menu button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->menuButton,targetPos,time));
                QPropertyAnimation* anim = fadeAnimation(ui->menuButton,true,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
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
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->saveButton,QPoint(-ui->saveButton->width(),0),time));
                group->addAnimation(targetPositionAnimation(ui->cancelButton,QPoint(-ui->cancelButton->width(),0),time));
                group->addAnimation(targetPositionAnimation(ui->upButton,QPoint(this->width(),0),time));
                group->addAnimation(targetPositionAnimation(ui->downButton,QPoint(this->width(),0),time));
            } else  {
                ui->saveButton->move(QPoint(-ui->saveButton->width(),0));
                ui->cancelButton->move(QPoint(-ui->cancelButton->width(),0));
                ui->upButton->move(QPoint(this->width(),0));
                ui->downButton->move(QPoint(this->width(),0));
            }

            ui->widgetButton->raise();
            mode = target;
        }
        break;
        case modes::display: {
            this->setFixedWidth(this->size().width());

            // label
            QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
            ui->label->setReadOnly(true);
            ui->label->setStyleSheet("QTextEdit {"
                                     "  border : 1px solid white;"
                                     "  background: transparent;"
                                     "}");
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

            // Process Views
            targetPos = QPoint((this->width() - 2*borderSize - processes->getSize(target).width())/2,borderSize + interImageSpace + std::max(ui->rankButton->height(),getHeightText(target)));
            if (animated) {
                group->addAnimation(targetPositionAnimation(processes,targetPos,time));
            } else {
                processes->move(targetPos);
            }
//            if (mode != modes::edition)
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
                if (state == states::retracted) {
                    images[i]->hide();
                } else {
                    images[i]->show();
                }
            }

            // Display button
            checkCount();
            if (animated) {
                QPropertyAnimation* anim = fadeAnimation(ui->displayButton,false,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->displayButton->hide();
            }

            // Resume button
            if (defaultMode != display) {
                targetPos = QPoint(interImageSpace,ui->rankButton->height() + interImageSpace + borderSize);
                qDebug() << processes->getSize(target).height();
                targetSize = ui->menuButton->size();
                if (animated) {
                    group->addAnimation(targetGeometryAnimation(ui->resumeButton,targetSize,targetPos,time));
                    QPropertyAnimation* anim = fadeAnimation(ui->resumeButton,true,time);
                    if (anim != nullptr)
                        group->addAnimation(anim);
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
                QPropertyAnimation* anim = fadeAnimation(components,false,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                components->hide();
            }
            components->lower();

            // Equipments
            if (animated) {
                QPropertyAnimation* anim = fadeAnimation(equipments,false,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                equipments->hide();
            }
            equipments->lower();

            // Show button
            targetSize = QSize(this->width()-2*borderSize,showButtonHeight);
            ui->showButton->setFixedSize(targetSize);
            ui->showButton->raise();

            // Menu button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->menuButton,targetPos,time));
                QPropertyAnimation* anim = fadeAnimation(ui->menuButton,true,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
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
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->saveButton,QPoint(-ui->saveButton->width(),0),time));
                group->addAnimation(targetPositionAnimation(ui->cancelButton,QPoint(-ui->cancelButton->width(),0),time));
                group->addAnimation(targetPositionAnimation(ui->upButton,QPoint(this->width(),0),time));
                group->addAnimation(targetPositionAnimation(ui->downButton,QPoint(this->width(),0),time));
            } else  {
                ui->saveButton->move(QPoint(-ui->saveButton->width(),0));
                ui->cancelButton->move(QPoint(-ui->cancelButton->width(),0));
                ui->upButton->move(QPoint(this->width(),0));
                ui->downButton->move(QPoint(this->width(),0));
            }

            ui->widgetButton->raise();
            mode = target;
        }
        break;
        case modes::edition: {
            this->setFixedWidth(this->size().width());

            // label
            QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
            ui->label->setReadOnly(false);
            ui->label->setStyleSheet("");
            ui->label->raise();

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
                group->addAnimation(targetPositionAnimation(ui->rankButton,QPoint(ui->label->x(),borderSize),time));
            } else {
                ui->rankButton->move(ui->label->x(),borderSize);
            }

            // Process Views
            targetPos = QPoint(borderSize,borderSize + getHeightText(target) + interImageSpace);
            processes->lower();
            if (animated) {
                group->addAnimation(targetPositionAnimation(processes,targetPos,time));
                for (int i = 0; i < anims.size(); ++i) {
                    group->addAnimation(anims[i]);
                }
                QPropertyAnimation* anim = fadeAnimation(processes,true,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                processes->move(targetPos);
                processes->show();
            }
            group->addAnimation(processes->switchMode(target,animated,time));

            // Images
            QList<QPoint> posList = arrangeImages(target);
            for (int i = 0; i < images.size(); ++i) {
                if (animated) {
                    group->addAnimation(targetPositionAnimation(images[i],posList[i],time));
                } else {
                    images[i]->move(posList[i]);
                }
                group->addAnimation(images[i]->switchMode(target,animated,time));
                if (state == states::retracted) {
                    images[i]->hide();
                } else {
                    images[i]->show();
                }
            }

            // Display button
            checkCount();
            if (animated) {
                QPropertyAnimation* anim = fadeAnimation(ui->displayButton,false,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->displayButton->hide();
            }

            // Resume button
            targetPos = QPoint(interImageSpace,ui->rankButton->height() + interImageSpace);
            targetSize = ui->menuButton->size();
            if (animated) {
                group->addAnimation(targetGeometryAnimation(ui->resumeButton,targetSize,targetPos,time));
                QPropertyAnimation* anim = fadeAnimation(ui->resumeButton,false,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->resumeButton->move(targetPos);
                ui->resumeButton->setFixedSize(targetSize);
                ui->resumeButton->hide();
            }

            // Components
            targetPos = QPoint(this->width()/2 + interImageSpace,borderSize + getHeightText(target) + interImageSpace);
            if (animated) {
                group->addAnimation(targetPositionAnimation(components,targetPos,time));
                QPropertyAnimation* anim = fadeAnimation(components,true,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
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
                QPropertyAnimation* anim = fadeAnimation(equipments,true,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                equipments->show();
                equipments->move(targetPos);
            }
            group->addAnimation(equipments->switchMode(target,animated,time));

            // Show button
            targetSize = QSize(this->width()-2*borderSize,showButtonHeight);
            ui->showButton->setFixedSize(targetSize);
            ui->showButton->raise();

            // Menu button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(targetPositionAnimation(ui->menuButton,targetPos,time));
                QPropertyAnimation* anim = fadeAnimation(ui->menuButton,false,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
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
            if (animated) {
                int interButton = (getHeightText(modes::edition) - ui->saveButton->height() - ui->cancelButton->height())/3;
                group->addAnimation(targetPositionAnimation(ui->saveButton,QPoint(borderSize,borderSize+interButton),time));
                group->addAnimation(targetPositionAnimation(ui->cancelButton,QPoint(borderSize,borderSize+2*interButton + ui->saveButton->height()),time));
                interButton = (getHeightText(modes::edition) - ui->upButton->height() - ui->downButton->height())/3;
                group->addAnimation(targetPositionAnimation(ui->upButton,QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->upButton->width())/2 - ui->upButton->width(),borderSize+interButton),time));
                group->addAnimation(targetPositionAnimation(ui->downButton,QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->downButton->width())/2 - ui->downButton->width(),borderSize+2*interButton+ui->upButton->height()),time));
            } else  {
                int interButton = (getHeightText(modes::edition) - ui->saveButton->height() - ui->cancelButton->height())/3;
                ui->saveButton->move(QPoint(borderSize,borderSize+interButton));
                ui->cancelButton->move(QPoint(borderSize,borderSize+2*interButton + ui->saveButton->height()));
                interButton = (getHeightText(modes::edition) - ui->upButton->height() - ui->downButton->height())/3;
                ui->upButton->move(QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->upButton->width())/2 - ui->upButton->width(),borderSize+interButton));
                ui->downButton->move(QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->downButton->width())/2 - ui->downButton->width(),borderSize+2*interButton+ui->upButton->height()));
            }

            ui->widgetButton->raise();
            mode = target;
        }
        break;
        default:
            break;
    }

    group->addAnimation(switchState(state,animated,time));
    group->start(QAbstractAnimation::DeleteWhenStopped);
    mode = target;

    qDebug() << countImages << limit << components->width();

    return group;
}

QAnimationGroup *c_stepView::switchState(states targetState, bool animated, int time) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QPropertyAnimation *anim = nullptr;
    switch (targetState) {
        case states::retracted: {
            for (int i = 0; i < images.size(); ++i) {
                if (!images[i]->isEmpty()) {
                    if (animated) {
                        anim = fadeAnimation(images[i],false,time);
                        if (anim != nullptr)
                            group->addAnimation(anim);
                        QObject::connect(anim,&QPropertyAnimation::finished, [this,i] () {images[i]->hide();});
                        images[i]->show();
                    } else {
                        images[i]->hide();
                    }
                }
            }
        }
        break;
        case states::opened: {
            for (int i = 0; i < images.size(); ++i) {
                if (!images[i]->isEmpty()) {
                    if (animated) {
                        anim = fadeAnimation(images[i],true,time);
                        if (anim != nullptr)
                            group->addAnimation(anim);
                    }
                    images[i]->show();
                }
            }
        }
        break;
    default:
        break;
    }

    QPoint targetPos = QPoint(borderSize, getHeightWidget(mode,targetState)-ui->showButton->height());
    QSize targetSize = QSize(this->width(), getHeightWidget(mode,targetState));
    if (animated) {
        group->addAnimation(targetPositionAnimation(ui->showButton,targetPos,time));
        group->addAnimation(targetSizeAnimation(this,targetSize,time));
    } else {
        ui->showButton->move(targetPos);
        this->setFixedSize(targetSize);
    }
    ui->showButton->raise();
    state = targetState;
    return group;
}

QSize c_stepView::getSize(modes target) const {
    return QSize(stepWidth,getHeightWidget(target,state));
}

int c_stepView::getHeightWidget(modes targetMode, states targetState) const {
    int rightSide, leftSide;
    switch (targetMode) {
        case modes::resume: {
            switch (targetState) {
                case states::retracted:
                    return std::max(ui->rankButton->height(),getHeightText(targetMode)) + borderSize + ui->showButton->height()
                            + processes->getSize(targetMode).height() + (processes->isEmpty()?borderSize:2*interImageSpace);
                case states::opened:
                    qDebug() << components->getSize(targetMode).height() + (components->isEmpty()?0:interImageSpace)
                             << equipments->getSize(targetMode).height() + (equipments->isEmpty()?0:interImageSpace)
                             << (!components->isEmpty() || !equipments->isEmpty()?interImageSpace:0);
                    rightSide = components->getSize(targetMode).height() + (components->isEmpty()?0:interImageSpace)
                            + equipments->getSize(targetMode).height() + (equipments->isEmpty()?0:interImageSpace)
                            + (!components->isEmpty() || !equipments->isEmpty()?interImageSpace:0);
                    leftSide = getImagesMaxHeigth(targetMode) + (countImages>maxNumberImages/2 ? ui->displayButton->height() + interImageSpace : 0) + interImageSpace;
                    return borderSize + std::max(ui->rankButton->height(),getHeightText(targetMode))
                            + processes->getSize(targetMode).height() + (processes->isEmpty()?borderSize:2*interImageSpace)
                            + std::max(leftSide,rightSide)
                            + ui->showButton->height();
                default:
                    break;
            }
        }
        break;
        case modes::display: {
            switch (targetState) {
                case states::retracted:
                    return std::max(ui->rankButton->height(),getHeightText(targetMode)) + borderSize + ui->showButton->height()
                            + processes->getSize(targetMode).height() + (processes->isEmpty()?borderSize:2*interImageSpace);
                case states::opened:
                    return std::max(ui->rankButton->height(),getHeightText(targetMode)) + borderSize + ui->showButton->height()
                            + processes->getSize(targetMode).height() + (processes->isEmpty()?borderSize:2*interImageSpace)
                            + getImagesMaxHeigth(targetMode) + borderSize;
                default:
                    break;
            }
        }
        break;
        case modes::edition: {
            switch (targetState) {
                case states::retracted:
                    return getHeightText(modes::edition) + 2*borderSize + ui->showButton->height();
                case states::opened:
                    return  borderSize + getHeightText(targetMode) + 4*interImageSpace
                            + ui->showButton->height()
                            + std::max(components->getSize(targetMode).height(),processes->getSize(targetMode).height())
                            + equipments->getSize(targetMode).height()
                            + getImagesMaxHeigth(targetMode);
                default:
                    break;
            }
        }
        break;
    default:
        break;
    }
    return 0;
}

QList<QPoint> c_stepView::arrangeImages(modes target) {
    QList<QPoint> res;
    int totalWidth = 0;
    int countImage, resumeImagesCount;

    QPoint point;
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
                                + processes->getSize(target).height() + (processes->isEmpty()?0:borderSize));
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
                                    + processes->getSize(target).height() + (processes->isEmpty()?0:borderSize));
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
