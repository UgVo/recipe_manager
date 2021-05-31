#ifndef C_PROCESSELEMVIEW_H
#define C_PROCESSELEMVIEW_H

#include <QWidget>
#include <QStringListModel>
#include <QLineEdit>
#include <QPropertyAnimation>
#include "utils/c_process.h"
#include "utils/c_dbmanager.h"
#include <QFontMetrics>
#include <ui/c_widget.h>


namespace Ui {
class c_processElemView;
}

class c_processElemView : public c_widget
{
    Q_OBJECT

public:
    explicit c_processElemView(QString processType, c_process* process = nullptr, QWidget *parent = nullptr);
    ~c_processElemView() override;

    QAbstractAnimation *switchMode(modes target = modes::resume, bool animated = false, int time = 600) override;
    QSize getSize(modes target) const override;
    void save() override;
    void rollback() override;

    bool isEmpty() const;

    static int heightProcess;

    void setProcess(c_process *value);
    void updateView();

signals:
    void removeProcess(c_process *);
    void resized();

private:
    QString formatProcessText(modes target) const;

    Ui::c_processElemView *ui;
    c_process* process;
    QString processType;
};

#endif // C_PROCESSELEMVIEW_H
