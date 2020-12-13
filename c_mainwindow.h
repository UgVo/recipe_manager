#ifndef C_MAINWINDOW_H
#define C_MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class c_mainWindow; }
QT_END_NAMESPACE

class c_mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    c_mainWindow(QWidget *parent = nullptr);
    ~c_mainWindow();

private:
    Ui::c_mainWindow *ui;
};
#endif // C_MAINWINDOW_H
