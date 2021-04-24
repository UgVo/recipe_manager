#include "c_stepview.h"
#include "ui_c_step_view.h"

QSize c_stepView::maxSizeImage(194,194);
int c_stepView::interImageSpace = 5;
int c_stepView::borderSize = 9;
int c_stepView::showButtonHeight = 21;
int c_stepView::borderMenuButton = 6;
int c_stepView::maxNumberImages = 4;

c_stepView::c_stepView(c_step *_step, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_stepView), step(_step) {
    ui->setupUi(this);

    QRect rect(2,2,46,46);
    QRegion region(rect, QRegion::Ellipse);
    ui->rankButton->setMask(region);
    QObject::connect(ui->showButton,&QPushButton::released,this,&c_stepView::triggerShowButton);

    ui->label->append(step->getDescription());
    ui->label->setAlignment(Qt::AlignJustify);
    QObject::connect(ui->label,&QTextEdit::textChanged,this,&c_stepView::slotTextModified);

    ui->rankButton->setText(QString("%1").arg(step->getRank()));
    rankEdit = step->getRank();

    ui->saveButton->setFixedWidth(ui->rankButton->width());
    ui->saveButton->setFixedSize(ui->saveButton->size());
    ui->cancelButton->setFixedSize(ui->saveButton->size());
    ui->upButton->setFixedSize(ui->upButton->size());
    ui->downButton->setFixedSize(ui->downButton->size());

    ui->displayButton->hide();
    QObject::connect(ui->displayButton,&QPushButton::clicked,[=] () {
        switchMode(recipe::modes::display);
    });

    ui->resumeButton->hide();
    QObject::connect(ui->resumeButton,&QPushButton::clicked,[=] () {
        switchMode(recipe::modes::resume);
    });

    ui->ingredientsLabel->hide();

    QList<QString> imageStringList = step->getImagesUrl();
    while (imageStringList.size() > maxNumberImages) {
        imageStringList.removeLast();
    }

    countImages = imageStringList.size();

    for (int i = 0; i < imageStringList.size(); ++i) {
        images.push_back(new c_image(imageStringList[i],this));
        QObject::connect(images.last(),&c_image::newImage,this,&c_stepView::imageAdded);
    }
    for (int i = imageStringList.size(); i < maxNumberImages; ++i) {
        images.push_back(new c_image("",this));
        QObject::connect(images.last(),&c_image::newImage,this,&c_stepView::imageAdded);
    }

    QList<c_process* > processList = step->getProcessingsPtr();
    for (int i = 0; i < processList.size(); ++i) {
        processes.push_back(new c_processElemView(processList[i],this));
    }

    components = new c_componentView(step->getComponentsPtr(),this);

    limit = this->width() - borderSize - interImageSpace - components->width();
    limit = limit>(this->width()/3)*2?(this->width()/3)*2:limit;

    equipements = new c_equipementsView(step->getEquipments(),this);

    QMenu *menu = new QMenu();
    menu->addAction("Edit",this,&c_stepView::editStepAnimationOn);
    menu->addAction("Delete",this,&c_stepView::slotDelete);
    menu->addAction("Add note",this,&c_stepView::slotAddNote);

    ui->menuButton->setMenu(menu);
    ui->rankButton->move(borderSize,borderSize);

    noteDialog = new c_notesDialog(step->getNotesPtr(),this);
    noteDialog->hide();
    QObject::connect(ui->noteButton,&QPushButton::released,this,&c_stepView::slotShowNotes);

    state = recipe::states::retracted;
    mode = recipe::modes::resume;

    switchMode(recipe::modes::resume,false);
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
//    lockSize(false);
    ui->showButton->raise();
    switch (state) {
        case recipe::states::retracted: {
            QParallelAnimationGroup *group = new QParallelAnimationGroup;
            QList<QPropertyAnimation *> anims = switchState(recipe::states::opened);
            for (int i = 0; i < anims.size(); ++i) {
                group->addAnimation(anims[i]);
            }
            group->start(QAbstractAnimation::DeleteWhenStopped);
        }
        break;
        case recipe::states::opened: {
            QParallelAnimationGroup *group = new QParallelAnimationGroup;
            QList<QPropertyAnimation *> anims = switchState(recipe::states::retracted);
            for (int i = 0; i < anims.size(); ++i) {
                group->addAnimation(anims[i]);
            }
            group->start(QAbstractAnimation::DeleteWhenStopped);
        }
        break;
        default:
            break;
    }
}

void c_stepView::editStepAnimationOn() {
    switchMode(recipe::modes::edition);
}

