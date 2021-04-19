#include "c_stepview.h"
#include "ui_c_step_view.h"

QSize c_stepView::maxSizeImage(150,150);
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
    QObject::connect(ui->showButton,&QPushButton::released,this,&c_stepView::slot_triggerShowImages);

    ui->label->append(step->getDescription());
    ui->label->setAlignment(Qt::AlignJustify);
    QObject::connect(ui->label,&QTextEdit::textChanged,this,&c_stepView::slotTextModified);

    ui->rankButton->setText(QString("%1").arg(step->getRank()));
    rankEdit = step->getRank();

    ui->saveButton->setFixedWidth(ui->rankButton->width());
    ui->saveButton->setFixedSize(ui->saveButton->size());
    ui->saveButton->move(QPoint(-ui->saveButton->width(),0));
    ui->cancelButton->setFixedWidth(ui->rankButton->width());
    ui->cancelButton->setFixedSize(ui->cancelButton->size());
    ui->cancelButton->move(QPoint(-ui->cancelButton->width(),0));
    ui->upButton->setFixedSize(ui->upButton->size());
    ui->upButton->move(QPoint(this->width(),0));
    ui->downButton->setFixedSize(ui->downButton->size());
    ui->downButton->move(QPoint(this->width(),0));

    ui->newImageButton->hide();

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
    mode = recipe::modes::display;
}

c_stepView::~c_stepView() {
    delete ui;
    delete noteDialog;
}

void c_stepView::setRank(int rank) {
    rankEdit = rank;
    step->setRank(rank);
}

void c_stepView::slot_triggerShowImages() {
    lockSize(false);
    ui->showButton->raise();
    switch (state) {
        case recipe::states::retracted:
            state = recipe::states::transition;
            for (int i = 0; i < imageSlots.size(); ++i) {
                imageSlots[i]->show();
            }
            openImageSlot();
            break;
        case recipe::states::opened:
            state = recipe::states::transition;
            closeImageSlot();
            break;
        default:
            break;
    }
}

void c_stepView::resizeEvent(QResizeEvent *) {
    QPixmap image;
    switch (state) {
        case recipe::states::retracted : {
            this->setFixedWidth(this->size().width());

            // label
            QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
            ui->label->setFixedWidth(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton);
            ui->label->setFixedHeight(getHeightText());
            ui->label->move(ui->rankButton->width()+borderSize*2,borderSize);
            ui->label->setReadOnly(true);

            // rank button
            ui->rankButton->move(borderSize,borderSize);

            // show button
            ui->showButton->setFixedSize(this->width()-2*borderSize,showButtonHeight);
            ui->showButton->move(borderSize,borderSize*2+std::max(ui->rankButton->height(),ui->label->height()));

            // menu button
            ui->menuButton->move(this->width()-borderMenuButton-ui->menuButton->width(),borderSize);

            // note button
            ui->noteButton->move(this->width()- borderMenuButton- (ui->menuButton->width() + ui->noteButton->width())/2 , 2*borderSize + ui->menuButton->height() );
            if (step->getNotes().isEmpty()) {
                ui->noteButton->hide();
            } else {
                ui->noteButton->show();
            }

            int heightMin = borderSize*2 + std::max(ui->rankButton->height(),ui->label->height()) + ui->showButton->height();
            this->setFixedHeight(heightMin);

            QList<QPoint> posList = arrangeImages();
            for (int i = 0; i < images.size(); ++i) {
                images[i]->move(posList[i]);
                images[i]->hide();
            }
            ui->widgetButton->raise();
            break;
        }
        case recipe::states::opened : {
            lockSize(true);
            break;
        }
        case recipe::states::transition:

            break;
    }
}

