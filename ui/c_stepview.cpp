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

    for (QList<QString>::iterator it = imageStringList.begin(); it != imageStringList.end() ; ++it ) {
        imageList.push_back(*it);
        imageSlots.push_back(new QLabel(this));
    }

    for (int i = 0; i < maxNumberImages; ++i) {
        deleteButtons.push_back(new QPushButton("x",this));
        deleteButtons.last()->setFixedSize(20,20);
        deleteButtons.last()->hide();
        addImageButtons.push_back(new QPushButton("Add Image",this));
        addImageButtons.last()->hide();
        QObject::connect(addImageButtons[i],&QPushButton::released,this,&c_stepView::handleAddImage);
    }

    QMenu *menu = new QMenu();
    menu->addAction("Edit",this,&c_stepView::editStepAnimationOn);
    menu->addAction("Delete",this,&c_stepView::slotDelete);
    menu->addAction("Add note",this,&c_stepView::slotAddNote);

    ui->menuButton->setMenu(menu);
    ui->rankButton->move(borderSize,borderSize);

    ratio = 1.0f;

    noteDialog = new c_notesDialog(step->getNotesPtr(),this);
    noteDialog->hide();
    QObject::connect(ui->noteButton,&QPushButton::released,this,&c_stepView::slotShowNotes);

    state = states::retracted;
    mode = modes::display;
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
        case states::retracted:
            state = states::transition;
            for (int i = 0; i < imageSlots.size(); ++i) {
                imageSlots[i]->show();
            }
            openImageSlot();
            break;
        case states::opened:
            state = states::transition;
            closeImageSlot();
            break;
        default:
            break;
    }
}

