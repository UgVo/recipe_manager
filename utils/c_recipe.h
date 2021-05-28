#ifndef C_RECIPE_H
#define C_RECIPE_H

#include <QObject>
#include "utils/utils.h"
#include "c_note.h"
#include "c_milestone.h"
#include "c_process.h"

class c_recipe : public QObject
{
    Q_OBJECT

public:
    c_recipe(int serving, QString name, QList<c_milestone> planning = QList<c_milestone>(),
                      QList<c_process> globalProcessing = QList<c_process>(),
                      QString imageUrl = "", QString settingUpImageUrl = "",
                      QList<c_note> notes = QList<c_note>(),
                      int id = -1);
    c_recipe(int serving, QString name, QList<c_milestone *> planning = QList<c_milestone *>(),
                      QList<c_process *> globalProcessing = QList<c_process *>(),
                      QString imageUrl = "", QString settingUpImageUrl = "",
                      QList<c_note *> notes = QList<c_note *>(),
                      int id = -1);
    c_recipe();
    c_recipe(c_recipe const &other);
    ~c_recipe() override;

    int getId() const;
    void setId(const int value);

    QString getImageUrl() const;
    void setImageUrl(const QString &value);

    QString getSettingUpImageUrl() const;
    void setSettingUpImageUrl(const QString &value);

    int getServings() const;
    void setServings(int value);

    QList<c_note> getNotes();
    QList<c_note> getNotes() const;
    QList<c_note *> getNotesPtr();
    void setNotes(const QList<c_note> &value);

    QList<c_milestone> getPlanning();
    QList<c_milestone> getPlanning() const;
    QList<c_milestone *> getPlanningPtr();
    void setPlanning(const QList<c_milestone> &value);
    QList<int> getMilestonesIds();

    QList<c_process> getGlobalProcessing();
    QList<c_process> getGlobalProcessing() const;
    QList<c_process *> getGlobalProcessingPtr();
    void setGlobalProcessing(const QList<c_process> &value);

    c_recipe& operator=(const c_recipe& other);
    bool operator==(const c_recipe& other) const;

    static c_recipe creatEmpty(int id = -1);
    bool isEmpty();

    bool isComplete() const;
    void setComplete(bool value);

    void completeRecipe();

    const QString &getName() const;
    void setName(const QString &newName);

signals:
    void nameChanged();

private:

    void completeProcessings();
    void completeNotes();
    void completeMilestones();

    int id;
    QString name;
    bool complete;
    QString imageUrl;
    QString settingUpImageUrl;
    int servings;
    QList<c_note *> notes;
    QList<c_milestone *> planning;
    QList<c_process *> globalProcessing;

    Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
};

#endif // C_RECIPE_H
