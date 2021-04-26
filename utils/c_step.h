#ifndef C_STEP_H
#define C_STEP_H

#include <QObject>
#include "utils/utils.h"
#include "utils/c_component.h"
#include "utils/c_note.h"
#include "utils/c_process.h"

class c_step
{

public:
    c_step(int rank = 0, QString description = QString(), QList<QString> imagesUrl = QList<QString>(),
            QList<QString> equipments = QList<QString>(), QList<c_component> components = QList<c_component>(),
            QList<c_note> notes = QList<c_note>(), QList<c_process> processings = QList<c_process>(), int id = -1 );
    ~c_step();
    c_step(const c_step &other);

    int getId() const;
    void setId(int value);

    int getRank() const;
    void setRank(int value);

    QList<c_component> getComponents() const;
    QList<c_component *> getComponentsPtr();
    void setComponents(const QList<c_component> &value);
    c_component *newComponent();
    void removeComponent(const c_component *value);

    QList<QString> getEquipments() const;
    void setEquipments(const QList<QString> &value);

    QString getDescription() const;
    void setDescription(const QString &value);

    QList<QString> getImagesUrl() const;
    QString getImagesUrlsStr() const;
    void setImagesUrl(const QList<QString> &value);
    void setImagesUrlStr(const QString &value);

    QList<c_note> getNotes() const;
    QList<c_note*> getNotesPtr();
    void setNotes(const QList<c_note> &value);
    c_note* addNote(const c_note value);
    void deleteNote(const c_note value);

    c_step &operator=(const c_step &other);
    bool operator==(const c_step &other) const;
    bool operator!=(const c_step &other) const;
    bool operator<(const c_step &other) const;

    QList<c_process> getProcessings() const;
    QList<c_process*> getProcessingsPtr();
    void setProcessings(const QList<c_process> &value);

    bool isCompleted() const;
    void setComplete(bool value);

    void completeStep();
private:

    void completeComponents();
    void completeNotes();
    void completeEquipements();

    int id;
    int rank;
    QList<c_component> components;
    QList<QString> equipments;
    QString description;
    QList<QString> imagesUrl;
    QList<c_note> notes;
    QList<c_process> processings;
    bool complete;
};

#endif // C_STEP_H
