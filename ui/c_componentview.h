#ifndef C_COMPONENTVIEW_H
#define C_COMPONENTVIEW_H

#include <QWidget>
#include <ui/c_componentelemview.h>
#include <QFontMetrics>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QPushButton>

namespace Ui {
class c_componentView;
}

class c_componentView : public QWidget
{
    Q_OBJECT

public:
    explicit c_componentView(QList<c_component*> components, QWidget *parent = nullptr);
    ~c_componentView();

    QList<QPropertyAnimation *> switchMode(int target = recipe::modes::resume, bool animated = true, int time = 1000);
    QSize getSize(int mode = recipe::modes::resume);

public slots:
    void newComponent();
    void removeComponent();

signals:
    void resized();

private:
    Ui::c_componentView *ui;
    QList<c_component *> components;
    QList<c_componentElemView *> componentsViews;
    QPushButton *addComponentButton;
};

#endif // C_COMPONENTVIEW_H
