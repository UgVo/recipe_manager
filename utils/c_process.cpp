#include "c_process.h"

c_process::c_process(QString _type, int _duration, int _temperature, int _id)
    : id(_id), type(_type.toLower()), duration(_duration), temperature(_temperature) {
}

bool c_process::operator==(const c_process &other) const {
    return (type == other.getType()
            && duration == other.duration
            && temperature == other.temperature);
}

bool c_process::operator<(const c_process &other) const {
    return id!=-1 ? id<other.getId() : false;
}

int c_process::getId() const {
    return id;
}

void c_process::setId(int value) {
    id = value;
}

QString c_process::getType() const {
    return type.left(1).toUpper()+type.mid(1);
}

void c_process::setType(const QString &value) {
    type = value.toLower();
}

int c_process::getDuration() const {
    return duration;
}

void c_process::setDuration(int value) {
    duration = value;
}

int c_process::getTemperature() const {
    return temperature;
}

void c_process::setTemperature(int value) {
    temperature = value;
}
