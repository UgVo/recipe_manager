#include "c_recipe.h"
#include "c_dbmanager.h"

c_recipe::c_recipe(int _serving,  QString _name, QList<c_milestone> _planning,
                   QList<c_process> _globalProcessing, QString _imageUrl,
                   QString _settingUpImageUrl, QList<c_note> _notes,
                   int _id)
    : id(_id), name(_name), imageUrl(_imageUrl), settingUpImageUrl(_settingUpImageUrl),
        servings(_serving) {
    complete = true;
    for (int i = 0; i < _planning.size(); ++i) {
        planning.push_back(new c_milestone(_planning[i]));
    }
    std::sort(planning.begin(),planning.end(),&recipe::compare<c_milestone>);

    for (int i = 0; i < _globalProcessing.size(); ++i) {
        globalProcessing.push_back(new c_process(_globalProcessing[i]));
    }

    for (int i = 0; i < _notes.size(); ++i) {
        notes.push_back(new c_note(_notes[i]));
    }
    std::sort(notes.begin(),notes.end(),&recipe::compare<c_note>);
}

c_recipe::c_recipe(int _serving, QString _name, QList<c_milestone *> _planning, QList<c_process *> _globalProcessing, QString _imageUrl, QString _settingUpImageUrl, QList<c_note *> _notes, int _id)
    :id(_id), name(_name), complete(true), imageUrl(_imageUrl), settingUpImageUrl(_settingUpImageUrl), servings(_serving), notes(_notes), planning(_planning), globalProcessing(_globalProcessing)
{
}

c_recipe::c_recipe() {
    id = -1;
    name = QString("Nom");
    complete = false;
    imageUrl = "";
    settingUpImageUrl = "";
    servings = -1;
    notes = QList<c_note *>();
    planning = QList<c_milestone *>();
    globalProcessing = QList<c_process *>();
}

c_recipe::c_recipe(const c_recipe &other) {
    *this = other;
}

c_recipe::~c_recipe() {
    for (int i = 0; i < planning.size(); ++i) {
        if (planning[i] != nullptr)
            delete  planning[i];
    }

    for (int i = 0; i < globalProcessing.size(); ++i) {
        if (globalProcessing[i] != nullptr)
            delete  globalProcessing[i];
    }

    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i] != nullptr)
            delete  notes[i];
    }
}

int c_recipe::getId() const {
    return id;
}

void c_recipe::setId(const int value) {
    id = value;
}

QString c_recipe::getImageUrl() const {
    return imageUrl;
}

void c_recipe::setImageUrl(const QString &value) {
    imageUrl = value;
}

QString c_recipe::getSettingUpImageUrl() const {
    return settingUpImageUrl;
}

void c_recipe::setSettingUpImageUrl(const QString &value) {
    settingUpImageUrl = value;
}

int c_recipe::getServings() const {
    return servings;
}

void c_recipe::setServings(int value) {
    servings = value;
}

QList<c_note> c_recipe::getNotes() {
    QList<c_note> res;
    for (int i = 0; i < notes.size(); ++i) {
        res.push_back(*notes[i]);
    }
    return res;
}

QList<c_note> c_recipe::getNotes() const {
    QList<c_note> res;
    for (int i = 0; i < notes.size(); ++i) {
        res.push_back(*notes[i]);
    }
    return res;
}

QList<c_note *> c_recipe::getNotesPtr() {
    return notes;
}


void c_recipe::setNotes(const QList<c_note> &value) {
    if (value.size() > notes.size()) {
        for (int i = 0; i < notes.size(); ++i) {
            *(notes[i]) = value[i];
        }
        for (qsizetype i = notes.size(); i < value.size(); ++i) {
            notes.push_back(new c_note(value[i]));
        }
    } else {
        for (int i = 0; i < value.size(); ++i) {
            *(notes[i]) = value[i];
        }
        qsizetype i = value.size();
        while (notes.size() > value.size()) {
            delete notes[i];
            notes.removeAt(i);
        }
    }
    std::sort(notes.begin(),notes.end(),&recipe::compare<c_note>);
}

QList<c_milestone> c_recipe::getPlanning() {
    QList<c_milestone> res;
    for (int i = 0; i < planning.size(); ++i) {
        res.push_back(*planning[i]);
    }
    return res;
}

QList<c_milestone> c_recipe::getPlanning() const {
    QList<c_milestone> res;
    for (int i = 0; i < planning.size(); ++i) {
        res.push_back(*planning[i]);
    }
    return res;
}

QList<c_milestone *> c_recipe::getPlanningPtr() {
    return planning;
}