void c_stepView::openImageSlot() {
    if (!images.isEmpty()) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        int hMax = getImagesMaxHeigth(mode);
        group->addAnimation(recipe::growAnimation(this,QSize(0,hMax+borderSize)));
        group->addAnimation(recipe::slideAnimation(ui->showButton,QPoint(0,hMax+borderSize)));

        for (int i = 0; i < images.size(); ++i) {
            if (!images[i]->isEmpty() && mode == recipe::modes::display) {
                group->addAnimation(recipe::fadeAnimation(images[i],true));
                images[i]->show();
            } else if (mode == recipe::modes::edition){
                group->addAnimation(recipe::fadeAnimation(images[i],true));
                images[i]->show();
            }
        }

        QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(recipe::states::opened);});
        group->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void c_stepView::closeImageSlot() {
    if (!images.isEmpty()) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        int hMax = getImagesMaxHeigth(mode);
        group->addAnimation(recipe::growAnimation(this,QSize(0,-(hMax+borderSize))));
        group->addAnimation(recipe::slideAnimation(ui->showButton,QPoint(0,-(hMax+borderSize))));

        for (int i = 0; i < images.size(); ++i) {
            group->addAnimation(recipe::fadeAnimation(images[i],false));
        }

        QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(recipe::states::retracted);});
        state = recipe::states::transition;
        group->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void c_stepView::editStepAnimationOn() {
    int slideDistance = 0;
    int finalLabelHeight = ui->label->height();
    int interButton = 0;
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    QRect rect;

    lockSize(false);

    ui->menuButton->setDisabled(true);

    ui->label->setReadOnly(false);
    ui->label->setStyleSheet("");
    ui->label->raise();

    QFontMetrics metrics(ui->label->document()->firstBlock().charFormat().font());
    ui->label->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
    ui->label->setFixedWidth(ui->label->width());

    ui->label->move(ui->rankButton->width()+borderSize*2,borderSize);

    if (metrics.height()*2+ui->label->height() > ui->rankButton->height()) {
        slideDistance = ui->rankButton->height() - metrics.height()*2;
        group->addAnimation(recipe::growAnimation(ui->label,QSize(0,slideDistance)));
        finalLabelHeight += metrics.height()*2;
    } else {
        group->addAnimation(recipe::growAnimation(ui->label,QSize(0,ui->rankButton->height() - ui->label->height())));
        finalLabelHeight += ui->rankButton->height() - ui->label->height();
    }

    QList<QPropertyAnimation*> res = arrangeImagesEditOn(QPoint(0,slideDistance));
    for (int i = 0; i < res.size(); ++i) {
        group->addAnimation(res[i]);
    }

    int slide = getImagesMaxHeigth(recipe::modes::edition) + finalLabelHeight + 3*borderSize - ui->showButton->pos().y();

    group->addAnimation(recipe::growAnimation(this,QSize(0,slide)));
    group->addAnimation(recipe::slideAnimation(ui->showButton,QPoint(0,slide)));

    for (int i = 0; i < images.size(); ++i) {
        images[i]->show();
        QList<QPropertyAnimation*> anims = images[i]->switchMode(recipe::modes::edition);
        for (QPropertyAnimation* anim: anims) {
            group->addAnimation(anim);
        }
        if (state == recipe::states::retracted || images[i]->isEmpty())
            group->addAnimation(recipe::fadeAnimation(images[i],true));
    }

    // Other buttons handeling
    interButton = (finalLabelHeight - ui->saveButton->height() - ui->cancelButton->height())/3;
    ui->saveButton->move(QPoint(-ui->saveButton->width(),borderSize+interButton));
    ui->cancelButton->move(QPoint(-ui->cancelButton->width(),borderSize+2*interButton + ui->saveButton->height()));
    interButton = (finalLabelHeight - ui->upButton->height() - ui->downButton->height())/3;
    ui->upButton->move(QPoint(this->width(),borderSize+interButton));
    ui->downButton->move(QPoint(this->width(),borderSize+2*interButton+ui->upButton->height()));

    group->addAnimation(recipe::slideAnimation(ui->rankButton,QPoint(ui->rankButton->width()+borderSize,0)));
    group->addAnimation(recipe::slideAnimation(ui->saveButton,QPoint(ui->saveButton->width()+borderSize,0)));
    group->addAnimation(recipe::slideAnimation(ui->cancelButton,QPoint(ui->cancelButton->width()+borderSize,0)));
    group->addAnimation(recipe::slideAnimation(ui->upButton,QPoint(-(ui->upButton->width()+borderMenuButton + (ui->menuButton->width()-ui->upButton->width())/2),0)));
    group->addAnimation(recipe::slideAnimation(ui->downButton,QPoint(-(ui->upButton->width()+borderMenuButton + (ui->menuButton->width()-ui->downButton->width())/2),0)));
    group->addAnimation(recipe::fadeAnimation(ui->menuButton,false));

    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {
        endTransition(recipe::states::opened);
    });
    state = recipe::states::transition;
    mode = recipe::modes::edition;
    group->start(QAbstractAnimation::DeleteWhenStopped);

    QObject::connect(ui->saveButton,&QPushButton::released,this,&c_stepView::editSaved);
    QObject::connect(ui->cancelButton,&QPushButton::released,this,&c_stepView::editCanceled);
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
    mode = recipe::modes::display;
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

