#include "c_stepview.h"
#include "ui_c_step_view.h"

int c_stepView::maxHeightImage = 150;
int c_stepView::interimageSpace = 5;
int c_stepView::borderSize = 9;

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
        imageSlots.push_back(new QLabel(this));
    }
    state = states::retracted;
}

c_stepView::~c_stepView() {
    delete ui;
}

void c_stepView::slot_triggerShowImages() {
    showImage = !showImage;
    if (showImage) {
        state = states::opening;
        rectInit = this->rect();

        int heightEnd = borderSize + interimageSpace*2 + ui->horizontalLayout->geometry().size().height() + hMax  + ui->showButton->size().height();
        int totalWidth = 0;
        for (int i = 0; i < imageSlots.size(); ++i) {
            totalWidth += imageSlots[i]->size().width();
        }
        QPoint point((this->size().width() - borderSize - totalWidth - ((imageSlots.size()-1)*interimageSpace))/2 ,borderSize + ui->horizontalLayout->geometry().size().height());
        for (int i = 0; i < imageSlots.size(); ++i) {
            imageSlots[i]->move(point);
            imageSlots[i]->show();
            point += QPoint(imageSlots[i]->size().width() + interimageSpace,0);
        }
        ui->widgetButton->raise();
        rectEnd = rectInit;
        rectEnd.setSize(QSize(rectEnd.size().width(), heightEnd));

        openImageSlot(rectInit,rectEnd);
    } else {
        state = states::retracting;
        closeImageSlot();
    }
}

void c_stepView::resizeEvent(QResizeEvent */*event*/) {
    QPixmap image;
    switch (state) {
        case states::retracted : {
            this->setFixedWidth(this->size().width());
            this->adjustSize();
            ui->label->adjustSize();

            int maxW = this->size().width()/imageSlots.size() - (imageSlots.size()+1)*interimageSpace - 2*borderSize;
            for (int i = 0; i < imageSlots.size(); ++i) {
                image = imageList[i].scaledToWidth(maxW,Qt::SmoothTransformation);
                if (image.size().height() <= maxHeightImage) {
                    imageSlots[i]->setFixedHeight(image.size().height());
                    imageSlots[i]->setFixedWidth(image.size().width());
                    imageSlots[i]->setPixmap(image);
                    imageSlots[i]->setBackgroundRole(QPalette::Base);
                    imageSlots[i]->hide();
                } else {
                    image = imageList[i].scaledToHeight(maxHeightImage,Qt::SmoothTransformation);
                    imageSlots[i]->setFixedHeight(image.size().height());
                    imageSlots[i]->setFixedWidth(image.size().width());
                    imageSlots[i]->setPixmap(image);
                    imageSlots[i]->setBackgroundRole(QPalette::Base);
                    imageSlots[i]->hide();
                }
            }
            hMax = 0;
            wMax = 0;
            for (int i = 0; i < imageSlots.size() ; ++i ) {
                if (imageSlots[i]->size().width() > wMax)
                    wMax = imageSlots[i]->size().width();
                if (imageSlots[i]->size().height() > hMax)
                    hMax = imageSlots[i]->size().height();
            }
            break;
        }
        case states::opened : {
            //this->setFixedSize(this->size());
            break;
        }
        case states::opening:
        case states::retracting:
//            this->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
//            this->setFixedWidth(this->size().width());
            break;
    }

}

void c_stepView::openImageSlot(QRect sizeInit, QRect sizeEnd) {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "size");
    animation->setDuration(1000);
    animation->setStartValue(sizeInit.size());
    animation->setEndValue(sizeEnd.size());

    animation->setEasingCurve(QEasingCurve::InOutQuart);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);

    for (int i = 0; i < imageSlots.size(); ++i) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(imageSlots[i]);
        imageSlots[i]->setGraphicsEffect(effect);
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
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(imageSlots[i]);
        imageSlots[i]->setGraphicsEffect(effect);
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
    for (int i = 0; i < imageSlots.size(); ++i) {
        imageSlots[i]->hide();
    }
}
