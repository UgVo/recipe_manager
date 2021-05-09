#include "c_notesdialog.h"
#include "ui_c_notesdialog.h"
#include "ui/c_stepview.h"

int c_notesDialog::spaceSliders = 3;
int c_notesDialog::heightSliders = 20;
int c_notesDialog::border = 3;

c_notesDialog::c_notesDialog(QList<c_note *> notes, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::c_notesDialog)
{
    ui->setupUi(this);

    for (int i = 0; i < notes.size(); ++i) {
        noteList.append(new c_noteView(notes[i]));
        static_cast<QHBoxLayout*>(ui->scrollAreaContents->layout())->insertWidget(0,noteList.last());
        QObject::connect(noteList.last(),&c_noteView::scrollToMe,this,&c_notesDialog::scrollTo);
        QObject::connect(noteList.last(),&c_noteView::deleteMe,this,&c_notesDialog::slotDeleteNote);
    }
    ui->scrollArea->setStyleSheet(" QScrollArea#scrollArea {"
                                  "     border : 0px solid black;"
                                  "}"
                                  " QWidget#scrollAreaContents {"
                                  "     background-color : white;"
                                  "     border-radius: 4px;"
                                  "}");
    upButton = new QPushButton("↑",this);
    upButton->setFixedHeight(heightSliders);
    upButton->hide();
    downButton = new QPushButton("↓",this);
    downButton->setFixedHeight(heightSliders);
    downButton->hide();
    addButton = new QPushButton("+",this);
    addButton->setFixedSize(QSize(20,20));

    this->setWindowTitle("Notes");

    QObject::connect(upButton,&QPushButton::pressed,this,&c_notesDialog::slotUpButtonPressed);
    QObject::connect(upButton,&QPushButton::released,this,&c_notesDialog::slotUpButtonReleased);
    QObject::connect(downButton,&QPushButton::pressed,this,&c_notesDialog::slotDownButtonPressed);
    QObject::connect(downButton,&QPushButton::released,this,&c_notesDialog::slotDownButtonReleased);
    QObject::connect(ui->scrollArea->verticalScrollBar(),&QScrollBar::valueChanged,this,&c_notesDialog::slotScrollValueChanged);
    QObject::connect(addButton,&QPushButton::released,this,&c_notesDialog::slotNewNote);
}

c_notesDialog::~c_notesDialog()
{
    delete ui;
}

void c_notesDialog::resizeEvent(QResizeEvent *) {
    QScrollBar* bar = ui->scrollArea->verticalScrollBar();

    upButton->setFixedWidth(int(float(ui->scrollArea->width())*0.9f));
    downButton->setFixedWidth(int(float(ui->scrollArea->width())*0.9f));

    upButton->move(ui->scrollArea->pos() + QPoint(int(float(ui->scrollArea->width())*0.05f),spaceSliders));
    downButton->move(ui->scrollArea->pos() + QPoint(int(float(ui->scrollArea->width())*0.05f),ui->scrollArea->height() - downButton->height() - spaceSliders));
    addButton->move(QPoint(this->width()- (border + addButton->width()),border));

    if (bar->value() !=  bar->minimum()) {
        upButton->show();
    } else {
        upButton->hide();
    }

    if (bar->value() != bar->maximum()) {
        downButton->show();
    } else {
        downButton->hide();
    }
}

void c_notesDialog::slotScrollValueChanged() {
    QScrollBar* bar = ui->scrollArea->verticalScrollBar();
    if (bar->value() !=  bar->minimum()) {
        upButton->show();
    } else {
        upButton->hide();
    }

    if (bar->value() != bar->maximum()) {
        downButton->show();
    } else {
        downButton->hide();
    }
}

void c_notesDialog::slotUpButtonPressed() {
    QObject::connect(&timer,&QTimer::timeout,this,&c_notesDialog::slotTimerUpTimeout);
    timer.start(200);
}

void c_notesDialog::slotUpButtonReleased() {
    QScrollBar* bar = ui->scrollArea->verticalScrollBar();
    bar->setValue(bar->value()-bar->singleStep());
    timer.stop();
    timer.disconnect();
}

void c_notesDialog::slotTimerUpTimeout() {
    QObject::disconnect(&timer,&QTimer::timeout,this,&c_notesDialog::slotTimerUpTimeout);
    QObject::connect(&timer,&QTimer::timeout,this,&c_notesDialog::slotScrollUp);
    timer.start(100);
}

void c_notesDialog::slotScrollUp() {
    if (upButton->isDown()) {
        QScrollBar* bar = ui->scrollArea->verticalScrollBar();
        bar->setValue(bar->value()-bar->singleStep());
    } else {
        timer.stop();
        timer.disconnect();
    }
}

void c_notesDialog::slotDownButtonPressed() {
    QObject::connect(&timer,&QTimer::timeout,this,&c_notesDialog::slotTimerDownTimeout);
    timer.start(200);
}

void c_notesDialog::slotDownButtonReleased() {
    QScrollBar* bar = ui->scrollArea->verticalScrollBar();
    bar->setValue(bar->value()+bar->singleStep());
    timer.stop();
    timer.disconnect();
}

void c_notesDialog::slotTimerDownTimeout() {
    QObject::disconnect(&timer,&QTimer::timeout,this,&c_notesDialog::slotTimerDownTimeout);
    QObject::connect(&timer,&QTimer::timeout,this,&c_notesDialog::slotScrollDown);
    timer.start(100);
}

void c_notesDialog::slotScrollDown() {
    if (downButton->isDown()) {
        QScrollBar* bar = ui->scrollArea->verticalScrollBar();
        bar->setValue(bar->value()+bar->singleStep());
    } else {
        timer.stop();
        timer.disconnect();
    }
}

void c_notesDialog::scrollTo() {
    c_noteView* sender = static_cast<c_noteView*>(QObject::sender());
    QScrollBar* bar = ui->scrollArea->verticalScrollBar();
    int slide = -bar->value() - heightSliders - spaceSliders;
    for (int i = noteList.size()-1; i >= 0; --i) {
        if (sender != noteList[i]) {
            slide += noteList[i]->height() + ui->scrollAreaContents->layout()->spacing();
        } else {
            break;
        }
    }
    QPropertyAnimation *anim = new QPropertyAnimation(bar,"value");
    anim->setDuration(500);
    anim->setStartValue(bar->value());
    anim->setEndValue(slide);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void c_notesDialog::slotNewNote() {
    noteList.append(new c_noteView(static_cast<c_stepView*>(parent())->getStep()->newNote()));
    static_cast<QHBoxLayout*>(ui->scrollAreaContents->layout())->insertWidget(0,noteList.last());
    QObject::connect(noteList.last(),&c_noteView::scrollToMe,this,&c_notesDialog::scrollTo);
    QObject::connect(noteList.last(),&c_noteView::deleteMe,this,&c_notesDialog::slotDeleteNote);
    noteList.last()->modify();
}

void c_notesDialog::slotDeleteNote() {
    c_noteView *sender = static_cast<c_noteView *>(QObject::sender());
    if (static_cast<c_stepView*>(parent())) {
        static_cast<c_stepView*>(parent())->deleteNote(sender->getNote());
    }
    sender->hide();
    static_cast<QHBoxLayout*>(ui->scrollAreaContents->layout())->removeWidget(sender);
    sender->deleteLater();
    noteList.removeOne(sender);
}

