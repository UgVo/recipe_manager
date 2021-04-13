#include "c_recipe.h"
#include "c_dbmanager.h"

c_recipe::c_recipe(int _serving, QList<c_milestone> _planning,
                   QList<c_process> _globalProcessing, QString _imageUrl,
                   QString _settingUpImageUrl, QList<c_note> _notes,
                   int _id)
    : id(_id), imageUrl(_imageUrl), settingUpImageUrl(_settingUpImageUrl),
        servings(_serving), notes(_notes), planning(_planning),
        globalProcessing(_globalProcessing) {
    complete = true;
    std::sort(notes.begin(),notes.end());
    std::sort(planning.begin(),planning.end());
    std::sort(globalProcessing.begin(),globalProcessing.end());
}

c_recipe::c_recipe(const c_recipe &recipe) {
    id = recipe.getId();
    imageUrl = recipe.getImageUrl();
    settingUpImageUrl = recipe.getSettingUpImageUrl();
    servings = recipe.getServings();
    notes = recipe.getNotes();
    planning = recipe.getPlanning();
    globalProcessing = recipe.getGlobalProcessing();
    complete = recipe.isComplete();

    std::sort(notes.begin(),notes.end());
    std::sort(planning.begin(),planning.end());
    std::sort(globalProcessing.begin(),globalProcessing.end());
}

c_recipe::~c_recipe() {

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
    return notes;
}

QList<c_note> c_recipe::getNotes() const {
    return notes;
}


void c_recipe::setNotes(const QList<c_note> &value) {
    notes = value;
    std::sort(notes.begin(),notes.end());
}

QList<c_milestone> c_recipe::getPlanning() {
    return planning;
}

QList<c_milestone> c_recipe::getPlanning() const {
    return planning;
}

void c_recipe::setPlanning(const QList<c_milestone> &value) {
    planning = value;
    std::sort(planning.begin(),planning.end());
}

QList<int> c_recipe::getMilestonesIds() {
    QList<int> res;
    for (QList<c_milestone>::iterator it = planning.begin(); it != planning.end(); ++it) {
        if (it->getId() != -1)
            res.append(it->getId());
    }
    return res;
}

bool c_recipe::addMilestone(const c_milestone milestone, int rank) {
    return false;
}

QList<c_process> c_recipe::getGlobalProcessing() {
    return globalProcessing;
}

QList<c_process> c_recipe::getGlobalProcessing() const {
    return globalProcessing;
}

void c_recipe::setGlobalProcessing(const QList<c_process> &value) {
    globalProcessing = value;
    std::sort(globalProcessing.begin(),globalProcessing.end());
}

c_recipe &c_recipe::operator=(const c_recipe &other) {
    id = other.getId();
    imageUrl = other.getImageUrl();
    settingUpImageUrl = other.getSettingUpImageUrl();
    servings = other.getServings();
    notes = other.getNotes();
    planning = other.getPlanning();
    globalProcessing = other.getGlobalProcessing();

    std::sort(notes.begin(),notes.end());
    std::sort(planning.begin(),planning.end());
    std::sort(globalProcessing.begin(),globalProcessing.end());

    return *this;
}

bool c_recipe::operator==(const c_recipe &other) const {
    return (imageUrl == other.getImageUrl())
            && (settingUpImageUrl == other.getSettingUpImageUrl())
            && (servings == other.getServings())
            && (notes == other.getNotes())
            && (planning == other.getPlanning())
            && (globalProcessing == other.getGlobalProcessing());
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
    globalProcessing = c_dbManager::getRecipeProcesses(id);
    std::sort(globalProcessing.begin(),globalProcessing.end());
}

void c_recipe::completeNotes() {
    notes = c_dbManager::getRecipeNotes(id);
    std::sort(notes.begin(),notes.end());
}

void c_recipe::completeMilestones() {
    planning = c_dbManager::getMilestones(id);
    std::sort(planning.begin(),planning.end());
    for (QList<c_milestone>::iterator it = planning.begin(); it != planning.end(); ++it) {
        it->completeMilestone();
    }
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
