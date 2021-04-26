#include "c_image.h"
#include "ui_c_image.h"
#include "c_stepview.h"

c_image::c_image(QString _pathImage, QWidget *_parent) :
    QWidget(_parent),
    ui(new Ui::c_image) {
    ui->setupUi(this);
    pathImage = _pathImage;
    ui->addButton->hide();
    ui->deleteButton->hide();

    mode = recipe::modes::display;
    state = recipe::states::fixed;

    QObject::connect(ui->addButton,&QPushButton::clicked,this,&c_image::addButtonClicked);
    QObject::connect(ui->deleteButton,&QPushButton::clicked,this,&c_image::deleteButtonClicked);

    if (static_cast<c_stepView*>(parent())) {
        int width = static_cast<c_stepView*>(parent())->width();
        int count = static_cast<c_stepView*>(parent())->countImage();

        computeSizes(width,count);

        ui->addButton->setFixedSize(imageSizes[recipe::modes::display] );
        this->setFixedSize(imageSizes[recipe::modes::display] );
        ui->deleteButton->move(QPoint(imageSizes[recipe::modes::display] .width()-ui->deleteButton->width(),0));

        if (!pathImage.isEmpty()) {
            image = QPixmap(pathImage).scaled(imageSizes[recipe::modes::display], Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->imageLabel->setFixedSize(image.size());
            ui->imageLabel->setPixmap(image);
            ui->imageLabel->setBackgroundRole(QPalette::Base);
            ui->imageLabel->setScaledContents(true);
            this->setFixedSize(image.size());

            ui->imageLabel->show();
            ui->addButton->hide();
            ui->deleteButton->hide();
        } else {
            ui->imageLabel->hide();
            ui->addButton->show();
            ui->deleteButton->hide();
        }
    }
}

c_image::~c_image() {
    delete ui;
}

QPixmap c_image::getImage() const {
    return image;
}

QSize c_image::getSize(int mode) const {
    return isEmpty()?addButtonSizes[mode]:imageSizes[mode];
}

void c_image::setImage(const QPixmap &value) {
    oldImage = image;
    image = value;
}

QString c_image::save() {
    oldImage = QPixmap();
    pathOldImage = QString();
    return pathImage;
}

void c_image::rollback() {
    image = oldImage;
    pathImage = pathOldImage;
    pathOldImage = QString();
    oldImage = QPixmap();

    if (image.isNull()) {
        ui->imageLabel->hide();
        ui->addButton->show();
        ui->deleteButton->hide();
    } else {
        ui->imageLabel->setPixmap(image);
        ui->imageLabel->setBackgroundRole(QPalette::Base);
        ui->imageLabel->setScaledContents(true);
        ui->imageLabel->show();
        ui->addButton->hide();
        ui->deleteButton->hide();
    }
    this->resize(this->size());
}

void c_image::updateSizes(int count) {
    if (static_cast<c_stepView*>(parent())) {
        int width = static_cast<c_stepView*>(parent())->getLimit();
        computeSizes(width,count);
    }
}

bool c_image::isEmpty() const{
    return pathImage.isEmpty();
}

QList<QPropertyAnimation*> c_image::switchMode(int newMode, bool animate, int time) {
    QList<QPropertyAnimation*> res;

    ui->imageLabel->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
    ui->addButton->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
    if (animate) {
        QPropertyAnimation* anim = new QPropertyAnimation(ui->imageLabel,"geometry");
        anim->setDuration(time);
        QRect rect = ui->imageLabel->rect();
        rect.setTopLeft(ui->imageLabel->pos());
        rect.setSize(ui->imageLabel->size());
        anim->setStartValue(rect);
        rect.setTopLeft(QPoint(0,0));
        rect.setSize(imageSizes[newMode]);
        anim->setEndValue(rect);
        anim->setEasingCurve(QEasingCurve::InOutQuart);
        res.append(anim);

        QPropertyAnimation* animAddButton = new QPropertyAnimation(ui->addButton,"geometry");
        animAddButton->setDuration(time);
        rect = ui->addButton->rect();
        rect.setTopLeft(ui->addButton->pos());
        rect.setSize(ui->addButton->size());
        animAddButton->setStartValue(rect);
        rect.setTopLeft(QPoint(0,0));
        rect.setSize(addButtonSizes[newMode]);
        animAddButton->setEndValue(rect);
        animAddButton->setEasingCurve(QEasingCurve::InOutQuart);
        res.append(animAddButton);

        QPropertyAnimation *animation = new QPropertyAnimation(this, "size");
        animation->setDuration(time);
        animation->setStartValue(this->size());
        animation->setEndValue(imageSizes[newMode]);
        animation->setEasingCurve(QEasingCurve::InOutQuart);
        res.append(animation);

        if (newMode == recipe::modes::edition) {
            oldImage = image;
            pathOldImage = pathImage;

            ui->deleteButton->show();
            ui->deleteButton->resize(21,21);
            QPoint startPoint = isEmpty() ? QPoint(addButtonSizes[newMode].width()-ui->deleteButton->width(),0):QPoint(imageSizes[newMode].width()-ui->deleteButton->width(),0);
            ui->deleteButton->move(startPoint);
            if (mode != newMode) {
                res.append(recipe::inflateAnimation(ui->deleteButton,ui->deleteButton->size()));
            }
        } else {
            res.append(recipe::deflateAnimation(ui->deleteButton,200));
        }
        if (isEmpty()) {
            ui->deleteButton->hide();
        }

        QObject::connect(anim,&QPropertyAnimation::finished,[=] () {
            state = recipe::states::fixed;
            ui->imageLabel->setFixedSize(ui->imageLabel->size());
            ui->addButton->setFixedSize(ui->addButton->size());
        });
        state = recipe::states::transition;
    } else {
        ui->imageLabel->setFixedSize(imageSizes[newMode]);
        ui->addButton->setFixedSize(addButtonSizes[newMode]);
        this->setFixedSize(imageSizes[newMode]);

        if (newMode == recipe::modes::edition) {
            oldImage = image;
            pathOldImage = pathImage;

            ui->deleteButton->show();
            ui->deleteButton->resize(21,21);
        } else {
            ui->deleteButton->hide();
        }
        if (isEmpty()) {
            ui->deleteButton->hide();
        }
        state = recipe::states::fixed;
    }


    mode = newMode;

    return res;
}

void c_image::resizeEvent(QResizeEvent *) {
    if (state != recipe::states::transition) {
        int width, count;
        if (static_cast<c_stepView*>(parent())) {
            width = static_cast<c_stepView*>(parent())->width();
            count = static_cast<c_stepView*>(parent())->countImage();

            computeSizes(width,count);

            ui->imageLabel->move(0,0);
            ui->addButton->move(0,0);

            switch (mode) {
                case recipe::modes::resume:
                    break;
                case recipe::modes::display:
                    ui->addButton->setFixedSize(addButtonSizes[recipe::modes::display]);
                    ui->deleteButton->move(QPoint(imageSizes[recipe::modes::display].width()-ui->deleteButton->width(),0));

                    if (!pathImage.isEmpty()) {
                        image = QPixmap(pathImage).scaled(imageSizes[recipe::modes::display], Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        ui->imageLabel->setFixedSize(image.size());
                        ui->imageLabel->setPixmap(image);
                        ui->imageLabel->setBackgroundRole(QPalette::Base);
                        ui->imageLabel->setScaledContents(true);
                        this->setFixedSize(image.size());

                        ui->imageLabel->show();
                        ui->addButton->hide();
                        ui->deleteButton->hide();

                    } else {
                        ui->imageLabel->hide();
                        ui->addButton->show();
                        ui->deleteButton->hide();
                    }
                    break;
                case recipe::modes::edition:
                    ui->addButton->setFixedSize(addButtonSizes[recipe::modes::edition]);
                    this->setFixedSize(imageSizes[recipe::modes::edition]);
                    ui->deleteButton->move(QPoint(imageSizes[recipe::modes::edition].width()-ui->deleteButton->width(),0));

                    if (!pathImage.isEmpty()) {
                        image = QPixmap(pathImage).scaled(imageSizes[recipe::modes::edition], Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        ui->imageLabel->setFixedSize(image.size());
                        ui->imageLabel->setPixmap(image);
                        ui->imageLabel->setBackgroundRole(QPalette::Base);
                        ui->imageLabel->setScaledContents(true);
                        this->setFixedSize(image.size());

                        ui->imageLabel->show();
                        ui->deleteButton->show();
                        ui->addButton->hide();

                    } else {
                        ui->imageLabel->hide();
                        ui->addButton->show();
                        ui->deleteButton->hide();
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

QString c_image::getPathImage() const {
    return pathImage;
}

void c_image::addButtonClicked() {
    ui->addButton->setEnabled(false);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"/home",tr("Images (*.png *.xpm *.jpg)"),nullptr,QFileDialog::DontUseNativeDialog);
    if (!fileName.isEmpty()) {
        QPixmap pixmap = QPixmap(fileName);
        if (!pixmap.isNull()) {
            image = pixmap.scaled(ui->addButton->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QSize endSize = image.size();
            ui->imageLabel->setFixedSize(QSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX));
            ui->imageLabel->setPixmap(image);
            ui->imageLabel->setBackgroundRole(QPalette::Base);
            ui->imageLabel->setScaledContents(true);
            this->setFixedSize(endSize);
            ui->imageLabel->show();
            ui->deleteButton->show();
            ui->addButton->hide();
            pathImage = fileName;

            ui->imageLabel->move(QPoint(endSize.width()/2,endSize.height()/2));

            QPropertyAnimation* anim = new QPropertyAnimation(ui->imageLabel,"geometry");
            anim->setDuration(1000);
            QRect rect = ui->imageLabel->rect();
            rect.setTopLeft(ui->imageLabel->pos());
            rect.setSize(QSize(0,0));
            anim->setStartValue(rect);
            rect.setTopLeft(QPoint(0,0));
            rect.setSize(endSize);
            anim->setEndValue(rect);
            anim->setEasingCurve(QEasingCurve::InOutQuart);

            state = recipe::states::transition;
            QObject::connect(anim,&QPropertyAnimation::finished,[=] () {
                state = recipe::states::fixed;
            });

            emit newImage(anim,image.size());
        }
    }
    ui->addButton->setEnabled(true);
}

void c_image::deleteButtonClicked() {
    if (pathOldImage.isEmpty()) {
        pathOldImage = pathImage;
        oldImage = image;
    }
    pathImage = QString();
    image = QPixmap();

    ui->imageLabel->hide();
    ui->deleteButton->hide();
    ui->addButton->show();

    this->setFixedSize(ui->addButton->size());
}

void c_image::computeSizes(int width, int count) {
    int availableWidth =  (width - 2*c_stepView::borderSize  - (count-1)*c_stepView::interImageSpace)/count;
    availableWidth = availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth;
    QSize displaySize = QSize(availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth,c_stepView::maxSizeImage.height());

    availableWidth =  (width - 2*c_stepView::borderSize  - (c_stepView::maxNumberImages-1)*c_stepView::interImageSpace)/c_stepView::maxNumberImages;
    availableWidth = availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth;
    QSize editSize = QSize(availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth,c_stepView::maxSizeImage.height());

    availableWidth =  (width - c_stepView::borderSize  - ((c_stepView::maxNumberImages/2))*c_stepView::interImageSpace)/(c_stepView::maxNumberImages/2);
    availableWidth = availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth;
    QSize resumeSize = QSize(availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth,c_stepView::maxSizeImage.height());

    if (!pathImage.isEmpty()) {
        imageSizes[recipe::modes::display] = QPixmap(pathImage).scaled(displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation).size();
        imageSizes[recipe::modes::edition] = QPixmap(pathImage).scaled(editSize, Qt::KeepAspectRatio, Qt::SmoothTransformation).size();
        imageSizes[recipe::modes::resume] = QPixmap(pathImage).scaled(resumeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation).size();
        addButtonSizes[recipe::modes::display] = displaySize;
        addButtonSizes[recipe::modes::edition] = editSize;
        addButtonSizes[recipe::modes::resume] = resumeSize;
    } else {
        imageSizes[recipe::modes::display] = displaySize;
        imageSizes[recipe::modes::edition] = editSize;
        imageSizes[recipe::modes::resume] = resumeSize;
        addButtonSizes[recipe::modes::display] = displaySize;
        addButtonSizes[recipe::modes::edition] = editSize;
        addButtonSizes[recipe::modes::resume] = resumeSize;
    }
}
