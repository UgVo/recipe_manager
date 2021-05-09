#ifndef C_IMAGE_H
#define C_IMAGE_H

#include <QWidget>
#include <ui/c_widget.h>
#include <QPropertyAnimation>
#include "utils/utils.h"

namespace Ui {
class c_image;
}

class c_image : public c_widget
{
    Q_OBJECT

public:

    explicit c_image(QString pathImage = QString(), QWidget *parent = nullptr);
    ~c_image();

    QPixmap getImage() const;
    QSize getSize(int target = modes::display) const;
    void setImage(const QPixmap &value);

    void save();
    void rollback();

    bool isEmpty() const;
    QAbstractAnimation *switchMode(int target = modes::resume, bool animated = true, int time = 1000);

    QString getPathImage() const;

public slots:
    void addButtonClicked();
    void deleteButtonClicked();

signals:
    void newImage(QPropertyAnimation* animation,QSize newSize);
    void resized();

private:

    Ui::c_image *ui;

    QMap<int,QSize> imageSizes;
    QMap<int,QSize> addButtonSizes;

    QString pathImage;
    QString pathOldImage;
    QPixmap image;
    QPixmap oldImage;

    QSize deleteButtonSize;
};

#endif // C_IMAGE_H
