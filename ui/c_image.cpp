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
}

c_image::~c_image() {
    delete ui;
}

QPixmap c_image::getImage() const {
    return image;
}

void c_image::setImage(const QPixmap &value) {
    oldImage = image;
    image = value;
}

QString c_image::save() {
    oldImage = QPixmap();
    return pathImage;
}

void c_image::rollback() {
    image = oldImage;
    pathImage = pathOldImage;
    pathOldImage = QString();
    oldImage = QPixmap();

    this->resize(this->size());
}

bool c_image::isEmpty() {
    return pathImage.isEmpty();
}

void c_image::switchMode(int newMode) {
    mode = newMode;
    this->resize(this->size());
}

void c_image::resizeEvent(QResizeEvent *) {
    if (state != recipe::states::transition) {
        int width, count, availableWidth;
//        if (static_cast<c_stepView*>(parent())) {
//            width = static_cast<c_stepView*>(parent())->width();
//            count = static_cast<c_stepView*>(parent())->countImage();
            width = 606;
            count = 2;

            ui->imageLabel->move(0,0);
            ui->addButton->move(0,0);

            switch (mode) {
                case recipe::modes::resume:
                    break;
                case recipe::modes::display:
                    availableWidth =  (width - 2*c_stepView::borderSize  - (count-1)*c_stepView::interImageSpace)/count;
                    availableWidth = availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth;
                    imagelSize = QSize(availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth,c_stepView::maxSizeImage.height());

                    ui->addButton->setFixedSize(imagelSize);
                    ui->deleteButton->move(QPoint(imagelSize.width()-ui->deleteButton->width(),0));

                    if (!pathImage.isEmpty()) {
                        image = QPixmap(pathImage).scaled(imagelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
                    availableWidth =  (width - 2*c_stepView::borderSize  - (c_stepView::maxNumberImages-1)*c_stepView::interImageSpace)/c_stepView::maxNumberImages;
                    availableWidth = availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth;
                    imagelSize = QSize(availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth,c_stepView::maxSizeImage.height());

                    ui->addButton->setFixedSize(imagelSize);
                    this->setFixedSize(imagelSize);
                    ui->deleteButton->move(QPoint(imagelSize.width()-ui->deleteButton->width(),0));

                    if (!pathImage.isEmpty()) {
                        image = QPixmap(pathImage).scaled(imagelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
//    }
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
            image = pixmap.scaled(imagelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
            //anim->start(QAbstractAnimation::DeleteWhenStopped);
            QObject::connect(anim,&QPropertyAnimation::finished,[=] () {
                state = recipe::states::fixed;
            });

            emit newImage(anim,image.size());
        }
    }
    ui->addButton->setEnabled(true);
}

void c_image::deleteButtonClicked() {
    pathOldImage = pathImage;
    pathImage = QString();
    oldImage = image;
    image = QPixmap();

    ui->imageLabel->hide();
    ui->deleteButton->hide();
    ui->addButton->show();

    this->setFixedSize(ui->addButton->size());
}
