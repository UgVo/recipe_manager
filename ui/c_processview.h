#ifndef C_PROCESSVIEW_H
#define C_PROCESSVIEW_H

#include <QWidget>
#include <QStringListModel>
#include "utils/c_process.h"
#include "utils/c_dbmanager.h"


namespace Ui {
class c_processView;
}

class c_processView : public QWidget
{
    Q_OBJECT

public:
    explicit c_processView(c_process* process, QWidget *parent = nullptr);
    ~c_processView();

private:
    Ui::c_processView *ui;
    c_process* process;
};

#endif // C_PROCESSVIEW_H
