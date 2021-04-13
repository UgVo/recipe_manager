#ifndef C_PROCESS_H
#define C_PROCESS_H

#include <QString>
class c_process
{
public:
    c_process(QString type = "none", int duration = 0, int temperature = 0, int id = -1);

    bool operator==(const c_process &other) const;
    bool operator<(const c_process &other) const;

    int getId() const;
    void setId(int value);

    QString getType() const;
    void setType(const QString &value);

    int getDuration() const;
    void setDuration(int value);

    int getTemperature() const;
    void setTemperature(int value);

private:
    int id;
    QString type;
    int duration;
    int temperature;
};

#endif // C_PROCESS_H
