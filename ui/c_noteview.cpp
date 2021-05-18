#include "c_noteview.h"
#include "ui_c_noteview.h"

c_noteView::c_noteView(c_note* _note, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::c_noteView)
{
    ui->setupUi(this);
    note = _note;
    ui->textEdit->setText(note->getText());
    ui->textEdit->setAlignment(Qt::AlignJustify);
    ui->textEdit->setReadOnly(true);
    ui->dateLabel->setText(note->getDate().toString());
    ui->widget->setStyleSheet("QWidget#widget { border : 1px solid black; border-radius: 4px; background-color : #F0F0F0;}");
    ui->textEdit->setStyleSheet("QTextEdit {"
                             "  border : 0px solid white;"
                             "  background: transparent;"
                             "}");
    ui->textEdit->installEventFilter(this);
    ui->buttonBox->hide();

    QMenu *menu = new QMenu();
    menu->addAction("Edit",this,&c_noteView::modify);
    menu->addAction("Delete",this,[this] () {emit deleteMe();});

    ui->toolButton->setMenu(menu);
    ui->toolButton->setStyleSheet("QToolButton::menu-indicator{image:none;}");
    ui->toolButton->setPopupMode(QToolButton::InstantPopup);

    QObject::connect(ui->buttonBox,&QDialogButtonBox::accepted,this,&c_noteView::save);
    QObject::connect(ui->buttonBox,&QDialogButtonBox::rejected,this,&c_noteView::cancel);

    textAltered = false;
    QObject::connect(ui->textEdit,&QTextEdit::textChanged,[this] () {
        if (!textAltered) {
            textAltered = true;
            ui->buttonBox->show();
        }
    });
}

c_noteView::~c_noteView() {
    delete ui;
}

void c_noteView::resizeEvent(QResizeEvent *) {
    ui->textEdit->setFixedHeight(getHeightText());
}

void c_noteView::focusOutEvent(QFocusEvent *) {

}

bool c_noteView::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->textEdit) {
        if (event->type() == QEvent::FocusOut) {
            if (!ui->textEdit->isReadOnly() && !textAltered) {
                cancel();
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        // pass the event on to the parent class
        return QWidget::eventFilter(obj, event);
    }
}

int c_noteView::getHeightText() {
    QRect rect = ui->textEdit->rect();
    int res = 0;
    for (int i = 0; i < ui->textEdit->document()->blockCount(); ++i) {
        QFontMetrics metrics(ui->textEdit->document()->findBlockByNumber(i).charFormat().font());
        res += metrics.boundingRect(rect,Qt::TextWordWrap,ui->textEdit->document()->findBlockByNumber(i).text()).size().height();
    }
    // offset to compensate difference in height between text and QEditText
    return res + 10;
}

void c_noteView::modify() {
    ui->textEdit->setReadOnly(false);
    ui->textEdit->setFocus();

    ui->textEdit->setStyleSheet("");
    emit scrollToMe();
    ui->textEdit->setFocus();
}

void c_noteView::save() {
    note->setText(ui->textEdit->document()->toRawText());
    note->setDate(QDateTime::currentDateTime());
    ui->dateLabel->setText(QDateTime::currentDateTime().toString());
    ui->textEdit->setStyleSheet("QTextEdit {"
                             "  border : 0px solid white;"
                             "  background: transparent;"
                             "}");
    ui->buttonBox->hide();
    ui->textEdit->setReadOnly(true);
    textAltered = false;
}

void c_noteView::cancel() {
    ui->textEdit->setText(note->getText());
    ui->textEdit->setStyleSheet("QTextEdit {"
                             "  border : 0px solid white;"
                             "  background: transparent;"
                             "}");
    ui->buttonBox->hide();
    ui->textEdit->setReadOnly(true);
    textAltered = false;
}

c_note *c_noteView::getNote() const
{
    return note;
}

bool c_noteView::isEmpty() {
    return ui->textEdit->document()->toRawText().isEmpty();
}

void c_noteView::setNote(c_note *value) {
    note = value;
    ui->textEdit->setText(note->getText());
    ui->textEdit->setAlignment(Qt::AlignJustify);
    ui->textEdit->setReadOnly(true);
    ui->dateLabel->setText(note->getDate().toString());
}