void c_recipe::setPlanning(const QList<c_milestone> &value) {
    if (value.size() > planning.size()) {
        for (int i = 0; i < planning.size(); ++i) {
            *(planning[i]) = value[i];
        }
        for (qsizetype i = planning.size(); i < value.size(); ++i) {
            planning.push_back(new c_milestone(value[i]));
        }
    } else {
        for (int i = 0; i < value.size(); ++i) {
            *(planning[i]) = value[i];
        }
        qsizetype i = value.size();
        while (planning.size() > value.size()) {
            delete planning[i];
            planning.removeAt(i);
        }
    }
    std::sort(planning.begin(),planning.end(),&recipe::compare<c_milestone>);
}

QList<int> c_recipe::getMilestonesIds() {
    QList<int> res;
    for (QList<c_milestone*>::iterator it = planning.begin(); it != planning.end(); ++it) {
        if ((*it)->getId() != -1)
            res.append((*it)->getId());
    }
    return res;
}

bool c_recipe::addMilestone(const c_milestone milestone, int rank) {
    return false;
}

QList<c_process> c_recipe::getGlobalProcessing() {
    QList<c_process> res;
    for (int i = 0; i < globalProcessing.size(); ++i) {
        res.push_back(*globalProcessing[i]);
    }
    return res;
}

QList<c_process> c_recipe::getGlobalProcessing() const {
    QList<c_process> res;
    for (int i = 0; i < globalProcessing.size(); ++i) {
        res.push_back(*globalProcessing[i]);
    }
    return res;
}

QList<c_process *> c_recipe::getGlobalProcessingPtr() {
    return globalProcessing;
}

void c_recipe::setGlobalProcessing(const QList<c_process> &value) {
    if (value.size() > globalProcessing.size()) {
        for (int i = 0; i < globalProcessing.size(); ++i) {
            *(globalProcessing[i]) = value[i];
        }
        for (qsizetype i = globalProcessing.size(); i < value.size(); ++i) {
            globalProcessing.push_back(new c_process(value[i]));
        }
    } else {
        for (int i = 0; i < value.size(); ++i) {
            *(globalProcessing[i]) = value[i];
        }
        qsizetype i = value.size();
        while (globalProcessing.size() > value.size()) {
            delete globalProcessing[i];
            globalProcessing.removeAt(i);
        }
    }
}

c_recipe &c_recipe::operator=(const c_recipe &other) {
    id = other.getId();
    name = other.name;
    imageUrl = other.getImageUrl();
    settingUpImageUrl = other.getSettingUpImageUrl();
    servings = other.getServings();
    setNotes(other.getNotes());
    setPlanning(other.getPlanning());
    setGlobalProcessing(other.getGlobalProcessing());

    return *this;
}

bool c_recipe::operator==(const c_recipe &other) const {
    return (imageUrl == other.getImageUrl())
            && (settingUpImageUrl == other.getSettingUpImageUrl())
            && (servings == other.getServings())
            && (getNotes() == other.getNotes())
            && (getPlanning() == other.getPlanning())
            && (getGlobalProcessing() == other.getGlobalProcessing());
}

c_recipe c_recipe::creatEmpty(int id) {
    c_recipe res;
    res.setId(id);
    return res;
}

bool c_recipe::isEmpty() {
    return (imageUrl == "")
            && (settingUpImageUrl == "")
            && (servings == 0)
            && (notes.empty())
            && (planning.empty())
            && (globalProcessing.empty());
}

void c_recipe::completeProcessings() {
    setGlobalProcessing(c_dbManager::getRecipeProcesses(id));
    std::sort(globalProcessing.begin(),globalProcessing.end());
}

void c_recipe::completeNotes() {
    setNotes(c_dbManager::getRecipeNotes(id));
    std::sort(notes.begin(),notes.end());
}

void c_recipe::completeMilestones() {
    setPlanning(c_dbManager::getMilestones(id));
    for (QList<c_milestone *>::iterator it = planning.begin(); it != planning.end(); ++it) {
        (*it)->completeMilestone();
    }
}

const QString &c_recipe::getName() const {
    return name;
}

void c_recipe::setName(const QString &newName) {
    if (name == newName)
        return;
    name = newName;
    emit nameChanged();
}

bool c_recipe::isComplete() const {
    return complete;
}

void c_recipe::setComplete(bool value) {
    complete = value;
}

void c_recipe::completeRecipe() {
    if (!complete) {
        completeProcessings();
        completeNotes();
        completeMilestones();
    }
    complete = true;
}
