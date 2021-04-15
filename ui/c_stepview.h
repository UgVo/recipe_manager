#ifndef C_STEP_VIEW_H
#define C_STEP_VIEW_H

#include <QWidget>
#include <QLabel>
#include <QDebug>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QTextBlock>
#include <QPushButton>
#include <QMenu>
#include <QFileDialog>
#include <QElapsedTimer>
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

    void setRank(int rank);

    QList<QPropertyAnimation *> arrangeImagesEditOn(QPoint verticalShift);
    QList<QPropertyAnimation *> arrangeImagesEditOff(QPoint verticalShift);

public slots:
    void slot_triggerShowImages();
    void resizeEvent(QResizeEvent *event);

    void openImageSlot();
    void closeImageSlot();

    void editStepAnimationOn();
    void editStepAnimationOff();

    void endStepAnimationOff(int state);

    void editSaved();
    void editCanceled();

    void upEdit();
    void downEdit();

    void editAreaSizeChanged(int increment);

    void endTransition(int state);

    void handleAddimage();

signals:
    void new_rank(int newRank);
    void saved(c_step* step);

private:

    enum states{retracted,opened,transition};
    enum modes{edition,display};

    int getHeightText();
    void lockSize(bool flag);
    QPropertyAnimation *slideAnimation(QWidget *parent, QPoint slide, QSize growth = QSize());
    QPropertyAnimation *growAnimation(QWidget *parent, QSize growth);
    QPropertyAnimation *fadeAnimation(QWidget *parent, bool up);

    QList<QPoint> arrangeImages(bool mode = modes::display, QPoint verticalShift = QPoint());
    int getImagesMaxHeigth();

    Ui::c_stepView *ui;
    c_step* step;
    QList<QLabel*> imageSlots;
    QList<QPixmap> imageList;
    QList<QLabel*> newImageSlots;
    QList<QPixmap> newImageList;
    QList<QPoint> saveImageShift;
    QList<QSize> saveImageSize;
    QList<float> ratioImages;
    QList<QPushButton*> addImageButtons;
    int saveDeltaSizeimage;
    QRect rectInit;
    QRect rectEnd;
    bool showImage;
    int state;
    int rankEdit;
    bool imageNumberChanged;
    float ratio;

    static int maxHeightImage;
    static QSize maxSizeImage;
    static int interImageSpace;
    static int borderSize;
    static int showButtonHeight;
    static int borderMenuButton;
    static int maxNumberImages;

};

#endif // C_STEP_VIEW_H
