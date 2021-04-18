#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QList>
#include <QSet>

namespace recipe
{
    enum unit {g,L,dL,cL,mL,cuillere,pincee,none};
    extern QMap<int,QString> unitToString;

    enum modes{resume,display,edition};
    enum states{retracted,opened,transition,fixed};
}

#endif // UTILS_H