void c_stepView::editStepAnimationOff() {
    lockSize(false);
    ui->menuButton->setDisabled(false);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());

    int futureHeightLabel = getHeightText();
    int slideImages, maxHeightLabelRank;
    if (ui->label->height() < ui->rankButton->height()) {
        group->addAnimation(recipe::growAnimation(ui->label,QSize(0,futureHeightLabel - ui->label->height())));
    } else {
        if (futureHeightLabel > ui->rankButton->height()) {
            slideImages = futureHeightLabel - ui->label->height();
            maxHeightLabelRank = futureHeightLabel;
        } else {
            slideImages = ui->rankButton->height() - ui->label->height();
            maxHeightLabelRank = ui->rankButton->height();
        }
        if (!images.isEmpty()) {
            for (int i = 0; i < images.size(); ++i) {
                images[i]->updateSizes(countImages);
                QList<QPropertyAnimation*> anims = images[i]->switchMode(recipe::modes::display);
                for (QPropertyAnimation* anim: anims) {
                    group->addAnimation(anim);
                }
                if (images[i]->isEmpty())
                    group->addAnimation(recipe::fadeAnimation(images[i],false));
            }
            QList<QPropertyAnimation*> animImages = arrangeImagesEditOff(QPoint(0,slideImages));
            for (int i = 0; i < animImages.size(); ++i) {
                group->addAnimation(animImages[i]);
            }
        }
        group->addAnimation(recipe::growAnimation(ui->label,QSize(0,futureHeightLabel - ui->label->height())));

        int slide = getImagesMaxHeigth(recipe::modes::display) + maxHeightLabelRank + 3*borderSize - ui->showButton->pos().y();
        group->addAnimation(recipe::growAnimation(this,QSize(0,slide)));
        group->addAnimation(recipe::slideAnimation(ui->showButton,QPoint(0,slide)));
    }

    group->addAnimation(recipe::slideAnimation(ui->upButton,QPoint(ui->upButton->width()+borderMenuButton + (ui->menuButton->width()-ui->upButton->width())/2,0)));
    group->addAnimation(recipe::slideAnimation(ui->downButton,QPoint(ui->downButton->width()+borderMenuButton + (ui->menuButton->width()-ui->downButton->width())/2,0)));
    group->addAnimation(recipe::slideAnimation(ui->rankButton,QPoint(-(ui->rankButton->width()+borderSize),0)));
    group->addAnimation(recipe::slideAnimation(ui->saveButton,QPoint(-(ui->saveButton->width()+borderSize),0)));
    group->addAnimation(recipe::slideAnimation(ui->cancelButton,QPoint(-(ui->cancelButton->width()+borderSize),0)));
    group->addAnimation(recipe::fadeAnimation(ui->menuButton,true));

    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {
        ui->label->setReadOnly(true);
        ui->label->setStyleSheet("QTextEdit {"
                                 "  border : 1px solid white;"
                                 "  background: transparent;"
                                 "}");
        ui->label->raise();
        ui->label->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
        ui->label->setFixedWidth(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton );

        ui->label->move(ui->rankButton->width()+borderSize*2,borderSize);
        this->endTransition(recipe::states::opened);
    });
    state = recipe::states::transition;
    mode = recipe::modes::resume;
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::editSaved() {
    imageList.clear();
    for (int i = 0; i < images.size(); ++i) {
        images[i]->save();
        if (!images[i]->isEmpty())
            imageList.push_back(images[i]->getPathImage());
    }

    step->setImagesUrl(imageList);

    editStepAnimationOff();
    QObject::disconnect(ui->saveButton,&QPushButton::released,this,&c_stepView::editSaved);
    QObject::disconnect(ui->cancelButton,&QPushButton::released,this,&c_stepView::editCanceled);

    QString res;
    for (int i = 0; i < ui->label->document()->blockCount(); ++i) {
        res.append(ui->label->document()->findBlockByNumber(i).text());
        res.append("\n");
    }
    res.remove(QRegExp("\n$"));
    step->setDescription(res);
    emit saved(step);
}

void c_stepView::editCanceled() {
    ui->label->clear();
    ui->label->append(step->getDescription());
    ui->label->setAlignment(Qt::AlignJustify);

    for (int i = 0; i < images.size(); ++i) {
        images[i]->rollback();
    }

    editStepAnimationOff();
    QObject::disconnect(ui->saveButton,&QPushButton::released,this,&c_stepView::editSaved);
    QObject::disconnect(ui->cancelButton,&QPushButton::released,this,&c_stepView::editCanceled);
}

void c_stepView::upEdit() {
    if (rankEdit > 0) {
        rankEdit--;
        emit new_rank(rankEdit);
    }
}

void c_stepView::downEdit() {
    rankEdit++;
    emit new_rank(rankEdit);
}

void c_stepView::slotTextModified() {
    QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
    if (ui->label->height() - getHeightText() < metrics.height()) {
        editAreaSizeChanged(metrics.height());
    }
    if (ui->label->height() - getHeightText() > 3*metrics.height()) {
        editAreaSizeChanged(-metrics.height());
    }
}

