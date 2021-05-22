#ifndef C_WIDGET_H
#define C_WIDGET_H

#include <QDebug>
#include <QSize>
#include <QWidget>
#include <QAnimationGroup>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QSequentialAnimationGroup>
#include <QLabel>
#include <QFontMetrics>
#include <QGraphicsPixmapItem>
#include <QMenu>
#include <utils/utils.h>

class c_pixmapGraphics : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY (qreal rotation READ rotation WRITE setRotation)
};

class c_widget : public QWidget
{
public:
    enum modes{resume,display,edition,minimal,setup,none};
    static QMap<modes,QString> mapModeToString;

    c_widget(QWidget *parent = nullptr, c_widget *m_parent = nullptr);

    virtual ~c_widget() override;

    virtual QSize getSize(modes target) const;
    virtual int getWidth(modes target) const;
    virtual QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true,int time = 500);
    virtual QAbstractAnimation *switchMode(modes target = modes::resume, bool = true, int time = 500, QAnimationGroup *parentGroupAnimation = nullptr);

    virtual QAbstractAnimation *runBehavior(bool animated,QAnimationGroup *group, QAnimationGroup *parentGroupAnimation);
    virtual void handleChildrenAnimation(QAbstractAnimation *animation);

    virtual void save();
    virtual void rollback();

    virtual void resizeEvent(QResizeEvent *event) override;

    modes getMode() const;
    void setMode(modes value);

protected:
    QAbstractAnimation *fadeAnimation(QWidget *parent, bool up, int time = 1000, int delay = 0);
    QPropertyAnimation *targetGeometryAnimation(QWidget* parent, QSize targetSize, QPoint targetPos, int time = 1000);
    QPropertyAnimation *targetPositionAnimation(QWidget* parent, QPoint targetPos, int time = 1000, int delay = 0);
    QPropertyAnimation *targetSizeAnimation(QWidget* parent, QSize targetSize, int time = 1000);
    QAnimationGroup *slideAndDeployAnimation(QWidget* parent, QPoint targetPos, int time = 1000,
                                             std::function<void()> lambda = nullptr, modes mode = modes::resume);
    QPropertyAnimation *deflateAnimation(QWidget *parent, int time);
    QPropertyAnimation *inflateAnimation(QWidget *parent, QSize endSize, int time);
    QPropertyAnimation *rotateAnimation(c_pixmapGraphics *parent, qreal angle, int time);
    QPropertyAnimation *openMenuAnimation(QMenu *menu, int time);
    QPropertyAnimation *closeMenuAnimation(QMenu *menu, int time);

    int getHorizontalAdvanceLabel(QLabel *label) const;

    modes mode;
    c_widget* m_parent;

    static int insideBorder;
    static int labelHeight;
    static int stepWidth;
    static int buttonHeight;
    static int borderSize;
};

#endif // C_WIDGET_H
