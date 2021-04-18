#ifndef C_IMAGE_H
#define C_IMAGE_H

#include <QWidget>
#include <QPropertyAnimation>
#include "utils/utils.h"

namespace Ui {
class c_image;
}

class c_image : public QWidget
{
    Q_OBJECT

public:

    explicit c_image(QString pathImage = QString(), QWidget *parent = nullptr);
    ~c_image();

    QPixmap getImage() const;
    void setImage(const QPixmap &value);

    QString save();
    void rollback();

    bool isEmpty();
    void switchMode(int mode = recipe::modes::resume);
    void resizeEvent(QResizeEvent *event);

    QString getPathImage() const;

public slots:
    void addButtonClicked();
    void deleteButtonClicked();

signals:
    void newImage(QPropertyAnimation* animation,QSize newSize);

private:
    Ui::c_image *ui;
    int mode;
    int state;

    QSize imagelSize;

    QString pathImage;
    QString pathOldImage;
    QPixmap image;
    QPixmap oldImage;
};

#endif // C_IMAGE_H
