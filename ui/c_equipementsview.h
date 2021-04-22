#ifndef C_EQUIPEMENTSVIEW_H
#define C_EQUIPEMENTSVIEW_H

#include <QWidget>
#include <QFontMetrics>

namespace Ui {
class c_equipementsView;
}

class c_equipementsView : public QWidget
{
    Q_OBJECT

public:
    explicit c_equipementsView(QList<QString> equipmentList, QWidget *parent = nullptr);
    ~c_equipementsView();

private:
    Ui::c_equipementsView *ui;
};

#endif // C_EQUIPEMENTSVIEW_H
