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
    explicit c_stepView(c_step *step, c_widget *widget = nullptr, QWidget *parent = nullptr);
    ~c_stepView() override;

    QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true, int time = 600, QAnimationGroup *parentGroupAnimation = nullptr) override;
    QSize getSize(modes target = modes::none) const override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *e) override;
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
    void slotShowNotes();
    void imageAdded();

signals:
    void swapRank(recipe::swap direction);
    void saved();
    void toDelete();
    void animationRequired(QAbstractAnimation* anim);
    void resized();

private:
    int getHeightText(modes targetMode = modes::resume) const;
    int getImagesMaxHeigth(modes target = modes::display) const;

    QList<QPoint> arrangeImages(modes target = modes::display);

    Ui::c_stepView *ui;
    QMenu *menu;
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
    bool checked;
};

#endif // C_STEP_VIEW_H
