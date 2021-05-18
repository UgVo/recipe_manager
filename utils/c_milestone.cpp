#include "c_milestone.h"
#include "c_dbmanager.h"

c_milestone::c_milestone(int _rank, QList<c_step> _steps, QString _name, int _id) : id (_id), rank(_rank), name(_name), complete(true) {
    for (int i = 0; i < _steps.size(); ++i) {
        steps.push_back(new c_step(_steps[i]));
    }
    normalizeStepOrder();
}

c_milestone::c_milestone(int _rank, QList<c_step *> _steps, QString _name, int _id) : id (_id), rank(_rank), name(_name), complete(true), steps(_steps) {
    normalizeStepOrder();
}

c_milestone::c_milestone(const c_milestone &other) {
    id = other.getId();
    rank = other.getRank();
    name = other.getName();
    complete = other.isComplete();
    setSteps(other.getSteps());
}

c_milestone::~c_milestone() {
    for (int i = 0; i < steps.size(); ++i) {
        if (steps[i] != nullptr)
            delete steps[i];
    }
    steps.clear();
}

int c_milestone::getId() const {
    return id;
}

void c_milestone::setId(int value) {
    id = value;
}

void c_milestone::normalizeStepOrder() {
    std::sort(steps.begin(),steps.end(), [] (c_step *step1, c_step *step2) {
        return *step1 < *step2;
    });
    for (int i = 0; i < steps.size(); ++i) {
        steps[i]->setRank(i+1);
    }
}

QList<c_step> c_milestone::getSteps() const {
    QList<c_step> res;
    for (int i = 0; i < steps.size(); ++i) {
        res.push_back(*steps[i]);
    }
    return res;

}

QList<c_step *> c_milestone::getStepsPtr() {
    return steps;
}

void c_milestone::setSteps(const QList<c_step> &value) {
    if (value.size() > steps.size()) {
        for (int i = 0; i < steps.size(); ++i) {
            *(steps[i]) = value[i];
        }
        for (qsizetype i = steps.size(); i < value.size(); ++i) {
            steps.push_back(new c_step(value[i]));
        }
    } else {
        for (int i = 0; i < value.size(); ++i) {
            *(steps[i]) = value[i];
        }
        qsizetype i = value.size();
        while (steps.size() > value.size()) {
            delete steps[i];
            steps.removeAt(i);
        }
    }
    normalizeStepOrder();
}

QList<int> c_milestone::getStepsIds() const {
    QList<int> res;
    for (QList<c_step*>::const_iterator it = steps.begin(); it != steps.end(); ++it) {
        res.push_back((*it)->getId());
    }
    return res;
}

bool c_milestone::swapSteps(c_step *step, recipe::swap direction) {
    qsizetype index = steps.indexOf(step,-1);
    if (index != -1) {
        if (direction == recipe::swapAbove) {
            if (index > 0) {
                steps.swapItemsAt(index,index-1);
                steps[index]->setRank(int(index+1));
                steps[index-1]->setRank(int(index));
                return true;
            }
        } else if (direction == recipe::swapBelow) {
            if (index < steps.size()-1) {
                steps.swapItemsAt(index,index+1);
                steps[index]->setRank(int(index+1));
                steps[index+1]->setRank(int(index+2));
                return true;
            }
        }
    }
    return false;
}

bool c_milestone::removeStep(c_step *step) {
    bool flag = steps.removeOne(step);
    normalizeStepOrder();
    return flag;
}

void c_milestone::completeMilestone() {
    setSteps(c_dbManager::getSteps(id));
    normalizeStepOrder();
    for (QList<c_step*>::iterator it = steps.begin(); it != steps.end(); ++it ) {
        (*it)->completeStep();
    }
    complete = true;
}

int c_milestone::getRank() const {
    return rank;
}

void c_milestone::setRank(int value) {
    rank = value;
    emit rankChanged();
}

bool c_milestone::operator==(const c_milestone &other) const {
    return rank == other.getRank()
            && steps == other.steps;
}

c_milestone &c_milestone::operator=(const c_milestone &other) {
    id = other.getId();
    rank = other.getRank();
    name = other.getName();
    complete = other.isComplete();
    steps = other.steps;

    return *this;
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
