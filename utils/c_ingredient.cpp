#include "c_ingredient.h"
#include "c_dbmanager.h"

c_ingredient::c_ingredient(QString _type, QString _name, c_recipe _subRecipe, int _id) :
    type(_type.toLower()), name(_name.toLower()), subRecipe(_subRecipe) ,id(_id) {
}

c_ingredient::c_ingredient(const c_ingredient &ingredient) {
    id = ingredient.getId();
    type = ingredient.getType();
    subRecipe = ingredient.getSubRecipe();
    name = ingredient.getName();
}

int c_ingredient::getId() const {
    return id;
}

void c_ingredient::setId(int value) {
    id = value;
}

QString c_ingredient::getType() const {
    return type;
}

void c_ingredient::setType(const QString &value) {
    type = value.toLower();
}

bool c_ingredient::asSubRecipe() const {
    return subRecipe.getId()!=-1;
}

c_recipe c_ingredient::getSubRecipe() const {
    return subRecipe;
}

c_recipe& c_ingredient::getSubRecipe() {
    return subRecipe;
}

void c_ingredient::setSubRecipe(c_recipe value) {
    subRecipe = value;
}

c_ingredient &c_ingredient::operator=(const c_ingredient &other) {
    id = other.getId();
    type = other.getType();
    subRecipe = other.getSubRecipe();
    name = other.getName();
    return *this;
}

bool c_ingredient::operator==(const c_ingredient &other) const {
    return (type == other.getType())
            && (name == other.getName())
            && (subRecipe == other.getSubRecipe());
}

int c_ingredient::registerIngredient() {
    if (id == -1) {
        id = c_dbManager::addIngredient(*this);
    } else {
        c_dbManager::updateIngredient(*this);
    }
    return id;
}

QString c_ingredient::getName() const {
    return name;
}

void c_ingredient::setName(const QString &value) {
    name = value.toLower();
}
