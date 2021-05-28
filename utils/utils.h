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
    extern QMap<unit,QString> unitToString;
    extern QMap<QString,QString> processToPixmapUrl;

    QString toCapitalised(QString str);

    template<typename T> bool compare(T *elem1,T *elem2) {
        return *elem1 < *elem2;
    };
}

#endif // UTILS_H