void c_stepView::resizeEvent(QResizeEvent *) {
    QPixmap image;
    switch (state) {
        case states::retracted : {
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

            // format images
            if (!imageSlots.isEmpty()) {
                int maxW = (this->size().width() - (imageSlots.size()-1)*interImageSpace - 2*borderSize)/imageSlots.size();

                maxW = maxW>maxSizeImage.width()?maxSizeImage.width():maxW;
                QSize labelsize(maxW>maxSizeImage.width()?maxSizeImage.width():maxW,maxSizeImage.height());
                for (int i = 0; i < imageSlots.size(); ++i) {
                    image = QPixmap(imageList[i]).scaled(labelsize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    imageSlots[i]->setFixedHeight(image.size().height());
                    imageSlots[i]->setFixedWidth(image.size().width());
                    imageSlots[i]->setPixmap(image);
                    imageSlots[i]->setBackgroundRole(QPalette::Base);
                    imageSlots[i]->setScaledContents(true);
                    imageSlots[i]->hide();
                }
                QList<QPoint> posList = arrangeImages();
                for (int i = 0; i < imageSlots.size(); ++i) {
                    imageSlots[i]->move(posList[i]);
                }
            }
            ui->widgetButton->raise();
            break;
        }
        case states::opened : {
            lockSize(true);
            break;
        }
        case states::transition:

            break;
    }
}

void c_stepView::openImageSlot() {
    if (!imageSlots.isEmpty()) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        int hMax = getImagesMaxHeigth(false);
        group->addAnimation(growAnimation(this,QSize(0,hMax+borderSize)));
        group->addAnimation(slideAnimation(ui->showButton,QPoint(0,hMax+borderSize)));

        for (int i = 0; i < imageSlots.size(); ++i) {
            group->addAnimation(fadeAnimation(imageSlots[i],true));
        }

        QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::opened);});
        group->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void c_stepView::closeImageSlot() {
    if (!imageSlots.isEmpty()) {
        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        int hMax = getImagesMaxHeigth(false);
        group->addAnimation(growAnimation(this,QSize(0,-(hMax+borderSize))));
        group->addAnimation(slideAnimation(ui->showButton,QPoint(0,-(hMax+borderSize))));

        for (int i = 0; i < imageSlots.size(); ++i) {
            group->addAnimation(fadeAnimation(imageSlots[i],false));
        }

        QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::retracted);});
        state = states::transition;
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
        group->addAnimation(growAnimation(ui->label,QSize(0,slideDistance)));
        finalLabelHeight += metrics.height()*2;
    } else {
        group->addAnimation(growAnimation(ui->label,QSize(0,ui->rankButton->height() - ui->label->height())));
        finalLabelHeight += ui->rankButton->height() - ui->label->height();
    }


    QList<QPropertyAnimation*> res = arrangeImagesEditOn(QPoint(0,slideDistance));
    for (int i = 0; i < res.size(); ++i) {
        group->addAnimation(res[i]);
    }

    int slide = (int(float(getImagesMaxHeigth())*ratio) + finalLabelHeight + 3*borderSize) - ui->showButton->pos().y();

    group->addAnimation(growAnimation(this,QSize(0,slide)));
    group->addAnimation(slideAnimation(ui->showButton,QPoint(0,slide)));

    for (int i = 0; i < imageSlots.size(); ++i) {
        imageSlots[i]->show();
        if (state == states::retracted)
            group->addAnimation(fadeAnimation(imageSlots[i],true));
    }

    // Other buttons handeling
    interButton = (finalLabelHeight - ui->saveButton->height() - ui->cancelButton->height())/3;
    ui->saveButton->move(QPoint(-ui->saveButton->width(),borderSize+interButton));
    ui->cancelButton->move(QPoint(-ui->cancelButton->width(),borderSize+2*interButton + ui->saveButton->height()));
    interButton = (finalLabelHeight - ui->upButton->height() - ui->downButton->height())/3;
    ui->upButton->move(QPoint(this->width(),borderSize+interButton));
    ui->downButton->move(QPoint(this->width(),borderSize+2*interButton+ui->upButton->height()));

    group->addAnimation(slideAnimation(ui->rankButton,QPoint(ui->rankButton->width()+borderSize,0)));
    group->addAnimation(slideAnimation(ui->saveButton,QPoint(ui->saveButton->width()+borderSize,0)));
    group->addAnimation(slideAnimation(ui->cancelButton,QPoint(ui->cancelButton->width()+borderSize,0)));
    group->addAnimation(slideAnimation(ui->upButton,QPoint(-(ui->upButton->width()+borderMenuButton + (ui->menuButton->width()-ui->upButton->width())/2),0)));
    group->addAnimation(slideAnimation(ui->downButton,QPoint(-(ui->upButton->width()+borderMenuButton + (ui->menuButton->width()-ui->downButton->width())/2),0)));
    group->addAnimation(fadeAnimation(ui->menuButton,false));

    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::opened);});
    state = states::transition;
    mode = modes::edition;
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
        group->addAnimation(growAnimation(ui->label,QSize(0,futureHeightLabel - ui->label->height())));
    } else {
        if (futureHeightLabel > ui->rankButton->height()) {
            slideImages = futureHeightLabel - ui->label->height();
            maxHeightLabelRank = futureHeightLabel;
        } else {
            slideImages = ui->rankButton->height() - ui->label->height();
            maxHeightLabelRank = ui->rankButton->height();
        }
        if (!imageSlots.isEmpty()) {
            QList<QPropertyAnimation*> animImages = arrangeImagesEditOff(QPoint(0,slideImages));
            for (int i = 0; i < animImages.size(); ++i) {
                group->addAnimation(animImages[i]);
            }
        }
        group->addAnimation(growAnimation(ui->label,QSize(0,futureHeightLabel - ui->label->height())));

        int slide = (int(float(getImagesMaxHeigth(false))*ratio) + maxHeightLabelRank + 3*borderSize) - ui->showButton->pos().y();
        group->addAnimation(growAnimation(this,QSize(0,slide)));
        group->addAnimation(slideAnimation(ui->showButton,QPoint(0,slide)));
    }

    group->addAnimation(slideAnimation(ui->upButton,QPoint(ui->upButton->width()+borderMenuButton + (ui->menuButton->width()-ui->upButton->width())/2,0)));
    group->addAnimation(slideAnimation(ui->downButton,QPoint(ui->downButton->width()+borderMenuButton + (ui->menuButton->width()-ui->downButton->width())/2,0)));
    group->addAnimation(slideAnimation(ui->rankButton,QPoint(-(ui->rankButton->width()+borderSize),0)));
    group->addAnimation(slideAnimation(ui->saveButton,QPoint(-(ui->saveButton->width()+borderSize),0)));
    group->addAnimation(slideAnimation(ui->cancelButton,QPoint(-(ui->cancelButton->width()+borderSize),0)));
    group->addAnimation(fadeAnimation(ui->menuButton,true));

    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endStepAnimationOff(states::opened);});
    state = states::transition;
    mode = modes::display;
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::endStepAnimationOff(int _state) {
    ui->label->setReadOnly(true);
    ui->label->setStyleSheet("QTextEdit {"
                             "  border : 1px solid white;"
                             "  background: transparent;"
                             "}");
    ui->label->raise();
    ui->label->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
    ui->label->setFixedWidth(this->width() - ui->rankButton->width() - 2*borderSize - ui->menuButton->width()-2*borderMenuButton );

    ui->label->move(ui->rankButton->width()+borderSize*2,borderSize);
    this->endTransition(_state);
}

