#include "utils.h"

namespace recipe {
    QMap<unit,QString> unitToString {
        {unit::g,"g"},
        {unit::L,"L"},
        {unit::dL,"dL"},
        {unit::cL,"cL"},
        {unit::mL,"mL"},
        {unit::cuillere,QObject::tr("cuillère")},
        {unit::pincee,QObject::tr("pincée")},
        {unit::none,""}
    };

    QString toCapitalised(QString str) {
        return str.left(1).toUpper()+str.mid(1);
    }
}
