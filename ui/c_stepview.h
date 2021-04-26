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
#include <ui/c_notesdialog.h>
#include <ui/c_image.h>
#include <ui/c_processelemview.h>
#include <ui/c_componentview.h>
#include <ui/c_equipementsview.h>
#include <ui/c_processview.h>

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

    void switchMode(int target = recipe::modes::resume, bool animated = true, int time = 1000);
    QList<QPropertyAnimation *> switchState(int targetState = recipe::states::retracted, bool animated = true, int time = 1000);

    int getHeightWidget(int mode, int state = recipe::states::retracted);

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

    int getLimit() const;

public slots:
    void triggerShowButton();

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

    int getHeightText(int targetMode = recipe::modes::resume);
    void lockSize(bool flag);

    QList<QPoint> arrangeImages(int target = recipe::modes::display, QPoint verticalShift = QPoint());
    int getImagesMaxHeigth(int mode = recipe::modes::display);

    bool hasImages();

    Ui::c_stepView *ui;
    c_step* step;
    QList<QString> imageList;
    QList<c_image*> images;
    c_componentView* components;
    c_equipementsView* equipments;
    c_processView* processes;

    c_notesDialog *noteDialog;

    int state;
    int mode;
    int rankEdit;
    int countImages;
    int limit;

};

#endif // C_STEP_VIEW_H