void c_stepView::editAreaSizeChanged(int increment) {
    lockSize(false);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QRect rect;

    group->addAnimation(recipe::growAnimation(ui->label,QSize(0,increment),250));
    group->addAnimation(recipe::growAnimation(this,QSize(0,increment),250));
    group->addAnimation(recipe::slideAnimation(ui->showButton,QPoint(0,increment),QSize(),250));

    QList<QPropertyAnimation*> arrangedImages = arrangeImagesEditOn(QPoint(0,increment),250);
    for (int i = 0; i < arrangedImages.size(); ++i) {
        group->addAnimation(arrangedImages[i]);
    }

    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(recipe::states::opened);});
    state = recipe::states::transition;
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::endTransition(int _state) {
    switch (mode) {
        case recipe::modes::display:
            for (int i = 0; i < images.size(); ++i) {
                if (images[i]->isEmpty())
                    images[i]->hide();
            }
            break;
        case recipe::modes::edition:
            for (int i = 0; i < images.size(); ++i) {
                    images[i]->show();
            }
            break;
        default:
            break;
    }
    switch (_state) {
        case recipe::states::opened :
            state = recipe::states::opened;
            this->setFixedSize(this->size());
            break;
        case recipe::states::retracted :
            state = recipe::states::retracted;
            this->setFixedSize(this->size());
            for (int i = 0; i < images.size(); ++i) {
                images[i]->hide();
            }
            break;
        default:
            return ;
    }
}

void c_stepView::slotDelete() {
    emit toDelete(this);
}

void c_stepView::slotAddNote() {
    if (step->getNotes().isEmpty()) {
        ui->noteButton->hide();
    } else {
        ui->noteButton->show();
    }
    noteDialog->slotNewNote();
    noteDialog->exec();
}

void c_stepView::slotShowNotes() {
    noteDialog->exec();
}

int c_stepView::countImage(){
    return countImages;
}

void c_stepView::imageAdded(QPropertyAnimation * animations) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    int shift = ui->label->height() + borderSize*3 + getImagesMaxHeigth(recipe::modes::edition) - ui->showButton->y();
    if (shift != 0) {
        lockSize(false);
        group->addAnimation(recipe::slideAnimation(ui->showButton,QPoint(0,shift)));
        group->addAnimation(recipe::growAnimation(this,QSize(0,shift)));
    }
    group->addAnimation(animations);
    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(recipe::states::opened);});
    state = recipe::states::transition;
    group->start(QAbstractAnimation::DeleteWhenStopped);
    checkCount();
}

c_note *c_stepView::addNoteToStep(c_note* newNote) {
    return step->addNote(*newNote);
}

void c_stepView::deleteNote(c_note *note) {
    step->deleteNote(*note);
}

void c_stepView::checkCount() {
    countImages = 0;
    for (int i = 0; i < images.size(); ++i) {
        if (!images[i]->isEmpty())
            countImages++;
    }
}

int c_stepView::getHeightText(int targetMode) {
    QRect rect = ui->label->rect();
    int res = 0;
    for (int i = 0; i < ui->label->document()->blockCount(); ++i) {
        QFontMetrics metrics(ui->label->document()->findBlockByNumber(i).charFormat().font());
        res += metrics.boundingRect(rect,Qt::TextWordWrap,ui->label->document()->findBlockByNumber(i).text()).size().height();
    }
    // offset to compensate difference in height between text and QEditText
    res += 8;
    if (targetMode == recipe::modes::display || targetMode == recipe::modes::resume) {
        return res + 8;
    } else if (targetMode == recipe::modes::edition){
        QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
        if (metrics.height()*2+res > ui->rankButton->height()) {
            res = metrics.height()*2 + res;
        } else {
            res = ui->rankButton->height();
        }
    }
    return res;
}

void c_stepView::lockSize(bool flag) {
    if (flag) {
        this->setFixedSize(this->size());
    } else {
        this->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
        this->setFixedWidth(this->size().width());
    }
}

QList<QPropertyAnimation *> c_stepView::arrangeImagesEditOn(QPoint verticalShift, int time) {
    QList<QPropertyAnimation*> res;
    QList<QPoint> newPos = arrangeImages(recipe::modes::edition,verticalShift);
    for (int i = 0; i < images.size(); ++i) {
        res.push_back(recipe::slideAnimation(images[i],newPos[i]-images[i]->pos(),QSize(),time));
    }
    return res;
}

QList<QPropertyAnimation *> c_stepView::arrangeImagesEditOff(QPoint verticalShift) {
    QList<QPropertyAnimation*> res;
    QList<QPoint> newPos = arrangeImages(recipe::modes::display,verticalShift);
    int index = 0;
    for (int i = 0; i < newPos.size(); ++i) {
        if (!images[i]->isEmpty())
            res.push_back(recipe::slideAnimation(images[i],newPos[index++]-images[i]->pos()));
    }
    return res;
}

