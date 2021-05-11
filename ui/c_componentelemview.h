#ifndef C_COMPONENTELEMVIEW_H
#define C_COMPONENTELEMVIEW_H

#include <QWidget>
#include <QObject>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QFontMetrics>
#include <utils/c_component.h>
#include <utils/c_ingredient.h>
#include <utils/utils.h>
#include <ui/c_widget.h>

namespace Ui {
class c_componentElemView;
}

class c_componentElemView : public c_widget
{
    Q_OBJECT

public:
    explicit c_componentElemView(c_component *component, QWidget *parent = nullptr);
    ~c_componentElemView();

    QAbstractAnimation *switchMode(modes mode = modes::resume, bool animated = true, int time = 1000);
    QSize getSize(modes target) const;
    void save();
    void rollback();

    c_component *getComponent();
    void setFocus();

signals:
    void deleteMe();

private:
    Ui::c_componentElemView *ui;
    c_component *component;

    static int heigthWidget;
};

#endif // C_COMPONENTELEMVIEW_H
