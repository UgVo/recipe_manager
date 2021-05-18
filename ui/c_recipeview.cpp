#include "c_recipeview.h"
#include "ui_c_recipeview.h"

c_recipeView::c_recipeView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_recipeView)
{
    ui->setupUi(this);
}

c_recipeView::~c_recipeView()
{
    delete ui;
}
