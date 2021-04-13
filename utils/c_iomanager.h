#ifndef C_IOMANAGER_H
#define C_IOMANAGER_H

#include <QObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

class c_ioManager
{

public:
    static QString sqlite;
    static QString postgresql;
    static QString none;

    c_ioManager(QString file = "config.json");

    QString getPassword() const;
    QString getHost() const;
    int getPort() const;
    QString getDbName() const;
    QString getUser() const;
    QString getDbType() const;

private:
    static QVector<QString> dbTypes;

    QString password;
    QString host;
    int port;
    QString dbName;
    QString user;
    QString dbType;

signals:

};

#endif // C_IOMANAGER_H
