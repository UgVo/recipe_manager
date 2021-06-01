#ifndef C_EQUIPEMENTSVIEW_H
#define C_EQUIPEMENTSVIEW_H

#include <QWidget>
#include <ui/c_widget.h>
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

class c_equipementsView : public c_directedWidget
{
    Q_OBJECT

public:
    explicit c_equipementsView(QList<QString> equipmentList, c_widget *widget, QWidget *parent = nullptr);
    ~c_equipementsView() override;

    QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = true, int time = 600, QAnimationGroup *parentGroupAnimation = nullptr) override;
    QSize getSize(modes mode) override;

    void save() override;
    void rollback() override;

    bool isEmpty() const;

    bool eventFilter(QObject* obj, QEvent* event) override;

    void setEquipmentList(const QSet<QString> equipments);
    void hideTitle(bool hide);

public slots:
    void addEquipment(QString newEquipment);
    void removeEquipment();

signals:
    void newEquipmentList(QList<QString> newEquipments);

private:
    static int numberMaxEquipement;

    Ui::c_equipementsView *ui;
    QList<QString> equipmentList;
    QList<QString> addedEquipment;
    QList<QString> toDeleteEquipment;
    QStringListModel *model;
    QList<QString> equipementsListModel;
    QMap<QString,QLabel *> equipmentLabelMap;
    QMap<QString,QPushButton *> equipmentButtonMap;
    QTimer timer;

    bool _hideTitle;
};

#endif // C_EQUIPEMENTSVIEW_H
