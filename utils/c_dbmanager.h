#ifndef C_DBMANAGER_H
#define C_DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>
#include <QSqlError>
#include <utils/utils.h>
#include <utils/c_iomanager.h>
#include <utils/c_ingredient.h>
#include <utils/c_recipe.h>
#include <utils/c_component.h>
#include <utils/c_note.h>
#include <utils/c_step.h>
#include <utils/c_milestone.h>

class c_dbManager
{

public:
    c_dbManager(c_ioManager* ioManager);
    ~c_dbManager();

    static bool connect();
    static void initDatabase();
    static void clear();
    static int getLastInsertId(QString table, QSqlQuery query);
    static bool enablePragma(bool flag);

    static bool addElementType(QString type);
    static bool loadElementTypes();
    static bool removeElementType(QString type);
    static bool clearElementTypes();
    static QSet<QString> getElementTypes();

    static bool addProcessType(QString type);
    static bool loadProcessTypes();
    static bool removeProcessType(QString type);
    static bool clearProcessTypes();
    static QSet<QString> getProcessTypes();

    static bool addEquipment(QString equipment);
    static bool loadEquipments();
    static bool removeEquipment(QString equipment);
    static bool clearEquipments();
    static QSet<QString> getEquipments();

    static int addIngredient(c_ingredient &ingredient);
    static int updateIngredient(const c_ingredient &ingredient);
    static c_ingredient getIngredient(int id, bool cache = true);
    static QMap<int, c_ingredient> getIngredients();
    static QList<c_ingredient> getAllIngredients();
    static bool removeIngredient(int id);
    static bool clearIngredients();

    static int addRecipe(c_recipe &recipe);
    static int updateRecipe(c_recipe &recipe);
    static c_recipe getRecipe(int id);
    static bool removeRecipe(int id);
    static bool clearRecipes();

    static int addProcess(c_process &process, int idRecipe, int idStep);
    static int updateProcess(c_process &process);
    static QList<c_process> getRecipeProcesses(int id);
    static QList<c_process> getStepProcesses(int id);
    static bool removeProcess(int id);
    static bool clearProcesses();

    static int addMilestone(c_milestone &milestone, int idRecipe);
    static int updateMilestone(c_milestone &milestone);
    static QList<c_milestone> getMilestones(int id);
    static bool removeMilestone(int id);
    static bool clearMilestones();

    static int addNote(c_note &note, int idRecipe, int idStep);
    static int updateNote(c_note &note);
    static QList<c_note> getRecipeNotes(int id);
    static QList<c_note> getStepNotes(int id);
    static bool removeNote(int id);
    static bool clearNotes();


    static int addStep(c_step &step, int idMilestone);
    static int updateStep(c_step &step);
    static QList<c_step> getSteps(int id);
    static QList<QString> getEquipments(int id);
    static bool removeStep(int id);
    static bool clearSteps();

    static int addComponent(c_component &component, int idStep);
    static int updateComponent(c_component &component);
    static bool removeComponent(int id);
    static QList<c_component> getComponents(int id);
    static bool clearComponents();

private:
    static QSqlDatabase m_db;
    static bool instanciated;
    static QMap<QString,int> tables;
    static QMap<QString,QString> tablesCreate;
    static QMap<QString,QStringList> indexes;
    static QString type;

    static QSet<QString> elementTypes;
    static QSet<QString> processTypes;
    static QSet<QString> equipments;
    static QMap<int,c_ingredient> ingredients;
};

#endif // C_DBMANAGER_H