int c_stepView::getHeightText() {
    QRect rect = ui->label->rect();
    int res = 0;
    for (int i = 0; i < ui->label->document()->blockCount(); ++i) {
        QFontMetrics metrics(ui->label->document()->findBlockByNumber(i).charFormat().font());
        res += metrics.boundingRect(rect,Qt::TextWordWrap,ui->label->document()->findBlockByNumber(i).text()).size().height();
    }
    // offset to compensate difference in height between text and QEditText
    return res + 8;
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

QList<QPoint> c_stepView::arrangeImages(int target, QPoint verticalShift) {
    QList<QPoint> res;
    int totalWidth = 0;
    QPoint point;

    switch (target) {
        case recipe::modes::display:
            for (int i = 0; i < images.size(); ++i) {
                if (!images[i]->isEmpty())
                    totalWidth += images[i]->getSize(recipe::modes::display).width();
            }
            point = QPoint((this->width() - 2*borderSize - totalWidth - ((images.size()-1)*interImageSpace))/2 + borderSize ,borderSize*2 + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
            for (int i = 0; i < images.size(); ++i) {
                res.push_back(point);
                point += QPoint(images[i]->getSize(recipe::modes::display).width() + interImageSpace,0);
            }
            break;
        case recipe::modes::edition:
            for (int i = 0; i < images.size(); ++i) {
                totalWidth += images[i]->getSize(recipe::modes::edition).width();
            }
            point = QPoint((this->width() - 2*borderSize - totalWidth - ((maxNumberImages-1)*interImageSpace))/2 + borderSize ,borderSize*2 + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
            for (int i = 0; i < images.size(); ++i) {
                res.push_back(point);
                point += QPoint(images[i]->getSize(recipe::modes::edition).width() + interImageSpace,0);
            }
            break;
    default:
        break;
    }


//        if (mode == modes::display) {
//            int widthTotalAvailableAfter = this->width() - 2*borderSize - (maxNumberImages-1)*interImageSpace;
//            widthTotalAvailableAfter = (widthTotalAvailableAfter > (maxSizeImage.width()*maxNumberImages) ? maxSizeImage.width()*(maxNumberImages) : widthTotalAvailableAfter);
//            ratio = float(widthTotalAvailableAfter)/float(totalWidth + ( maxSizeImage.width()* (maxNumberImages-imageSlots.size())));

//            QPoint point = QPoint(borderSize ,borderSize*2 + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
//            for (int i = 0; i < imageSlots.size(); ++i) {
//                res.push_back(point);
//                point += QPoint(int(float(imageSlots[i]->size().width())*ratio) + interImageSpace,0);
//            }

//            for (int i = imageSlots.size(); i < maxNumberImages; ++i) {
//                res.push_back(point);
//                point += QPoint(int(float(maxSizeImage.width())*ratio) + interImageSpace,0);
//            }
//        } else {
//            QPoint point = QPoint(borderSize ,borderSize*2 + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
//            for (int i = 0; i < imageSlots.size(); ++i) {
//                res.push_back(point);
//                point += QPoint(imageSlots[i]->size().width() + interImageSpace,0);
//            }

//            for (int i = imageSlots.size(); i < maxNumberImages; ++i) {
//                res.push_back(point);
//                point += QPoint(int(float(maxSizeImage.width())*ratio) + interImageSpace,0);
//            }
//        }

    return res;
}

int c_stepView::getImagesMaxHeigth(int mode) {
    int max = 0;
    switch (mode) {
        case recipe::modes::display:
            for (int i = 0; i < images.size(); ++i) {
                if (images[i]->height() > max && !images[i]->isEmpty()) {
                    max = images[i]->getSize(recipe::modes::display).height();
                }
            }
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

c_step *c_stepView::getStep() const {
    return step;
}
