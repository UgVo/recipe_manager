#ifndef C_PROCESSVIEW_H
#define C_PROCESSVIEW_H

#include <QWidget>
#include <QStringListModel>
#include <QLineEdit>
#include <QPropertyAnimation>
#include "utils/c_process.h"
#include "utils/c_dbmanager.h"
#include <QFontMetrics>


namespace Ui {
class c_processView;
}

class c_processView : public QWidget
{
    Q_OBJECT

public:
    explicit c_processView(c_process* process, QWidget *parent = nullptr);
    ~c_processView();

    QList<QPropertyAnimation*> switchMode(int mode);
    QSize getSize(int mode);

    static int heightProcess;

private:
    Ui::c_processView *ui;
    c_process* process;
    int mode;
    QMap<int,QSize> sizes;
};

#endif // C_PROCESSVIEW_H
