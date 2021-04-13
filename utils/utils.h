#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QList>
#include <QSet>

namespace recipe
{
    enum unit {g,L,cuillere,pincee,none};
    extern QMap<int,QString> unitToString;
}

#endif // UTILS_H
