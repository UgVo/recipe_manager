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

        QSize targetSize = getSize(recipe::modes::display);
        ui->addButton->setFixedSize(targetSize);
        this->setFixedSize(targetSize);
        ui->deleteButton->move(QPoint(targetSize.width()-ui->deleteButton->width(),0));
        deleteButtonSize = ui->deleteButton->size();

        if (!pathImage.isEmpty()) {
            image = QPixmap(pathImage).scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
        //computeSizes(width,count);
    }
}

bool c_image::isEmpty() const{
    return pathImage.isEmpty();
}

QList<QPropertyAnimation*> c_image::switchMode(int target, bool animated, int time) {
    QList<QPropertyAnimation*> res;
    switch (target) {
    case recipe::modes::display:
    case recipe::modes::resume: {
        ui->imageLabel->move(0,0);
        ui->addButton->move(0,0);
        QSize targetSize = getSize(target);
        if (isEmpty()) {
            if (animated) {
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->addButton,true,1000);
                if (anim != nullptr)
                    res.push_back(anim);
                anim = recipe::fadeAnimation(ui->imageLabel,false,1000);
                if (anim != nullptr)
                    res.push_back(anim);
                res.push_back(recipe::targetSizeAnimation(ui->addButton,targetSize,time));
            } else {
                ui->imageLabel->hide();
                ui->addButton->show();
                ui->addButton->setFixedSize(targetSize);
            }
        } else {
            if (animated) {
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->addButton,false,1000);
                if (anim != nullptr)
                    res.push_back(anim);
                anim = recipe::fadeAnimation(ui->imageLabel,true,1000);
                if (anim != nullptr)
                    res.push_back(anim);
                res.push_back(recipe::targetSizeAnimation(ui->imageLabel,targetSize,time));
            } else {
                ui->imageLabel->show();
                ui->addButton->hide();
                ui->addButton->setFixedSize(targetSize);
                ui->imageLabel->setFixedSize(targetSize);
            }
        }
        if (animated) {
            res.push_back(recipe::deflateAnimation(ui->deleteButton,time));
            res.push_back(recipe::targetSizeAnimation(this,targetSize,time));
        } else {
            ui->deleteButton->hide();
            this->setFixedSize(targetSize);
        }
    }

        break;
    case recipe::modes::edition: {
        ui->imageLabel->move(0,0);
        ui->addButton->move(0,0);
        ui->deleteButton->move(getSize(target).width()-deleteButtonSize.width(),0);
        QSize targetSize = getSize(target);
        if (isEmpty()) {
            if (animated) {
                res.push_back(recipe::targetSizeAnimation(ui->addButton,targetSize,time));
                res.push_back(recipe::deflateAnimation(ui->deleteButton,time));
            } else {
                ui->addButton->show();
                ui->addButton->setFixedSize(targetSize);
            }
            ui->addButton->show();
            ui->deleteButton->hide();
            ui->imageLabel->hide();
        } else {
            if (animated) {
                QPropertyAnimation* anim = recipe::fadeAnimation(ui->addButton,false,1000);
                if (anim != nullptr)
                    res.push_back(anim);
                anim = recipe::fadeAnimation(ui->imageLabel,true,1000);
                if (anim != nullptr)
                    res.push_back(anim);
                res.push_back(recipe::targetSizeAnimation(ui->imageLabel,targetSize,time));
                if (ui->deleteButton->isHidden() || ui->deleteButton->size() != deleteButtonSize) {
                    ui->deleteButton->show();
                    res.push_back(recipe::inflateAnimation(ui->deleteButton,deleteButtonSize,time));
                }
                ui->deleteButton->raise();
            } else {
                ui->imageLabel->show();
                ui->addButton->hide();
                ui->imageLabel->setFixedSize(targetSize);
            }
        }
        if (animated)
            res.push_back(recipe::targetSizeAnimation(this,targetSize,time));
        else
            this->setFixedSize(targetSize);
    }
        break;
    default:
        break;
    }

    return res;
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

            ui->imageLabel->move(0,0);
            ui->imageLabel->setFixedSize(endSize);

            state = recipe::states::fixed;
            emit newImage(new QPropertyAnimation(),QSize());
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

    emit resized();
}

QSize c_image::getSize(int mode) const {
    int width = 0;
    int count = 0;
    int availableWidth = 0;
    if (static_cast<c_stepView *>(parent())) {
        width = static_cast<c_stepView *>(parent())->width();
        count = static_cast<c_stepView *>(parent())->getImageCount();
    }
    switch (mode) {
    case recipe::modes::resume: {
        availableWidth =  (width - c_stepView::borderSize  - ((c_stepView::maxNumberImages/2))*c_stepView::interImageSpace)/(c_stepView::maxNumberImages/2);
        availableWidth = availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth;
        QSize resumeSize = QSize(availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth,c_stepView::maxSizeImage.height());
        if (isEmpty())
            return resumeSize;
        else
            return QPixmap(pathImage).scaled(resumeSize, Qt::KeepAspectRatio, Qt::SmoothTransformation).size();

    }
    case recipe::modes::display: {
        int availableWidth =  (width - 2*c_stepView::borderSize  - (count-1)*c_stepView::interImageSpace)/count;
        availableWidth = availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth;
        QSize displaySize = QSize(availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth,c_stepView::maxSizeImage.height());
        if (isEmpty())
            return displaySize;
        else
            return QPixmap(pathImage).scaled(displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation).size();
    }
    case recipe::modes::edition: {
        int availableWidth =  (width - 2*c_stepView::borderSize  - (c_stepView::maxNumberImages)*c_stepView::interImageSpace)/c_stepView::maxNumberImages;
        availableWidth = availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth;
        QSize displaySize = QSize(availableWidth>c_stepView::maxSizeImage.width()?c_stepView::maxSizeImage.width():availableWidth,c_stepView::maxSizeImage.height());
        if (isEmpty())
            return displaySize;
        else
            return QPixmap(pathImage).scaled(displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation).size();
    }
    default:
        break;
    }

    return isEmpty()?addButtonSizes[mode]:imageSizes[mode];
}
