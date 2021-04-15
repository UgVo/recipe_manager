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

    QList<QPropertyAnimation *> arrangeImagesEditOn(QPoint verticalShift, bool update = false, int time = 1000);
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

    void slotTextModified();
    void editAreaSizeChanged(int increment);

    void endTransition(int state);

    void handleAddImage();
    void handleDeleteImage();

    void slotAddNote();
    void slotShowNotes();

signals:
    void new_rank(int newRank);
    void saved(c_step* step);

private:

    enum states{retracted,opened,transition};
    enum modes{edition,display};

    int getHeightText();
    void lockSize(bool flag);
    QPropertyAnimation *slideAnimation(QWidget *parent, QPoint slide, QSize growth = QSize(), int time = 1000);
    QPropertyAnimation *growAnimation(QWidget *parent, QSize growth, int time = 1000);
    QPropertyAnimation *fadeAnimation(QWidget *parent, bool up);

    QList<QPoint> arrangeImages(bool mode = modes::display, QPoint verticalShift = QPoint());
    QList<QPropertyAnimation*> enableDeleteButtons(bool flag);
    int getImagesMaxHeigth(bool flag = true);

    Ui::c_stepView *ui;
    c_step* step;
    QList<QLabel*> imageSlots;
    QList<QString> imageList;
    QList<QLabel*> newImageSlots;
    QList<QString> newImageList;
    QList<QLabel*> oldImageSlots;
    QList<QString> oldImagesList;
    QList<QPushButton*> addImageButtons;
    QList<QPushButton*> deleteButtons;

    int saveDeltaSizeimage;
    int state;
    int mode;
    int rankEdit;
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
