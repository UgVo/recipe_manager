#ifndef C_MILESTONE_H
#define C_MILESTONE_H

#include "utils/c_step.h"
#include "utils/utils.h"
#include <QList>
#include <QString>
#include <QRegExp>
#include <QObject>

class c_milestone : public QObject
{
    Q_OBJECT

public:
    c_milestone(int rank = 0, QList<c_step> steps = QList<c_step>(), QString name = "", int id = -1);
    c_milestone(const c_milestone &other);

    int getId() const;
    void setId(int value);

    void normalizeStepOrder();
    QList<c_step> getSteps() const;
    QList<c_step *> getStepsPtr();
    void setSteps(const QList<c_step> &value);
    QList<int> getStepsIds() const;

    void completeMilestone();

    int getRank() const;
    void setRank(int value);

    bool operator==(const c_milestone& other) const;
    c_milestone &operator=(const c_milestone& other);
    bool operator<(const c_milestone& other) const;

    bool isComplete() const;
    void setComplete(bool value);

    QString getName() const;
    void setName(const QString &value);

signals:
    void rankChanged();

private:
    int id;
    int rank;
    QString name;
    bool complete;
    QList<c_step> steps;
};

#endif // C_MILESTONE_H
