#ifndef C_COMPONENTVIEW_H
#define C_COMPONENTVIEW_H

#include <QWidget>
#include <ui/c_widget.h>
#include <ui/c_componentelemview.h>
#include <QFontMetrics>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QPushButton>

namespace Ui {
class c_componentView;
}

class c_componentView : public c_widget
{
    Q_OBJECT

public:
    explicit c_componentView(QList<c_component*> components, c_widget *widget, QWidget *parent = nullptr);
    ~c_componentView() override;

    QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true, int time = 1000) override;
    QSize getSize(modes target = modes::resume) const override;
    int getWidth(modes target = modes::resume) const override;

    void save();
    void rollback();

    bool isEmpty() const;

public slots:
    void newComponent();
    void removeComponent();

signals:
    void resized();

private:
    Ui::c_componentView *ui;
    QList<c_componentElemView *> toDeleteComponents;
    QList<c_componentElemView *> addedComponents;
    QList<c_componentElemView *> componentsViews;
    QPushButton *addComponentButton;

    bool enableResize;
};

#endif // C_COMPONENTVIEW_H
