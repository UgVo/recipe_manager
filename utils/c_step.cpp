#include "c_step.h"
#include "c_dbmanager.h"

c_step::c_step(int _rank, QString _description, QList<QString> _imagesUrl, QList<QString> _equipments, QList<c_component> _components, QList<c_note> _notes, QList<c_process> _processings, int _id)
    : id(_id), rank(_rank), components(_components), equipments(_equipments), description(_description), imagesUrl(_imagesUrl), notes(_notes), processings(_processings) {
    complete = true;
    std::sort(notes.begin(),notes.end());
}

c_step::~c_step() {

}

c_step::c_step(const c_step &other) {
    complete = other.isCompleted();
    id = other.getId();
    rank = other.getRank();
    components = other.getComponents();
    equipments = other.getEquipments();
    description = other.getDescription();
    imagesUrl = other.getImagesUrl();
    notes = other.getNotes();
    std::sort(notes.begin(),notes.end());
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
}

QList<c_component> c_step::getComponents() const {
    return components;
}

QList<c_component *> c_step::getComponentsPtr() {
    QList<c_component*> res;
    for (int i = 0; i < components.size(); ++i) {
        res.push_back(&(components[i]));
    }
    return res;
}

void c_step::setComponents(const QList<c_component> &value) {
    components = value;
}

c_component *c_step::newComponent() {
    components.push_back(c_component(c_ingredient()));
    return &components.last();
}

void c_step::removeComponent(const c_component *value) {
    for (int i = 0; i < components.size(); ++i) {
        if (&components[i] == value) {
            components.removeAt(i);
            return;
        }
    }
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
    res.remove(QRegExp(",$"));
    return res;
}

void c_step::setImagesUrl(const QList<QString> &value) {
    imagesUrl = value;
}

void c_step::setImagesUrlStr(const QString &value) {
    imagesUrl = value.split(",");
}

QList<c_note> c_step::getNotes() const {
    return notes;
}

QList<c_note *> c_step::getNotesPtr() {
    QList<c_note*> res;
    for (int i = 0; i < notes.size(); ++i) {
        res.push_back(&(notes[i]));
    }
    return res;
}

void c_step::setNotes(const QList<c_note> &value) {
    notes = value;
    std::sort(notes.begin(),notes.end());
}

c_note *c_step::addNote(const c_note value) {
    notes.append(value);
    c_note* ptr = &notes.last();
    std::sort(notes.begin(),notes.end());
    return ptr;
}

void c_step::deleteNote(const c_note value){
    notes.removeOne(value);
}

QList<c_process> c_step::getProcessings() const {
    return processings;
}

QList<c_process *> c_step::getProcessingsPtr() {
    QList<c_process*> res;
    for (int i = 0; i < processings.size(); ++i) {
        res.push_back(&(processings[i]));
    }
    return res;
}

void c_step::setProcessings(const QList<c_process> &value) {
    processings = value;
}

c_process *c_step::newProcessing() {
    processings.push_back(c_process());
    return &processings.last();
}

void c_step::removeProcessing(const c_process *value) {
    for (int i = 0; i < processings.size(); ++i) {
        if (&processings[i] == value) {
            processings.removeAt(i);
            return;
        }
    }
}

c_step &c_step::operator=(const c_step &other) {
    complete = other.isCompleted();
    id = other.getId();
    rank = other.getRank();
    components = other.getComponents();
    equipments = other.getEquipments();
    description = other.getDescription();
    imagesUrl = other.getImagesUrl();
    notes = other.getNotes();

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
    if (components.size() == other.getComponents().size()) {
        for (int i = 0; i < components.size(); ++i) {
            if (!other.getComponents().contains(components[i])) {
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
    components = c_dbManager::getComponents(id);
}

void c_step::completeNotes() {
    notes = c_dbManager::getStepNotes(id);
}

void c_step::completeEquipements() {
    equipments = c_dbManager::getEquipments(id);
}