void c_stepView::editSaved() {
    enableDeleteButtons(false);

    imageList.append(newImageList);
    imageSlots.append(newImageSlots);

    step->setImagesUrl(imageList);

    newImageList.clear();
    newImageSlots.clear();

    for (int i = 0; i < oldImageSlots.size(); ++i) {
        oldImageSlots[i]->deleteLater();
    }
    oldImagesList.clear();
    oldImageSlots.clear();

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
    enableDeleteButtons(false);

    ui->label->clear();
    ui->label->append(step->getDescription());
    ui->label->setAlignment(Qt::AlignJustify);

    imageSlots.append(oldImageSlots);
    imageList.append(oldImagesList);
    for (int i = 0; i < imageSlots.size(); ++i) {
        imageSlots[i]->show();
    }

    oldImageSlots.clear();
    oldImagesList.clear();

    for (int i = 0; i < newImageSlots.size(); ++i) {
        newImageSlots[i]->deleteLater();
    }
    newImageSlots.clear();
    newImageList.clear();

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

    group->addAnimation(growAnimation(ui->label,QSize(0,increment),250));
    group->addAnimation(growAnimation(this,QSize(0,increment),250));
    group->addAnimation(slideAnimation(ui->showButton,QPoint(0,increment),QSize(),250));

    QList<QPropertyAnimation*> arrangedImages = arrangeImagesEditOn(QPoint(0,increment),true,250);
    for (int i = 0; i < arrangedImages.size(); ++i) {
        group->addAnimation(arrangedImages[i]);
    }

    QList<QPropertyAnimation*> deletebuttons = enableDeleteButtons(false);
    for (int i = 0; i < deletebuttons.size(); ++i) {
        group->addAnimation(deletebuttons[i]);
    }

    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::opened);});
    state = states::transition;
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::endTransition(int _state) {
    switch (_state) {
    case states::opened :
        if (mode == modes::edition) {
            enableDeleteButtons(true);
        } else {
            enableDeleteButtons(false);
        }
        state = states::opened;
        this->setFixedSize(this->size());
        break;
    case states::retracted :
        enableDeleteButtons(false);
        state = states::retracted;
        this->setFixedSize(this->size());
        for (int i = 0; i < imageSlots.size(); ++i) {
            imageSlots[i]->hide();
        }
        break;
    default:
        return ;
    }
}

void c_stepView::handleAddImage() {
    QPushButton* sender = static_cast<QPushButton*>(QObject::sender());
    sender->setEnabled(false);
    int index = 0;
    for (int i = 0; i < addImageButtons.size() ; ++i) {
        if (sender == addImageButtons[i])
            index = i;
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/home",tr("Images (*.png *.xpm *.jpg)"),nullptr,QFileDialog::DontUseNativeDialog);
    if (imageSlots.size() < maxNumberImages) {
        QPixmap pix = QPixmap(fileName);
        if (!pix.isNull()) {
            QParallelAnimationGroup *group = new QParallelAnimationGroup;

            newImageList.push_back(fileName);
            newImageSlots.push_back(new QLabel(this));

            int maxW = (this->size().width() - (newImageSlots.size()-1)*interImageSpace - 2*borderSize)/newImageSlots.size();

            maxW = maxW>maxSizeImage.width()?maxSizeImage.width():maxW;
            QSize labelsize(maxW>maxSizeImage.width()?maxSizeImage.width():maxW,maxSizeImage.height());

            pix = pix.scaled(labelsize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            if (int(float(labelsize.height())*ratio)>getImagesMaxHeigth()) {
                lockSize(false);
                group->addAnimation(slideAnimation(ui->showButton,QPoint(0,int(float(labelsize.height())*ratio)-getImagesMaxHeigth())));
                group->addAnimation(growAnimation(this,QSize(0,int(float(labelsize.height())*ratio)-getImagesMaxHeigth())));
            }

            newImageSlots.last()->setPixmap(pix);
            newImageSlots.last()->setBackgroundRole(QPalette::Base);
            newImageSlots.last()->setScaledContents(true);
            newImageSlots.last()->hide();

            newImageSlots.last()->move(addImageButtons[index]->pos());
            addImageButtons[index]->hide();
            newImageSlots.last()->show();

            QPropertyAnimation *animation = new QPropertyAnimation( newImageSlots.last(), "geometry");
            animation->setDuration(1000);
            QRect rect = newImageSlots.last()->rect();
            rect.setTopLeft(newImageSlots.last()->pos());
            rect.setSize(QSize(0,0));
            animation->setStartValue(rect);
            rect.setSize(QSize(int(float(pix.width())*ratio),int(float(pix.height())*ratio)));
            animation->setEndValue(rect);
            animation->setEasingCurve(QEasingCurve::InOutQuart);
            group->addAnimation(animation);

            QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::opened);});
            state = states::transition;

            group->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            sender->setEnabled(true);
        }
    }
}

