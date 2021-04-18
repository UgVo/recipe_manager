#ifndef C_NOTEVIEW_H
#define C_NOTEVIEW_H

#include <QWidget>
#include <utils/c_note.h>
#include <QTextBlock>
#include <QMenu>
#include <QDebug>

namespace Ui {
class c_noteView;
}

class c_noteView : public QWidget
{
    Q_OBJECT

public:
    explicit c_noteView(c_note* note, QWidget *parent = nullptr);
    ~c_noteView();

    void resizeEvent(QResizeEvent *);
    void focusOutEvent(QFocusEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

    int getHeightText();

    void modify();
    void save();
    void cancel();

    c_note *getNote() const;
    bool isEmpty();

    void setNote(c_note *value);

signals:
    void scrollToMe();
    void deleteMe();
    void editFinished();

private:
    Ui::c_noteView *ui;
    c_note* note;
    bool textAltered;
};

#endif // C_NOTEVIEW_H
