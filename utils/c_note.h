#ifndef C_NOTE_H
#define C_NOTE_H

#include <QString>
#include <QDateTime>
#include "utils/utils.h"

class c_note
{
public:
    c_note(QString text, QDateTime date = QDateTime::currentDateTime() , int id = -1);

    int getId() const;
    void setId(int value);

    QString getText() const;
    void setText(const QString &value);

    QDateTime getDate() const;
    void setDate(const QDateTime &value);

    c_note &operator=(const c_note &other);
    bool operator==(const c_note &other) const;
    bool operator<(const c_note &other) const;

private:
    int id;
    QString text;
    QDateTime date;
};

#endif // C_NOTE_H
