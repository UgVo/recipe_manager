#include "c_iomanager.h"

QString c_ioManager::sqlite("sqlite");
QString c_ioManager::postgresql("postgresql");
QString c_ioManager::none("none");
QVector<QString> c_ioManager::dbTypes({sqlite,postgresql,none});

c_ioManager::c_ioManager(QString fileName) {
    QString val;
    QFile file;
    QJsonDocument doc;
    QJsonObject jObject_config;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    doc = QJsonDocument::fromJson(val.toUtf8());
    jObject_config = doc.object();

    dbType = dbTypes.contains(jObject_config.value("type").toString())?
                jObject_config.value("type").toString():none;
    if (!dbType.compare(postgresql)) {
        password = jObject_config.value("password").toString();
        host = jObject_config.value("host").toString();
        port = jObject_config.value("port").toInt();
        dbName = jObject_config.value("dbname").toString();
        user = jObject_config.value("user").toString();
        qDebug() << password << host << port << dbName << user;
    } else if (!dbType.compare(sqlite)) {
        password = "";
        host = "";
        port = 0;
        dbName = jObject_config.value("dbname").toString();
        user = "";
    } else {
        password = "";
        host = "";
        port = 0;
        dbName = "";
        user = "";
    }
}

QString c_ioManager::getPassword() const {
    return password;
}

QString c_ioManager::getHost() const {
    return host;
}

int c_ioManager::getPort() const {
    return port;
}

QString c_ioManager::getDbName() const {
    return dbName;
}

QString c_ioManager::getUser() const {
    return user;
}

QString c_ioManager::getDbType() const {
    return dbType;
}
