#ifndef C_INGREDIENT_H
#define C_INGREDIENT_H

#include <QObject>
#include <QList>
#include "utils/utils.h"

class c_recipe;

class c_ingredient
{

public:
    c_ingredient(QString type = "none", QString name = "none", c_recipe *subRecipe = nullptr,int id = -1);
    c_ingredient(const c_ingredient& ingredient);

    int getId() const;
    void setId(int value);

    QString getType() const;
    void setType(const QString &value);

    bool asSubRecipe() const;
    c_recipe getSubRecipe() const;
    c_recipe& getSubRecipe();
    void setSubRecipe(c_recipe value);

    c_ingredient &operator=(const c_ingredient &other);
    bool operator==(const c_ingredient &other) const;

    int registerIngredient();

    QString getName() const;
    void setName(const QString &value);

    bool isEmpty();

private:
    QString type;
    QString name;
    c_recipe *subRecipe;
    int id;

signals:

};

#endif // C_INGREDIENT_H
