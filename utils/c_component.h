#ifndef C_COMPONENT_H
#define C_COMPONENT_H

#include <QObject>
#include <QMap>
#include "utils/utils.h"
#include <QDebug>

class c_ingredient;

class c_component
{

public:
    c_component(const c_ingredient& ingredient, const int quantity = 0, const recipe::unit unit = recipe::unit::none , const int id = -1);
    c_component(const c_component& other);
    ~c_component();

    c_ingredient& getIngredient();
    c_ingredient& getIngredient() const;
    void setIngredient(const c_ingredient &value);

    int getQuantity() const;
    void setQuantity(int value);

    recipe::unit getUnit() const;
    void setUnit(recipe::unit value);

    int getId() const;
    void setId(int value);

    c_component &operator=(const c_component &other);
    bool operator==(const c_component &other) const;

private:
    c_ingredient *ingredient;
    int quantity;
    recipe::unit unit;
    int id;

signals:

};

#endif // C_COMPONENT_H