void c_stepView::handleDeleteImage() {
    QPushButton* sender = static_cast<QPushButton*>(QObject::sender());
    sender->hide();
    int index = 0;
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    for (int i = 0; i < deleteButtons.size(); ++i) {
        if (sender == deleteButtons[i]) {
            index = i;
        }
    }
    if (index < imageSlots.size()) {
        oldImageSlots.push_back(imageSlots[index]);
        oldImagesList.push_back(imageList[index]);
        imageSlots[index]->hide();
        imageSlots.removeAt(index);
        imageList.removeAt(index);
        QList<QPropertyAnimation*> res = arrangeImagesEditOn(QPoint(),true);
        for (int i = 0; i < res.size(); ++i) {
            group->addAnimation(res[i]);
        }
    } else {
        int heigthRemoved = newImageSlots[index - imageSlots.size()]->height();

        newImageSlots[index - imageSlots.size()]->deleteLater();
        newImageSlots.removeAt(index - imageSlots.size());
        newImageList.removeAt(index - imageSlots.size());
        addImageButtons[index - imageSlots.size()]->show();

        QList<QPropertyAnimation*> res = arrangeImagesEditOn(QPoint(),true);
        for (int i = 0; i < res.size(); ++i) {
            group->addAnimation(res[i]);
        }

        int slide = getImagesMaxHeigth() - heigthRemoved;
        if (slide < 0) {
            lockSize(false);
            group->addAnimation(slideAnimation(ui->showButton,QPoint(0,slide)));
            group->addAnimation(growAnimation(this,QSize(0,slide)));
        }
    }
    QObject::connect(group,&QParallelAnimationGroup::finished,[=] () {endTransition(states::opened);});
    state = states::transition;
    enableDeleteButtons(false);
    group->start(QAbstractAnimation::DeleteWhenStopped);
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
}

void c_stepView::slotShowNotes() {
    noteDialog->exec();
}

c_note *c_stepView::addNoteToStep(c_note* newNote) {
    return step->addNote(*newNote);
}

