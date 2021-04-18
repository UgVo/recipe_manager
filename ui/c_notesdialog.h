#ifndef C_NOTESDIALOG_H
#define C_NOTESDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QScrollBar>
#include <QTimer>
#include <QDebug>
#include <QPropertyAnimation>
#include "c_noteview.h"

class c_stepView;

namespace Ui {
class c_notesDialog;
}

class c_notesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit c_notesDialog(QList<c_note*> notes, QWidget *parent = nullptr);
    ~c_notesDialog();

    void resizeEvent(QResizeEvent *);

public slots:
    void slotScrollValueChanged();

    void slotUpButtonPressed();
    void slotUpButtonReleased();
    void slotTimerUpTimeout();
    void slotScrollUp();

    void slotDownButtonPressed();
    void slotDownButtonReleased();
    void slotTimerDownTimeout();
    void slotScrollDown();

    void scrollTo();

    void slotNewNote();
    void slotEditFinished();
    void slotDeleteNote();

private:
    Ui::c_notesDialog *ui;
    QList<c_noteView*> noteList;
    QPushButton *upButton;
    QPushButton *downButton;
    QPushButton *addButton;

    QTimer timer;

    static int spaceSliders;
    static int heightSliders;
    static int border;
};

#endif // C_NOTESDIALOG_H
