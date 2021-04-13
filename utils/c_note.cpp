#include "c_note.h"

c_note::c_note(QString _text, QDateTime _date, int _id) : id(_id), text(_text), date(_date) { }

int c_note::getId() const {
    return id;
}

void c_note::setId(int value) {
    id = value;
}

QString c_note::getText() const {
    return text;
}

void c_note::setText(const QString &value) {
    text = value;
}

QDateTime c_note::getDate() const {
    return date;
}

void c_note::setDate(const QDateTime &value) {
    date = value;
}

c_note &c_note::operator=(const c_note &other) {
    id = other.getId();
    text = other.getText();
    date = other.getDate();

    return *this;
}

bool c_note::operator==(const c_note &other) const {
    return (!text.compare(other.getText()) && (date == other.getDate()));
}

bool c_note::operator<(const c_note &other) const {
    return date < other.date;
}
