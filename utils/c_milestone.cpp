#include "c_milestone.h"
#include "c_dbmanager.h"

c_milestone::c_milestone(int _rank, QList<c_step> _steps, QString _name, int id) : id (id), rank(_rank), name(_name), complete(true), steps(_steps) {
    normalizeStepOrder();
}

int c_milestone::getId() const {
    return id;
}

void c_milestone::setId(int value) {
    id = value;
}

void c_milestone::normalizeStepOrder() {
    std::sort(steps.begin(),steps.end());
    for (int i = 0; i < steps.size(); ++i) {
        steps[i].setRank(i+1);
    }
}

QList<c_step> c_milestone::getSteps() const {
    return steps;
}

QList<c_step *> c_milestone::getStepsPtr() {
    QList<c_step *> res;
    for (int i = 0; i < steps.size(); ++i) {
        res.push_back(&steps[i]);
    }
    return res;
}

void c_milestone::setSteps(const QList<c_step> &value) {
    steps = value;
    normalizeStepOrder();
}

QList<int> c_milestone::getStepsIds() const {
    QList<int> res;
    for (QList<c_step>::const_iterator it = steps.begin(); it != steps.end(); ++it) {
        res.push_back(it->getId());
    }
    return res;
}

void c_milestone::completeMilestone() {
    steps = c_dbManager::getSteps(id);
    normalizeStepOrder();
    for (QList<c_step>::iterator it = steps.begin(); it != steps.end(); ++it ) {
        it->completeStep();
    }
    complete = true;
}

int c_milestone::getRank() const {
    return rank;
}

void c_milestone::setRank(int value) {
    rank = value;
}

bool c_milestone::operator==(const c_milestone &other) const {
    return rank == other.getRank()
            && steps == other.getSteps();
}

bool c_milestone::operator<(const c_milestone &other) const {
    return rank < other.getRank();
}

bool c_milestone::isComplete() const {
    return complete;
}

void c_milestone::setComplete(bool value) {
    complete = value;
}

QString c_milestone::getName() const
{
    return name;
}

void c_milestone::setName(const QString &value)
{
    name = value;
}
