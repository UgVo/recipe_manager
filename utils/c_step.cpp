#include "c_step.h"
#include "c_dbmanager.h"

c_step::c_step(int _rank, QString _description, QList<QString> _imagesUrl, QList<QString> _equipments, QList<c_component> _components, QList<c_note> _notes, QList<c_process> _processings, int _id)
    : id(_id), rank(_rank), equipments(_equipments), description(_description), imagesUrl(_imagesUrl) {
    complete = true;
    for (int i = 0; i < _processings.size(); ++i) {
        processings.push_back(new c_process(_processings[i]));
    }
    for (int i = 0; i < _components.size(); ++i) {
        components.push_back(new c_component(_components[i]));
    }
    for (int i = 0; i < _notes.size(); ++i) {
        notes.push_back(new c_note(_notes[i]));
    }
    std::sort(notes.begin(),notes.end(),[] (c_note *note1, c_note *note2) {
        return *note1 < *note2;
    });
}

c_step::~c_step() {
    for (int i = 0; i < processings.size(); ++i) {
        if (processings[i] != nullptr)
            delete processings[i];
    }
    processings.clear();
    for (int i = 0; i < components.size(); ++i) {
        if (components[i] != nullptr)
            delete components[i];
    }
    components.clear();
    for (int i = 0; i < notes.size(); ++i) {
        if (notes[i] != nullptr)
            delete notes[i];
    }
    notes.clear();
}

c_step::c_step(const c_step &other) {
    complete = other.isCompleted();
    id = other.getId();
    rank = other.getRank();
    setComponents(other.getComponents());
    equipments = other.getEquipments();
    description = other.getDescription();
    imagesUrl = other.getImagesUrl();
    setProcessings(other.getProcessings());
    setNotes(other.getNotes());
    std::sort(notes.begin(),notes.end(),[] (c_note *note1, c_note *note2) {
        return *note1 < *note2;
    });
}

int c_step::getId() const {
    return id;
}

void c_step::setId(int value) {
    id = value;
}

int c_step::getRank() const {
    return rank;
}

void c_step::setRank(int value) {
    rank = value;
    emit rankChanged();
}

QList<c_component> c_step::getComponents() const {
    QList<c_component> res;
    for (int i = 0; i < components.size(); ++i) {
        res.push_back(*(components[i]));
    }
    return res;
}

QList<c_component *> c_step::getComponentsPtr() {
    return components;
}

void c_step::setComponents(const QList<c_component> &value) {
    if (value.size() > components.size()) {
        for (int i = 0; i < components.size(); ++i) {
            *(components[i]) = value[i];
        }
        for (qsizetype i = components.size(); i < value.size(); ++i) {
            components.push_back(new c_component(value[i]));
        }
    } else {
        for (int i = 0; i < value.size(); ++i) {
            *(components[i]) = value[i];
        }
        qsizetype i = value.size();
        while (components.size() > value.size()) {
            delete components[i];
            components.removeAt(i);
        }
    }
}

c_component *c_step::newComponent() {
    components.push_back(new c_component(c_ingredient()));
    return components.last();
}

void c_step::removeComponent(const c_component *value) {
    components.removeOne(value);
}

QList<QString> c_step::getEquipments() const {
    return equipments;
}

void c_step::setEquipments(const QList<QString> &value) {
    equipments = value;
}

QString c_step::getDescription() const {
    return description;
}

void c_step::setDescription(const QString &value) {
    description = value;
}

QList<QString> c_step::getImagesUrl() const {
    return imagesUrl;
}

QString c_step::getImagesUrlsStr() const {
    QString res;
    foreach (QString url, imagesUrl) {
        res.append(url);
        res.append(",");
    }
    res.remove(QRegularExpression(",$"));
    return res;
}

void c_step::setImagesUrl(const QList<QString> &value) {
    imagesUrl = value;
}

void c_step::setImagesUrlStr(const QString &value) {
    imagesUrl = value.split(",");
}

QList<c_note> c_step::getNotes() const {
    QList<c_note> res;
    for (int i = 0; i < notes.size(); ++i) {
        res.push_back(*(notes[i]));
    }
    return res;
}

QList<c_note *> c_step::getNotesPtr() {
    return notes;
}

void c_step::setNotes(const QList<c_note> &value) {
    if (value.size() > notes.size()) {
        for (int i = 0; i < processings.size(); ++i) {
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
    std::sort(notes.begin(),notes.end(),[] (c_note *note1, c_note *note2) {
        return *note1 < *note2;
    });
}

c_note *c_step::newNote() {
    notes.push_back(new c_note());
    return notes.last();
}

void c_step::deleteNote(const c_note *value){
    notes.removeOne(value);
}

QList<c_process> c_step::getProcessings() const {
    QList<c_process> res;
    for (int i = 0; i < processings.size(); ++i) {
        res.push_back(*(processings[i]));
    }
    return res;
}

QList<c_process *> c_step::getProcessingsPtr() {
    return processings;
}

void c_step::setProcessings(const QList<c_process> &value) {
    if (value.size() > processings.size()) {
        for (int i = 0; i < processings.size(); ++i) {
            *(processings[i]) = value[i];
        }
        for (qsizetype i = processings.size(); i < value.size(); ++i) {
            processings.push_back(new c_process(value[i]));
        }
    } else {
        for (int i = 0; i < value.size(); ++i) {
            *(processings[i]) = value[i];
        }
        qsizetype i = value.size();
        while (processings.size() > value.size()) {
            delete processings[i];
            processings.removeAt(i);
        }
    }
}

c_process *c_step::newProcessing() {
    processings.push_back(new c_process());
    return processings.last();
}

void c_step::removeProcessing(const c_process *value) {
    processings.removeOne(value);
}

c_step &c_step::operator=(const c_step &other) {
    complete = other.isCompleted();
    id = other.getId();
    rank = other.getRank();
    components = other.components;
    equipments = other.getEquipments();
    description = other.getDescription();
    imagesUrl = other.getImagesUrl();
    processings = other.processings;
    notes = other.notes;
    std::sort(notes.begin(),notes.end(),[] (c_note *note1, c_note *note2) {
        return *note1 < *note2;
    });

    return *this;
}

bool c_step::operator==(const c_step &other) const {
    if (equipments.size() == other.getEquipments().size()) {
        foreach (QString equip,other.getEquipments()) {
            if (!equipments.contains(equip)) {
                return false;
            }
        }
    }
    if (imagesUrl.size() == other.getImagesUrl().size()) {
        foreach (QString image, other.getImagesUrl()) {
            if (!imagesUrl.contains(image)) {
                return false;
            }
        }
    }
    if (components.size() == other.components.size()) {
        for (int i = 0; i < components.size(); ++i) {
            if (!other.components.contains(components[i])) {
                return false;
            }
        }
    }
    bool flag = (rank == other.rank) && (description == other.getDescription())
                && complete && other.isCompleted();

    return flag;
}
bool c_step::operator!=(const c_step &other) const {
    return !(*this==other);
}

bool c_step::operator<(const c_step &other) const {
    return rank < other.getRank();
}

bool c_step::isCompleted() const {
    return complete;
}

void c_step::setComplete(bool value) {
    complete = value;
}

void c_step::completeStep() {
    if (!complete) {
        completeComponents();
        completeEquipements();
        completeNotes();
        complete = true;
    }
}

void c_step::completeComponents() {
    setComponents(c_dbManager::getComponents(id));
}

void c_step::completeNotes() {
    setNotes(c_dbManager::getStepNotes(id));
}

void c_step::completeEquipements() {
    equipments = c_dbManager::getEquipments(id);
}
