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
    explicit c_componentElemView(c_component *component, c_widget *_widget, QWidget *parent = nullptr);
    ~c_componentElemView() override;

    QAbstractAnimation *switchMode(modes mode = modes::resume, bool animated = true, int time = 600, QAnimationGroup *parentGroupAnimation = nullptr) override;
    QSize getSize(modes target) const override;
    void save() override;
    void rollback() override;

    c_component *getComponent();
    void updateComponent(c_component *newComponent);
    void setFocus();

signals:
    void deleteMe();

private:
    Ui::c_componentElemView *ui;
    c_component *component;

    static int heigthWidget;
};

#endif // C_COMPONENTELEMVIEW_H
