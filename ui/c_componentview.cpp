#include "c_componentview.h"
#include "ui_c_componentview.h"
#include "c_stepview.h"

c_componentView::c_componentView(QList<c_component *> _components, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_componentView), components(_components) {
    ui->setupUi(this);

    int widthMin = 0;
    int heigthMin = 0;
    int max = parent->width()/2-c_stepView::borderSize - c_stepView::interImageSpace;
    QHBoxLayout* layout = static_cast<QHBoxLayout*>(ui->widget->layout());
    for (int i = 0; i < components.size(); ++i) {
        componentsViews.push_back(new c_componentElemView(components[i],this));
        layout->insertWidget(i+1,componentsViews.last());
        if (componentsViews.last()->width() > widthMin)
            widthMin = componentsViews.last()->width();
        heigthMin += componentsViews.last()->height();
    }
    QFontMetrics metric(ui->label_2->font());
    widthMin = std::max(widthMin,metric.horizontalAdvance(ui->label_2->text()));
    widthMin = widthMin > max ? max : widthMin;
    this->setFixedWidth(widthMin);
    this->setFixedHeight(heigthMin + components.size()*layout->spacing() + ui->label_2->height());

    ui->widget->setStyleSheet("QWidget#widget {"
                              " border : 1px solid white;"
                              " border-radius : 2px;"
                              "}");
}

c_componentView::~c_componentView() {
    delete ui;
}
