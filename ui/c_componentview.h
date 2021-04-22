#ifndef C_COMPONENTVIEW_H
#define C_COMPONENTVIEW_H

#include <QWidget>
#include <ui/c_componentelemview.h>
#include <QFontMetrics>
#include <QCheckBox>
#include <QHBoxLayout>

namespace Ui {
class c_componentView;
}

class c_componentView : public QWidget
{
    Q_OBJECT

public:
    explicit c_componentView(QList<c_component*> components, QWidget *parent = nullptr);
    ~c_componentView();

private:
    Ui::c_componentView *ui;
    QList<c_component*> components;
    QList<c_componentElemView*> componentsViews;
};

#endif // C_COMPONENTVIEW_H
