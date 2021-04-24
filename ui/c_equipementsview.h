#ifndef C_EQUIPEMENTSVIEW_H
#define C_EQUIPEMENTSVIEW_H

#include <QWidget>
#include <QFontMetrics>
#include <QPushButton>
#include <QCompleter>
#include <QTimer>
#include <QStringListModel>
#include <QPropertyAnimation>
#include <utils/utils.h>

namespace Ui {
class c_equipementsView;
}

class c_equipementsView : public QWidget
{
    Q_OBJECT

public:
    explicit c_equipementsView(QList<QString> equipmentList, QWidget *parent = nullptr);
    ~c_equipementsView();

    QList<QPropertyAnimation *> switchMode(int targetMode = recipe::modes::resume, bool animated = true, int time = 1000);
    QSize getSize(int mode);

    bool eventFilter(QObject* obj, QEvent* event);

public slots:
    void addEquipment(QString newEquipment);
    void removeEquipment();

signals:
    void newEquipmentList(QList<QString> newEquipments);

private:
    static int numberMaxEquipement;

    Ui::c_equipementsView *ui;
    QList<QString> equipmentList;
    QList<QPushButton *> buttonList;
    QStringListModel *model;
    QList<QString> allEquipementsList;
    QTimer timer;

    bool write;
};

#endif // C_EQUIPEMENTSVIEW_H
