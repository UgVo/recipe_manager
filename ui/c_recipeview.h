#ifndef C_RECIPEVIEW_H
#define C_RECIPEVIEW_H

#include <QWidget>

namespace Ui {
class c_recipeView;
}

class c_recipeView : public QWidget
{
    Q_OBJECT

public:
    explicit c_recipeView(QWidget *parent = nullptr);
    ~c_recipeView();

private:
    Ui::c_recipeView *ui;
};

#endif // C_RECIPEVIEW_H