void c_stepView::deleteNote(c_note *note) {
    step->deleteNote(*note);
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

QPropertyAnimation *c_stepView::slideAnimation(QWidget *parent, QPoint slide, QSize growth, int time) {
    QRect rect;
    QSize size;
    QPropertyAnimation *animation = new QPropertyAnimation(parent,"geometry");
    animation->setDuration(time);
    rect = parent->rect();
    size = parent->size();
    rect.setTopLeft(parent->pos());
    rect.setSize(size);
    animation->setStartValue(rect);
    rect.setTopLeft(parent->pos() + slide);
    rect.setSize(size + growth);
    animation->setEndValue(rect);
    animation->setEasingCurve(QEasingCurve::InOutQuart);

    return animation;
}

QPropertyAnimation *c_stepView::growAnimation(QWidget *parent, QSize growth, int time) {
    QPropertyAnimation *animation = new QPropertyAnimation(parent, "size");
    animation->setDuration(time);
    animation->setStartValue(parent->size());
    animation->setEndValue(parent->size() + growth);

    animation->setEasingCurve(QEasingCurve::InOutQuart);

    return animation;
}

QPropertyAnimation *c_stepView::fadeAnimation(QWidget *parent, bool up) {
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(parent);
    parent->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(1000);
    animation->setStartValue(up ? 0.0 : 1.0);
    animation->setEndValue(up ? 1.0 : 0.0);
    animation->setEasingCurve(QEasingCurve::InOutQuart);

    return animation;
}

QList<QPropertyAnimation *> c_stepView::arrangeImagesEditOn(QPoint verticalShift, bool update, int time) {
    QList<QPropertyAnimation*> res;
    QList<QPoint> newPos = arrangeImages(modes::edition,verticalShift);
    qDebug() << newPos;
    QPoint shift;
    QRect rect;
    QSize growth = QSize(0,0);
    QSize sizeButtonStart = QSize(0,0);
    QSize sizeButtonsEnd = QSize(0,0);
    for (int i = 0; i < maxNumberImages; ++i) {
         if (update) {
            sizeButtonStart = addImageButtons[i]->size();
            sizeButtonsEnd = addImageButtons[i]->size();
         } else {
            sizeButtonsEnd = QSize(int(float(maxSizeImage.width())*ratio),int(float(getImagesMaxHeigth())*ratio));
            addImageButtons[i]->move(newPos[i]);
         }
            addImageButtons[i]->lower();

            QPropertyAnimation *animation = new QPropertyAnimation( addImageButtons[i], "geometry");
            animation->setDuration(time);
            rect = addImageButtons[i]->rect();
            rect.setTopLeft(addImageButtons[i]->pos());
            rect.setSize(sizeButtonStart);
            animation->setStartValue(rect);
            rect.setTopLeft(newPos[i]);
            rect.setSize(sizeButtonsEnd);
            animation->setEndValue(rect);
            animation->setEasingCurve(QEasingCurve::InOutQuart);
            res.push_back(animation);

        if (i < imageSlots.size()) {
            if (!update) {
                imageSlots[i]->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
                growth = QSize(-int((1.0f-ratio)*float(imageSlots[i]->width())),-int((1.0f-ratio)*float(imageSlots[i]->height())));
            }
            shift = newPos[i] - imageSlots[i]->pos();
            res.push_back(slideAnimation(imageSlots[i],shift,growth,time));
            addImageButtons[i]->hide();
        } else if (i < imageSlots.size() + newImageSlots.size()) {
            shift = newPos[i] - newImageSlots[i-imageSlots.size()]->pos();
            res.push_back(slideAnimation(newImageSlots[i-imageSlots.size()],shift,growth,time));
            addImageButtons[i]->hide();
        } else {
            addImageButtons[i]->show();
            addImageButtons[i]->setEnabled(true);
        }
    }
    return res;
}

QList<QPropertyAnimation *> c_stepView::arrangeImagesEditOff(QPoint verticalShift) {
    QList<QPropertyAnimation*> res;
    QList<QPoint> newPos = arrangeImages(modes::display,verticalShift);
    QPoint shift;
    QRect rect;
    for (int i = 0; i < imageSlots.size(); ++i) {
        imageSlots[i]->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
        QSize growth(-int((1.0f-ratio)*float(imageSlots[i]->width())),-int((1.0f-ratio)*float(imageSlots[i]->height())));
        shift = newPos[i] - imageSlots[i]->pos();
        res.push_back(slideAnimation(imageSlots[i],shift,growth));
        imageSlots[i]->raise();
    }
    for (int i = 0; i < addImageButtons.size(); ++i) {
        QPropertyAnimation *animation = new QPropertyAnimation( addImageButtons[i], "geometry");
        animation->setDuration(1000);
        rect = addImageButtons[i]->rect();
        rect.setTopLeft(addImageButtons[i]->pos());
        rect.setSize(addImageButtons[i]->size());
        animation->setStartValue(rect);
        rect.setSize(QSize(0,0));
        animation->setEndValue(rect);
        animation->setEasingCurve(QEasingCurve::InOutQuart);
        res.push_back(animation);
    }
    return res;
}

QList<QPoint> c_stepView::arrangeImages(bool traget, QPoint verticalShift) {
    QList<QPoint> res;
    int totalWidth = 0;
    for (int i = 0; i < imageSlots.size(); ++i) {
        totalWidth += imageSlots[i]->width();
    }
    if (traget == modes::display) {
        int widthTotalAvailableAfter = this->width() - 2*borderSize - (imageSlots.size()-1)*interImageSpace;
        widthTotalAvailableAfter = (widthTotalAvailableAfter > (maxSizeImage.width()*imageSlots.size()) ? maxSizeImage.width()*(imageSlots.size()) : widthTotalAvailableAfter);
        ratio = float(widthTotalAvailableAfter)/float(totalWidth);

        QPoint point = QPoint((this->size().width() - 2*borderSize - widthTotalAvailableAfter - ((imageSlots.size()-1)*interImageSpace))/2 + borderSize ,borderSize*2 + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
        for (int i = 0; i < imageSlots.size(); ++i) {
            res.push_back(point);
            point += QPoint(int(float(imageSlots[i]->size().width())*ratio) + interImageSpace,0);
        }
    } else {
        if (mode == modes::display) {
            int widthTotalAvailableAfter = this->width() - 2*borderSize - (maxNumberImages-1)*interImageSpace;
            widthTotalAvailableAfter = (widthTotalAvailableAfter > (maxSizeImage.width()*maxNumberImages) ? maxSizeImage.width()*(maxNumberImages) : widthTotalAvailableAfter);
            ratio = float(widthTotalAvailableAfter)/float(totalWidth + ( maxSizeImage.width()* (maxNumberImages-imageSlots.size())));

            QPoint point = QPoint(borderSize ,borderSize*2 + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
            for (int i = 0; i < imageSlots.size(); ++i) {
                res.push_back(point);
                point += QPoint(int(float(imageSlots[i]->size().width())*ratio) + interImageSpace,0);
            }

            for (int i = imageSlots.size(); i < maxNumberImages; ++i) {
                res.push_back(point);
                point += QPoint(int(float(maxSizeImage.width())*ratio) + interImageSpace,0);
            }
        } else {
            QPoint point = QPoint(borderSize ,borderSize*2 + std::max(ui->rankButton->height(),ui->label->height())) + verticalShift;
            for (int i = 0; i < imageSlots.size(); ++i) {
                res.push_back(point);
                point += QPoint(imageSlots[i]->size().width() + interImageSpace,0);
            }

            for (int i = imageSlots.size(); i < maxNumberImages; ++i) {
                res.push_back(point);
                point += QPoint(int(float(maxSizeImage.width())*ratio) + interImageSpace,0);
            }
        }
    }
    return res;
}

QList<QPropertyAnimation *> c_stepView::enableDeleteButtons(bool flag) {
    QList<QPropertyAnimation *> res;
    if (flag) {
        for (int i = 0; i < deleteButtons.size(); ++i) {
            deleteButtons[i]->move(- deleteButtons[i]->width(),0);
            deleteButtons[i]->hide();
            QObject::disconnect(deleteButtons[i],&QPushButton::released,this,&c_stepView::handleDeleteImage);
        }
        for (int i = 0; i < imageSlots.size(); ++i) {
            deleteButtons[i]->move(imageSlots[i]->pos());
            deleteButtons[i]->show();
            deleteButtons[i]->raise();
            QObject::connect(deleteButtons[i],&QPushButton::released,this,&c_stepView::handleDeleteImage);
        }
        for (int i = 0; i < newImageSlots.size(); ++i) {
            qDebug() << "index" << i+imageSlots.size();
            deleteButtons[i+imageSlots.size()]->move(newImageSlots[i]->pos());
            deleteButtons[i+imageSlots.size()]->show();
            deleteButtons[i+imageSlots.size()]->raise();
            QObject::connect(deleteButtons[i+imageSlots.size()],&QPushButton::released,this,&c_stepView::handleDeleteImage);
        }
    } else {
        for (int i = 0; i < deleteButtons.size(); ++i) {
            deleteButtons[i]->move(- deleteButtons[i]->width(),0);
            deleteButtons[i]->hide();
            QObject::disconnect(deleteButtons[i],&QPushButton::released,this,&c_stepView::handleDeleteImage);
        }
    }
    return res;
}

int c_stepView::getImagesMaxHeigth(bool flag) {
    int max = 0;
    for (int i = 0; i < imageSlots.size(); ++i) {
        if (imageSlots[i]->height() > max)
            max = imageSlots[i]->height();
    }
    for (int i = 0; i < newImageSlots.size(); ++i) {
        if (newImageSlots[i]->height() > max)
            max = newImageSlots[i]->height();
    }
    if (flag) {
        for (int i = 0; i < addImageButtons.size(); ++i) {
            if (addImageButtons[i]->height() > max)
                max = addImageButtons[i]->height();
        }
    }
    return max;
}

c_step *c_stepView::getStep() const {
    return step;
}
