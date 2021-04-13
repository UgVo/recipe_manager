#include "c_stepview.h"
#include "ui_c_step_view.h"

c_stepView::c_stepView(c_step *_step, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_stepView), step(_step) {
    ui->setupUi(this);

    QRect rect(2,2,46,46);
    QRegion region(rect, QRegion::Ellipse);
    ui->rankButton->setMask(region);
    QObject::connect(ui->showButton,&QPushButton::released,this,&c_stepView::slot_triggerShowImages);
    showImage = false;

    ui->label->setText(step->getDescription());

    QList<QString> imageStringList = step->getImagesUrl();
    for (QList<QString>::iterator it = imageStringList.begin(); it != imageStringList.end() ; ++it ) {
        imageList.push_back(QPixmap(*it));
        imageSlots.push_back(new QLabel());
        imageSlotsCopy.push_back(new QLabel(this));
        dynamic_cast<QHBoxLayout*>(ui->imageSlot->layout())->insertWidget(1,imageSlots.last());
    }

    ui->imageSlot->hide();

    state = states::retracted;
    size_retracted = QSize(0,0);
}

c_stepView::~c_stepView() {
    delete ui;
}

void c_stepView::slot_triggerShowImages() {
    showImage = !showImage;
    if (showImage) {
        state = states::opened;
        rectInit = this->rect();
        ui->imageSlot->show();
        QPoint point;
        for (int i = 0; i < imageSlots.size(); ++i) {
            point = ui->imageSlot->mapToParent(imageSlots[i]->pos());
            imageSlotsCopy[i]->move(point+QPoint(0,15));
            imageSlotsCopy[i]->show();
        }
        ui->widgetButton->raise();
        rectEnd = this->rect();
        ui->imageSlot->hide();
        this->adjustSize();

        openImageSlot(rectInit,rectEnd);
    } else {
        closeImageSlot();
    }
}

void c_stepView::resizeEvent(QResizeEvent */*event*/) {
    switch (state) {
        case states::retracted :
            size_retracted = this->size();
            this->setFixedWidth(size_retracted.width());
            this->adjustSize();
            ui->imageSlot->size();
            int maxW = this->size().width()/imageSlots.size() - imageSlots.size()*5 - 18;
            int maxH = 150;
            QPixmap image;
            for (int i = 0; i < imageSlots.size(); ++i) {
                image = imageList[i].scaledToWidth(maxW,Qt::SmoothTransformation);
                if (image.size().height() <= maxH) {
                    imageSlots[i]->setFixedHeight(image.size().height());
                    imageSlots[i]->setFixedWidth(image.size().width());
                    imageSlots[i]->setPixmap(image);
                    imageSlots[i]->setBackgroundRole(QPalette::Base);
                    imageSlotsCopy[i]->setFixedHeight(image.size().height());
                    imageSlotsCopy[i]->setFixedWidth(image.size().width());
                    imageSlotsCopy[i]->setPixmap(image);
                    imageSlotsCopy[i]->setBackgroundRole(QPalette::Base);
                    imageSlotsCopy[i]->hide();
                } else {
                    image = imageList[i].scaledToHeight(maxH,Qt::SmoothTransformation);
                    imageSlots[i]->setFixedHeight(image.size().height());
                    imageSlots[i]->setFixedWidth(image.size().width());
                    imageSlots[i]->setPixmap(image);
                    imageSlots[i]->setBackgroundRole(QPalette::Base);
                    imageSlotsCopy[i]->setFixedHeight(image.size().height());
                    imageSlotsCopy[i]->setFixedWidth(image.size().width());
                    imageSlotsCopy[i]->setPixmap(image);
                    imageSlotsCopy[i]->setBackgroundRole(QPalette::Base);
                    imageSlotsCopy[i]->hide();
                }
            }
        break;
    }

}

void c_stepView::openImageSlot(QRect sizeInit, QRect sizeEnd) {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "size");
    animation->setDuration(1000);
    animation->setStartValue(sizeInit.size());
    animation->setEndValue(sizeEnd.size()+QSize(0,15));

    animation->setEasingCurve(QEasingCurve::InOutQuart);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);

    for (int i = 0; i < imageSlots.size(); ++i) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(imageSlotsCopy[i]);
        imageSlotsCopy[i]->setGraphicsEffect(effect);
        QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
        animation->setDuration(1000);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::InOutQuart);
        group->addAnimation(animation);
    }

    QObject::connect(group,&QParallelAnimationGroup::finished,this,&c_stepView::endOpen);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::endOpen() {
    state = states::opened;
}

void c_stepView::closeImageSlot() {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "size");
    animation->setDuration(1000);
    animation->setStartValue(this->size());
    animation->setEndValue(rectInit.size());

    animation->setEasingCurve(QEasingCurve::InOutQuart);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);


    for (int i = 0; i < imageSlots.size(); ++i) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(imageSlotsCopy[i]);
        imageSlotsCopy[i]->setGraphicsEffect(effect);
        QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
        animation->setDuration(1000);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);
        animation->setEasingCurve(QEasingCurve::InOutQuart);
        group->addAnimation(animation);
    }

    QObject::connect(group,&QParallelAnimationGroup::finished,this,&c_stepView::endClose);
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_stepView::endClose() {
    state = states::retracted;
}
