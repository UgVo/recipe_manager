#include "utils.h"
#include "c_component.h"
#include "c_dbmanager.h"
#include "c_recipe.h"
#include "c_ingredient.h"

namespace recipe {
    QMap<int,QString> unitToString {
        {unit::g,"g"},
        {unit::L,"L"},
        {unit::dL,"dL"},
        {unit::cL,"cL"},
        {unit::mL,"mL"},
        {unit::cuillere,QObject::tr("cuillère")},
        {unit::pincee,QObject::tr("pincée")},
        {unit::none,""}
    };
}
