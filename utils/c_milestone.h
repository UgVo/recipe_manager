#ifndef C_MILESTONE_H
#define C_MILESTONE_H

#include "utils/c_step.h"
#include "utils/utils.h"
#include <QList>
#include <QString>
#include <QRegExp>

class c_milestone
{
public:
    c_milestone(int rank = 0, QList<c_step> steps = QList<c_step>(), int id = -1);

    int getId() const;
    void setId(int value);

    QList<c_step> getSteps() const;
    QList<c_step *> getStepsPtr();
    void setSteps(const QList<c_step> &value);
    QList<int> getStepsIds() const;

    void completeMilestone();

    int getRank() const;
    void setRank(int value);

    bool operator==(const c_milestone& other) const;
    bool operator<(const c_milestone& other) const;

    bool isComplete() const;
    void setComplete(bool value);

private:
    int id;
    int rank;
    bool complete;
    QList<c_step> steps;
};

#endif // C_MILESTONE_H
