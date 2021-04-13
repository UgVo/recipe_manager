#ifndef C_STEP_VIEW_H
#define C_STEP_VIEW_H

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <utils/c_step.h>

namespace Ui {
class c_stepView;
}

class c_stepView : public QWidget
{
    Q_OBJECT

public:
    explicit c_stepView(c_step *step, QWidget *parent = nullptr);
    ~c_stepView();

public slots:
    void slot_triggerShowImages();
    void resizeEvent(QResizeEvent *event);

    void openImageSlot(QRect sizeInit, QRect sizeEnd);
    void endOpen();
    void closeImageSlot();
    void endClose();

private:
    Ui::c_stepView *ui;
    c_step* step;
    QList<QLabel*> imageSlots;
    QList<QPixmap> imageList;
    QRect rectInit;
    QRect rectEnd;
    int hMax, wMax;
    bool showImage;
    int state;

    enum states{retracted,opening,opened,retracting};
    static int maxHeightImage;
    static int interimageSpace;
    static int borderSize;
};

#endif // C_STEP_VIEW_H