void c_stepView::switchMode(int target, bool animated, int time) {
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QSize targetSize;
    QPoint targetPos;
    switch (target) {
        case recipe::modes::resume: {
            this->setFixedWidth(this->size().width());

            // label
            QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
            ui->label->setReadOnly(true);
            ui->label->setStyleSheet("QTextEdit {"
                                     "  border : 1px solid white;"
                                     "  background: transparent;"
                                     "}");
            targetSize = QSize(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton,getHeightText(recipe::modes::resume));
            targetPos = QPoint(ui->rankButton->width()+borderSize*2,borderSize);
            if (animated) {
                group->addAnimation(recipe::targetGeometryAnimation(ui->label,targetSize,targetPos,time));
            } else {
                ui->label->setFixedSize(targetSize);
                ui->label->move(targetPos);
            }

            // rank button
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->rankButton,QPoint(borderSize,borderSize),time));
            } else {
                ui->rankButton->move(borderSize,borderSize);
            }

            // Process Views
            QList<QPoint> posProcesses = arrangeProcess(target);
            for (int i = 0; i < processes.size(); ++i) {
                processes[i]->show();
                processes[i]->switchMode(recipe::modes::resume);
                if (animated) {
                    group->addAnimation(recipe::targetPositionAnimation(processes[i],posProcesses[i],time));
                } else {
                    processes[i]->move(posProcesses[i]);
                }
            }

            // Images
            QList<QPoint> posList = arrangeImages(recipe::modes::resume);
            for (int i = 0; i < images.size(); ++i) {
                QList<QPropertyAnimation*> anims = images[i]->switchMode(recipe::modes::resume,animated);
                if (animated) {
                    group->addAnimation(recipe::targetPositionAnimation(images[i],posList[i],time));
                    for (int i = 0; i < anims.size(); ++i) {
                        group->addAnimation(anims[i]);
                    }
                } else {
                    images[i]->move(posList[i]);
                }
                if (state == recipe::states::retracted) {
                    images[i]->hide();
                } else {
                    images[i]->show();
                }
            }

            // Display button
            checkCount();
            if (countImages>maxNumberImages/2) {
                int y = borderSize + ui->label->height() + (processes.isEmpty()?borderSize:c_processElemView::heightProcess + 2*interImageSpace)
                        + getImagesMaxHeigth(recipe::modes::resume) - ui->displayButton->height() - interImageSpace ;
                ui->displayButton->move(QPoint(posList[1].x() - (interImageSpace+ui->displayButton->width())/2,y));
                ui->displayButton->setText(QString("+%1").arg(countImages-maxNumberImages/2));
                if (animated) {
                    QPropertyAnimation* anim = recipe::fadeAnimation(ui->displayButton,true,1000);
                    if (anim != nullptr)
                        group->addAnimation(anim);
                } else {
                    ui->displayButton->show();
                }
            } else {
                if (animated) {
                    QPropertyAnimation* anim = recipe::fadeAnimation(ui->displayButton,false,1000);
                    if (anim != nullptr)
                        group->addAnimation(anim);
                } else {
                    ui->displayButton->hide();
                }
            }

            // Resume button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize + ui->label->height() - ui->resumeButton->height());
            targetSize = ui->menuButton->size();
            if (animated) {
                group->addAnimation(recipe::targetGeometryAnimation(ui->resumeButton,targetSize,targetPos,time));
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->resumeButton,false,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->resumeButton->move(targetPos);
                ui->resumeButton->setFixedSize(targetSize);
                ui->resumeButton->hide();
            }

            // Ingredients
            targetPos = QPoint(limit + interImageSpace,borderSize+std::max(ui->rankButton->height(),ui->label->height())
                               + (processes.isEmpty()?borderSize:c_processElemView::heightProcess + 2*interImageSpace));

            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(components,targetPos,time));
                QPropertyAnimation* anim = recipe::fadeAnimation(components,true,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                components->move(targetPos);
                components->show();
            }

            // Equipments
            targetPos = QPoint(limit + interImageSpace, borderSize+std::max(ui->rankButton->height(),ui->label->height())
                               + (processes.isEmpty()?borderSize:c_processElemView::heightProcess + 2*interImageSpace) + components->height() + interImageSpace);
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(equipments,targetPos,time));
                QPropertyAnimation* anim = recipe::fadeAnimation(equipments,true,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                equipments->move(targetPos);
                equipments->show();
            }

            // Show button
            targetSize = QSize(this->width()-2*borderSize,showButtonHeight);
            ui->showButton->setFixedSize(targetSize);
            ui->showButton->raise();

            // Menu button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->menuButton,targetPos,time));
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->menuButton,true,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->menuButton->move(targetPos);
            }

            // Note button
            targetPos = QPoint(this->width()- borderMenuButton- (ui->menuButton->width() + ui->noteButton->width())/2 , 2*borderSize + ui->menuButton->height());
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->noteButton,targetPos,time));
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
                group->addAnimation(recipe::targetPositionAnimation(ui->saveButton,QPoint(-ui->saveButton->width(),0),time));
                group->addAnimation(recipe::targetPositionAnimation(ui->cancelButton,QPoint(-ui->cancelButton->width(),0),time));
                group->addAnimation(recipe::targetPositionAnimation(ui->upButton,QPoint(this->width(),0),time));
                group->addAnimation(recipe::targetPositionAnimation(ui->downButton,QPoint(this->width(),0),time));
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
        case recipe::modes::display: {
            this->setFixedWidth(this->size().width());

            // label
            QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
            ui->label->setReadOnly(true);
            ui->label->setStyleSheet("QTextEdit {"
                                     "  border : 1px solid white;"
                                     "  background: transparent;"
                                     "}");
            targetSize = QSize(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton,getHeightText(recipe::modes::display));
            targetPos = QPoint(ui->rankButton->width()+borderSize*2,borderSize);
            if (animated) {
                group->addAnimation(recipe::targetGeometryAnimation(ui->label,targetSize,targetPos,time));
            } else {
                ui->label->setFixedSize(targetSize);
                ui->label->move(targetPos);
            }

            // rank button
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->rankButton,QPoint(borderSize,borderSize),time));
            } else {
                ui->rankButton->move(borderSize,borderSize);
            }

            // Process Views
            QList<QPoint> posProcesses = arrangeProcess(target);
            for (int i = 0; i < processes.size(); ++i) {
                processes[i]->show();
                processes[i]->switchMode(recipe::modes::resume);
                if (animated) {
                    group->addAnimation(recipe::targetPositionAnimation(processes[i],posProcesses[i],time));
                } else {
                    processes[i]->move(posProcesses[i]);
                }
            }

            // Images
            QList<QPoint> posList = arrangeImages(recipe::modes::display);
            for (int i = 0; i < images.size(); ++i) {
                QList<QPropertyAnimation*> anims = images[i]->switchMode(recipe::modes::display,animated);
                if (animated) {
                    group->addAnimation(recipe::targetPositionAnimation(images[i],posList[i],time));
                    for (int i = 0; i < anims.size(); ++i) {
                        group->addAnimation(anims[i]);
                    }
                } else {
                    images[i]->move(posList[i]);
                }
                if (state == recipe::states::retracted) {
                    images[i]->hide();
                } else {
                    images[i]->show();
                }
            }

            // Display button
            checkCount();
            if (animated) {
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->displayButton,false,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->displayButton->hide();
            }

            // Resume button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize + ui->label->height() - ui->resumeButton->height());
            targetSize = ui->menuButton->size();
            if (animated) {
                group->addAnimation(recipe::targetGeometryAnimation(ui->resumeButton,targetSize,targetPos,time));
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->resumeButton,true,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->resumeButton->move(targetPos);
                ui->resumeButton->setFixedSize(targetSize);
                ui->resumeButton->show();
            }

            // Ingredients
            if (animated) {
                QPropertyAnimation* anim = recipe::fadeAnimation(components,false,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                components->hide();
            }

            // Equipments
            if (animated) {
                QPropertyAnimation* anim = recipe::fadeAnimation(equipments,false,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                equipments->hide();
            }

            // Show button
            targetSize = QSize(this->width()-2*borderSize,showButtonHeight);
            ui->showButton->setFixedSize(targetSize);
            ui->showButton->raise();

            // Menu button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->menuButton,targetPos,time));
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->menuButton,true,time);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->menuButton->move(targetPos);
            }

            // Note button
            targetPos = QPoint(this->width()- borderMenuButton- (ui->menuButton->width() + ui->noteButton->width())/2 , 2*borderSize + ui->menuButton->height());
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->noteButton,targetPos,time));
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
                group->addAnimation(recipe::targetPositionAnimation(ui->saveButton,QPoint(-ui->saveButton->width(),0),time));
                group->addAnimation(recipe::targetPositionAnimation(ui->cancelButton,QPoint(-ui->cancelButton->width(),0),time));
                group->addAnimation(recipe::targetPositionAnimation(ui->upButton,QPoint(this->width(),0),time));
                group->addAnimation(recipe::targetPositionAnimation(ui->downButton,QPoint(this->width(),0),time));
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
        case recipe::modes::edition: {
            this->setFixedWidth(this->size().width());

            // label
            QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
            ui->label->setReadOnly(false);
            ui->label->setStyleSheet("");
            ui->label->raise();

            targetSize = QSize(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton,getHeightText(target));
            targetPos = QPoint(ui->rankButton->width()+borderSize*2,borderSize);
            if (animated) {
                group->addAnimation(recipe::targetGeometryAnimation(ui->label,targetSize,targetPos,time));
            } else {
                ui->label->setFixedSize(targetSize);
                ui->label->move(targetPos);
            }

            // rank button
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->rankButton,QPoint(ui->label->x(),borderSize),time));
            } else {
                ui->rankButton->move(ui->label->x(),borderSize);
            }

            // Process Views
            QList<QPoint> posProcesses = arrangeProcess(recipe::modes::edition);
            for (int i = 0; i < processes.size(); ++i) {
                processes[i]->show();
                processes[i]->switchMode(recipe::modes::edition);
                if (animated) {
                    group->addAnimation(recipe::targetPositionAnimation(processes[i],posProcesses[i],time));
                } else {
                    processes[i]->move(posProcesses[i]);
                }
            }

            // Images
            QList<QPoint> posList = arrangeImages(recipe::modes::edition);
            for (int i = 0; i < images.size(); ++i) {
                QList<QPropertyAnimation*> anims = images[i]->switchMode(recipe::modes::edition,animated);
                if (animated) {
                    group->addAnimation(recipe::targetPositionAnimation(images[i],posList[i],time));
                    for (int i = 0; i < anims.size(); ++i) {
                        group->addAnimation(anims[i]);
                    }
                } else {
                    images[i]->move(posList[i]);
                }
                if (state == recipe::states::retracted) {
                    images[i]->hide();
                } else {
                    images[i]->show();
                }
            }

            // Display button
            checkCount();
            if (animated) {
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->displayButton,false,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->displayButton->hide();
            }

            // Resume button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize + ui->label->height() - ui->resumeButton->height());
            targetSize = ui->menuButton->size();
            if (animated) {
                group->addAnimation(recipe::targetGeometryAnimation(ui->resumeButton,targetSize,targetPos,time));
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->resumeButton,false,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                ui->resumeButton->move(targetPos);
                ui->resumeButton->setFixedSize(targetSize);
                ui->resumeButton->hide();
            }

            // Ingredients
            if (animated) {
                group->addAnimation(recipe::fadeAnimation(components,false,1000));
            } else {
                components->hide();
            }

            // Equipments
            targetPos = QPoint(borderSize, borderSize + getHeightText(recipe::modes::edition) + interImageSpace);
            equipments->switchMode(recipe::modes::edition);
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(equipments,targetPos,time));
                QPropertyAnimation* anim = recipe::fadeAnimation(equipments,true,1000);
                if (anim != nullptr)
                    group->addAnimation(anim);
            } else {
                equipments->show();
                equipments->move(targetPos);
            }

            // Show button
            targetSize = QSize(this->width()-2*borderSize,showButtonHeight);
            ui->showButton->setFixedSize(targetSize);
            ui->showButton->raise();

            // Menu button
            targetPos = QPoint(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->menuButton,targetPos,time));
                group->addAnimation(recipe::fadeAnimation(ui->menuButton,false,time));
            } else {
                ui->menuButton->move(targetPos);
            }

            // Note button
            targetPos = QPoint(this->width()- borderMenuButton- (ui->menuButton->width() + ui->noteButton->width())/2 , 2*borderSize + ui->menuButton->height());
            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->noteButton,targetPos,time));
            } else {
                ui->noteButton->move(targetPos);
            }
            if (step->getNotes().isEmpty()) {
                ui->noteButton->hide();
            } else {
                ui->noteButton->show();
            }

            // Edition Buttons
            int interButton = (getHeightText(recipe::modes::edition) - ui->saveButton->height() - ui->cancelButton->height())/3;
            ui->saveButton->move(QPoint(-ui->saveButton->width(),borderSize+interButton));
            ui->cancelButton->move(QPoint(-ui->cancelButton->width(),borderSize+2*interButton + ui->saveButton->height()));

            interButton = (getHeightText(recipe::modes::edition) - ui->upButton->height() - ui->downButton->height())/3;
            ui->upButton->move(QPoint(this->width(),borderSize+interButton));
            ui->downButton->move(QPoint(this->width(),borderSize+2*interButton+ui->upButton->height()));

            if (animated) {
                group->addAnimation(recipe::targetPositionAnimation(ui->saveButton,QPoint(borderSize,borderSize+interButton),time));
                group->addAnimation(recipe::targetPositionAnimation(ui->cancelButton,QPoint(borderSize,borderSize+2*interButton + ui->saveButton->height()),time));
                group->addAnimation(recipe::targetPositionAnimation(ui->upButton,QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->upButton->width())/2 - ui->upButton->width(),borderSize+interButton),time));
                group->addAnimation(recipe::targetPositionAnimation(ui->downButton,QPoint(this->width() - (this->width() - ui->label->x() - ui->label->width() - ui->downButton->width())/2 - ui->downButton->width(),borderSize+2*interButton+ui->upButton->height()),time));
            } else  {
                ui->saveButton->move(QPoint(borderSize,borderSize+interButton));
                ui->cancelButton->move(QPoint(borderSize,borderSize+2*interButton + ui->saveButton->height()));
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

    QList<QPropertyAnimation *> anims = switchState(state,animated,time);
    for (int i = 0; i < anims.size(); ++i) {
        if (anims[i] != nullptr)
            group->addAnimation(anims[i]);
    }
    group->start(QAbstractAnimation::DeleteWhenStopped);
    mode = target;
}

