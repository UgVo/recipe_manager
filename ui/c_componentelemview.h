#ifndef C_COMPONENTELEMVIEW_H
#define C_COMPONENTELEMVIEW_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QFontMetrics>
#include <utils/c_component.h>
#include <utils/c_ingredient.h>
#include <utils/utils.h>

namespace Ui {
class c_componentElemView;
}

class c_componentElemView : public QWidget
{
    Q_OBJECT

public:
    explicit c_componentElemView(c_component *component, QWidget *parent = nullptr);
    ~c_componentElemView();

    QList<QPropertyAnimation *> switchMode(int mode = recipe::modes::resume);
    void save();
    void rollback();

    c_component *getComponent();
    void setFocus();

signals:
    void deleteMe();

private:
    Ui::c_componentElemView *ui;
    c_component *component;
};

#endif // C_COMPONENTELEMVIEW_H
