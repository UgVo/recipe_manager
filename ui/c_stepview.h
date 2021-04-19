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
#include "c_notesdialog.h"
#include "c_image.h"

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

    QList<QPropertyAnimation *> arrangeImagesEditOn(QPoint verticalShift, int time = 1000);
    QList<QPropertyAnimation *> arrangeImagesEditOff(QPoint verticalShift);

    c_step *getStep() const;

    c_note* addNoteToStep(c_note *newNote);
    void deleteNote(c_note *note);

    void checkCount();

    static int maxHeightImage;
    static QSize maxSizeImage;
    static int interImageSpace;
    static int borderSize;
    static int showButtonHeight;
    static int borderMenuButton;
    static int maxNumberImages;

public slots:
    void slot_triggerShowImages();
    void resizeEvent(QResizeEvent *event);

    void openImageSlot();
    void closeImageSlot();

    void editStepAnimationOn();
    void editStepAnimationOff();

    void editSaved();
    void editCanceled();

    void upEdit();
    void downEdit();

    void slotTextModified();
    void editAreaSizeChanged(int increment);

    void endTransition(int state);

    void slotDelete();
    void slotAddNote();
    void slotShowNotes();

    int countImage();

    void imageAdded(QPropertyAnimation *animations);

signals:
    void new_rank(int newRank);
    void saved(c_step* step);
    void toDelete(c_stepView *widget);

private:

    int getHeightText();
    void lockSize(bool flag);

    QList<QPoint> arrangeImages(int target = recipe::modes::display, QPoint verticalShift = QPoint());
    int getImagesMaxHeigth(int mode = recipe::modes::display);

    bool hasImages();

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
    QList<c_image*> images;

    c_notesDialog *noteDialog;

    int state;
    int mode;
    int rankEdit;
    int countImages;

};

#endif // C_STEP_VIEW_H
