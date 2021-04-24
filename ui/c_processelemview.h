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
class c_processElemView;
}

class c_processElemView : public QWidget
{
    Q_OBJECT

public:
    explicit c_processElemView(c_process* process, QWidget *parent = nullptr);
    ~c_processElemView();

    QList<QPropertyAnimation*> switchMode(int mode);
    QSize getSize(int mode);

    static int heightProcess;

private:
    Ui::c_processElemView *ui;
    c_process* process;
    int mode;
    QMap<int,QSize> sizes;
};

#endif // C_PROCESSVIEW_H
