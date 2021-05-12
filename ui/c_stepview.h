#ifndef C_STEP_VIEW_H
#define C_STEP_VIEW_H

#include <QWidget>
#include <ui/c_widget.h>
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

class c_stepView : public c_widget
{
    Q_OBJECT

public:
    explicit c_stepView(c_step *step, QWidget *parent = nullptr);
    ~c_stepView();

    QAnimationGroup *switchMode(modes target = modes::resume, bool animated = true, int time = 600, QAbstractAnimation *childAnims = nullptr);
    QSize getSize(modes target = modes::none) const;

    void resizeEvent(QResizeEvent *event);

    int getImageCount();

    c_step *getStep() const;


    void checkCount();
    int getLimit() const;
    void updateLimit();

    static QSize maxSizeImage;
    static int interImageSpace;
    static int borderMenuButton;
    static int maxNumberImages;

public slots:
    void triggerShowButton();
    void editSaved();
    void editCanceled();
    void slotAddNote();
    void slotShowNotes();
    void imageAdded(QAbstractAnimation *animations);

signals:
    void upRank();
    void downRank();
    void saved(c_step* step);
    void toDelete(c_stepView *widget);
    void animationRequired(QAbstractAnimation* anim);

private:

    int getHeightText(modes targetMode = modes::resume) const;
    QList<QPoint> arrangeImages(modes target = modes::display);
    int getImagesMaxHeigth(modes target = modes::display) const;

    Ui::c_stepView *ui;
    c_step* step;
    QList<QString> imageList;
    QList<c_image*> images;
    c_componentView* components;
    c_equipementsView* equipments;
    c_processView* processes;
    c_notesDialog *noteDialog;

    int countImages;
    int limit;

    modes defaultMode;

};

#endif // C_STEP_VIEW_H
