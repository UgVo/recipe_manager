#include "c_equipementsview.h"
#include "ui_c_equipementsview.h"
#include "c_stepview.h"

c_equipementsView::c_equipementsView(QList<QString> equipmentList, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_equipementsView) {
    ui->setupUi(this);
    ui->textEdit->setText(equipmentList.join(", "));

    int width = parent->width() - static_cast<c_stepView*>(parent)->getLimit() - c_stepView::borderSize - c_stepView::interImageSpace;
    this->setFixedWidth(width);

    QFontMetrics metrics =  QFontMetrics(ui->textEdit->document()->firstBlock().charFormat().font());
    ui->textEdit->setFixedHeight(metrics.boundingRect(ui->textEdit->rect(),Qt::TextWordWrap,equipmentList.join(",")).height());

    ui->widget->setStyleSheet("QWidget#widget {"
                              " border : 1px solid white;"
                              " border-radius : 2px;"
                              "}");
    ui->textEdit->setStyleSheet("QTextEdit {"
                                "  border : 1px solid white;"
                                "  background: transparent;"
                                "}");
    int top,bottom;
    ui->widget->layout()->getContentsMargins(nullptr,&top,nullptr,&bottom);
    this->setFixedHeight(ui->label->height() + ui->textEdit->height() + ui->widget->layout()->spacing() + top + bottom);
}

c_equipementsView::~c_equipementsView()
{
    delete ui;
}