QList<QPropertyAnimation *> c_stepView::switchState(int targetState, bool animated, int time) {
    QList<QPropertyAnimation *> res;
    QPropertyAnimation *anim = nullptr;
    switch (targetState) {
        case recipe::states::retracted: {
            for (int i = 0; i < images.size(); ++i) {
                if (!images[i]->isEmpty()) {
                    if (animated) {
                        anim = recipe::fadeAnimation(images[i],false,time);
                        if (anim != nullptr)
                            res.push_back(anim);
                        QObject::connect(anim,&QPropertyAnimation::finished, [this,i] () {images[i]->hide();});
                        images[i]->show();
                    } else {
                        images[i]->hide();
                    }
                }
            }
        }
        break;
        case recipe::states::opened: {
            for (int i = 0; i < images.size(); ++i) {
                if (!images[i]->isEmpty()) {
                    if (animated) {
                        anim = recipe::fadeAnimation(images[i],true,time);
                        if (anim != nullptr)
                            res.push_back(anim);
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
        res.push_back(recipe::targetPositionAnimation(ui->showButton,targetPos,time));
        res.push_back(recipe::targetSizeAnimation(this,targetSize,time));
    } else {
        ui->showButton->move(targetPos);
        this->setFixedSize(targetSize);
    }
    ui->showButton->raise();
    state = targetState;
    return res;
}

int c_stepView::getHeightWidget(int mode, int state) {
    switch (mode) {
        case recipe::modes::resume: {
            switch (state) {
                case recipe::states::retracted:
                    return std::max(ui->rankButton->height(),ui->label->height()) + borderSize + ui->showButton->height()
                            + (processes.isEmpty()?borderSize:c_processElemView::heightProcess + 2*interImageSpace);
                case recipe::states::opened:
                    return std::max(ui->rankButton->height(),ui->label->height()) + borderSize + ui->showButton->height()
                            + (processes.isEmpty()?borderSize:c_processElemView::heightProcess + 2*interImageSpace)
                            + std::max(getImagesMaxHeigth(mode),components->height() + equipments->getSize(mode).height() + 2*interImageSpace);
                default:
                    break;
            }
        }
        break;
        case recipe::modes::display: {
            switch (state) {
                case recipe::states::retracted:
                    return std::max(ui->rankButton->height(),ui->label->height()) + borderSize + ui->showButton->height()
                            + (processes.isEmpty()?borderSize:c_processElemView::heightProcess + 2*interImageSpace);
                case recipe::states::opened:
                    return std::max(ui->rankButton->height(),ui->label->height()) + borderSize + ui->showButton->height()
                            + (processes.isEmpty()?borderSize:c_processElemView::heightProcess + 2*interImageSpace)
                            + getImagesMaxHeigth(mode);
                default:
                    break;
            }
        }
        break;
        case recipe::modes::edition: {
            switch (state) {
                case recipe::states::retracted:
                    return getHeightText(recipe::modes::edition) + 2*borderSize + ui->showButton->height();
                            //+ (processes.isEmpty()?borderSize:c_processView::heightProcess + 2*interImageSpace);
                case recipe::states::opened:
                    qDebug() << getHeightText(recipe::modes::edition) << getImagesMaxHeigth(recipe::modes::edition);
                    return getHeightText(recipe::modes::edition) + 3*borderSize + ui->showButton->height()
//                            + (processes.isEmpty()?borderSize:c_processView::heightProcess + 2*interImageSpace)
                            + equipments->getSize(mode).height()
                            + getImagesMaxHeigth(recipe::modes::edition);
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

QList<QPoint> c_stepView::arrangeImages(int target, QPoint verticalShift) {
    QList<QPoint> res;
    int totalWidth = 0;
    int countImage;
    QPoint point;
    switch (target) {
        case recipe::modes::display:
                countImage = 0;
                for (int i = 0; i < images.size(); ++i) {
                    if (!images[i]->isEmpty()) {
                        totalWidth += images[i]->getSize(recipe::modes::display).width();
                        countImage++;
                    }
                }
                point = QPoint( (this->width() - 2*borderSize - totalWidth - ((countImage-1)*interImageSpace))/2 + borderSize,
                                borderSize*2 + std::max(ui->rankButton->height(),ui->label->height()) +
                                    (processes.isEmpty()?0:c_processElemView::heightProcess))
                        + verticalShift;
                for (int i = 0; i < images.size(); ++i) {
                    res.push_back(point);
                    point += QPoint(images[i]->getSize(recipe::modes::display).width() + interImageSpace,0);
                }
            break;
        case recipe::modes::resume:
                countImage = 0;
                for (int i = 0; i < images.size(); ++i) {
                    if (!images[i]->isEmpty() && countImage<maxNumberImages/2) {
                        totalWidth += images[i]->getSize(recipe::modes::resume).width();
                        countImage++;
                    }
                }
                point = QPoint( (limit - borderSize - totalWidth - ((countImage)*interImageSpace))/2 + borderSize,
                                borderSize*2 + std::max(ui->rankButton->height(),ui->label->height()) +
                                    (processes.isEmpty()?0:c_processElemView::heightProcess)) + verticalShift;
                for (int i = 0; i < maxNumberImages/2; ++i) {
                    res.push_back(point);
                    point += QPoint(images[i]->getSize(recipe::modes::resume).width() + interImageSpace,0);
                }
                for (int i = maxNumberImages/2; i < images.size(); ++i) {
                    res.push_back(QPoint(this->width(),0));
                }
            break;
        case recipe::modes::edition:
            for (int i = 0; i < images.size(); ++i) {
                totalWidth += images[i]->getSize(recipe::modes::edition).width();
            }
            point = QPoint((this->width() - 2*borderSize - totalWidth - ((maxNumberImages-1)*interImageSpace))/2 + borderSize,
                           borderSize + getHeightText(recipe::modes::edition) +2*interImageSpace + equipments->getSize(recipe::modes::edition).height());
            for (int i = 0; i < images.size(); ++i) {
                res.push_back(point);
                point += QPoint(images[i]->getSize(recipe::modes::edition).width() + interImageSpace,0);
            }
            break;
    default:
        break;
    }
    return res;
}

QList<QPoint> c_stepView::arrangeProcess(int target, QPoint verticalShift) {
    QList<QPoint> res;
    int totalWidth = 0;
    QPoint point;
    switch (target) {
        case recipe::modes::display:
        case recipe::modes::resume:
            for (int i = 0; i < processes.size(); ++i) {
                totalWidth += processes[i]->getSize(recipe::modes::display).width();
            }
            point = QPoint((this->width() - 2*borderSize - totalWidth - ((processes.size()-1)*interImageSpace))/2 + borderSize ,borderSize + interImageSpace + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
            for (int i = 0; i < processes.size(); ++i) {
                res.push_back(point);
                point += QPoint(processes[i]->getSize(recipe::modes::display).width() + interImageSpace,0);
            }
            break;
        case recipe::modes::edition:
            for (int i = 0; i < processes.size(); ++i) {
                totalWidth += processes[i]->getSize(recipe::modes::edition).width();
            }
            point = QPoint((this->width() - 2*borderSize - totalWidth - ((maxNumberImages-1)*interImageSpace))/2 + borderSize ,borderSize + interImageSpace + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
            for (int i = 0; i < processes.size(); ++i) {
                res.push_back(point);
                point += QPoint(processes[i]->getSize(recipe::modes::edition).width() + interImageSpace,0);
            }
            break;
    default:
        break;
    }
    return res;
}

int c_stepView::getImagesMaxHeigth(int mode) {
    int max = 0;
    switch (mode) {
        case recipe::modes::display:
            for (int i = 0; i < images.size(); ++i) {
                if (images[i]->height() > max && !images[i]->isEmpty()) {
                    max = images[i]->getSize(mode).height();
                }
            }
            break;
        case recipe::modes::resume:
            for (int i = 0; i < images.size(); ++i) {
                if (images[i]->height() > max && !images[i]->isEmpty()) {
                    max = images[i]->getSize(mode).height();
                }
            }
            if (countImages>maxNumberImages/2)
                max += ui->displayButton->height() + interImageSpace;
            max += interImageSpace;
            break;
        case recipe::modes::edition:
            for (int i = 0; i < images.size(); ++i) {
                if (images[i]->height() > max) {
                    max = images[i]->getSize(recipe::modes::edition).height();
                }
            }
            break;
    default:
        break;
    }
    return max;
}

bool c_stepView::hasImages() {
    for (int i = 0; i < images.size(); ++i) {
        if (!images[i]->isEmpty())
            return true;
    }
    return false;
}

int c_stepView::getLimit() const
{
    return limit;
}

c_step *c_stepView::getStep() const {
    return step;
}
