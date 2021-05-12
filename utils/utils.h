#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QList>
#include <QMap>
#include <QObject>

namespace recipe
{
    enum unit {g,L,dL,cL,mL,cuillere,pincee,none};
    enum swap{swapAbove,swapBelow};
    extern QMap<int,QString> unitToString;

    QString toCapitalised(QString str);
}

#endif // UTILS_H
