#include "c_dbmanager.h"

bool c_dbManager::instanciated = false;
QSqlDatabase c_dbManager::m_db = QSqlDatabase();
QMap<QString,int> c_dbManager::tables {
    {"element_types",1},
    {"process_types",1},
    {"equipments",1},
    {"ingredients",4},
    {"processes",6},
    {"milestones",4},
    {"recipes",4},
    {"steps",5},
    {"components",5},
    {"notes",5},
    {"rel_step_equi",2}
};
QMap<QString,QString> c_dbManager::tablesCreate {
    {"element_types",
        "CREATE TABLE element_types ("
        "   name text, "
        "   CONSTRAINT PK_elementName PRIMARY KEY(name)"
        ")"},
    {"process_types",
        "CREATE TABLE process_types ( 	"
        "   name text, 	"
        "   CONSTRAINT PK_processName PRIMARY KEY(name)"
        ")"},
    {"equipments",
        "CREATE TABLE equipments ("
        "   name text, "
        "   CONSTRAINT PK_equipementName PRIMARY KEY(name)"
        ")"},
    {"ingredients",
        "CREATE TABLE ingredients ("
        "   id SERIAL UNIQUE,"
        "   type text,"
        "   name text,"
        "   recipe_id integer,"
        "   CONSTRAINT PK_ingredient PRIMARY KEY(id),"
        "   CONSTRAINT FK_typeName FOREIGN KEY (type) REFERENCES element_types (name) ON DELETE CASCADE"
        ")"},
    {"processes",
        "CREATE TABLE processes ("
        "   id SERIAL UNIQUE,"
        "   type text,"
        "   duration integer,"
        "   temperature integer,"
        "   recipe_id integer,"
        "   step_id integer,"
        "   CONSTRAINT PK_process PRIMARY KEY(id),"
        "   CONSTRAINT FK_processType FOREIGN KEY (type) REFERENCES process_types (name) ON DELETE CASCADE,"
        "   CONSTRAINT FK_recipeId FOREIGN KEY (recipe_id) REFERENCES recipes (id) ON DELETE CASCADE,"
        "   CONSTRAINT FK_stepId_processes FOREIGN KEY (step_id) REFERENCES steps (id) ON DELETE CASCADE"
        ")"},
    {"milestones",
        "CREATE TABLE milestones ("
        "   id SERIAL UNIQUE,"
        "   rank integer,"
        "   name text,"
        "   recipe_id integer,"
        "   CONSTRAINT PK_milestones PRIMARY KEY(id),"
        "   CONSTRAINT FK_recipe FOREIGN KEY(recipe_id) REFERENCES recipes(id) ON DELETE CASCADE"
        ")"},
    {"recipes",
        "CREATE TABLE recipes ("
        "   id SERIAL UNIQUE,"
        "   image_url text,"
        "   setting_up_image_url text,"
        "   servings integer,"
        "   CONSTRAINT PK_recipe PRIMARY KEY(id)"
        ")"},
    {"steps",
        "CREATE TABLE steps ("
        "   id SERIAL UNIQUE,"
        "   rank integer,"
        "   description text,"
        "   imagesurls text,"
        "   milestone_id integer,"
        "   CONSTRAINT PK_step PRIMARY KEY(id),"
        "   CONSTRAINT FK_milestone FOREIGN KEY (milestone_id) REFERENCES milestones(id) ON DELETE CASCADE"
        ")"},
    {"components",
        "CREATE TABLE components ("
        "   id SERIAL UNIQUE,"
        "   ingredient_id integer,"
        "   quantity integer,"
        "   unit integer,"
        "   step_id integer,"
        "   CONSTRAINT PK_component PRIMARY KEY(id),"
        "   CONSTRAINT FK_ingredientId FOREIGN KEY (ingredient_id) REFERENCES ingredients(id) ON DELETE CASCADE,"
        "   CONSTRAINT FK_stepId FOREIGN KEY (step_id) REFERENCES steps(id) ON DELETE CASCADE"
        ")"},
    {"notes",
        "CREATE TABLE notes ("
        "   id SERIAL UNIQUE,"
        "   text text,"
        "   date date,"
        "   recipe_id integer,"
        "   step_id integer,"
        "   CONSTRAINT PK_notes PRIMARY KEY(id)"
        "   CONSTRAINT FK_recipeId_notes FOREIGN KEY (recipe_id) REFERENCES recipes (id) ON DELETE CASCADE,"
        "   CONSTRAINT FK_stepId_notes FOREIGN KEY (step_id) REFERENCES steps (id) ON DELETE CASCADE"
        ")"},
    {"rel_step_equi",
        "CREATE TABLE rel_step_equi ("
        "   step_id integer,"
        "   equipment text,"
        "   CONSTRAINT PK_re_step_equi PRIMARY KEY (step_id,equipment),"
        "   CONSTRAINT FK_step_rel FOREIGN KEY (step_id) REFERENCES steps(id) ON DELETE CASCADE,"
        "   CONSTRAINT FK_equi_rel FOREIGN KEY (equipment) REFERENCES equipments(name) ON DELETE CASCADE"
        ")"}
};

QMap<QString,QStringList> c_dbManager::indexes {
    {"ingredients",
        {"CREATE INDEX pkid_ingredient ON ingredients (id)"}},
    {"processes",
        {"CREATE INDEX pkid_processes ON processes (id)",
        "CREATE INDEX fkrecipe_id_processes ON processes (recipe_id)",
        "CREATE INDEX fkstep_id_processes ON processes (step_id)"}},
    {"milestones",
        {"CREATE INDEX pkid_milestones ON milestones (id)",
        "CREATE INDEX fkrecipe_id_milstones ON milestones (recipe_id)"}},
    {"recipes",
        {"CREATE INDEX pkid_recipes ON recipes (id)"}},
    {"steps",
        {"CREATE INDEX fkmilestone_id ON steps (milestone_id)",
        "CREATE INDEX pkid_steps ON steps (id)"}},
    {"rel_step_equi",
        {"CREATE INDEX pkfkstep_id_rel ON rel_step_equi (step_id)",
        "CREATE INDEX pkfkequipement_rel ON rel_step_equi (equipment)"}},
    {"components",
        {"CREATE INDEX fkstep_id_component ON components (step_id)",
        "CREATE INDEX pkid_components ON components (id)"}},
    {"notes",
        {"CREATE INDEX pkid_notes ON notes (id)",
        "CREATE INDEX fkrecipe_id_notes ON notes (recipe_id)",
        "CREATE INDEX fkstep_id_notes ON notes (step_id)"}}
};

QString c_dbManager::type = "";
QSet<QString> c_dbManager::elementTypes;
QSet<QString> c_dbManager::processTypes;
QSet<QString> c_dbManager::equipments;
QMap<int,c_ingredient> c_dbManager::ingredients;

c_dbManager::c_dbManager(c_ioManager* ioManager) {
    type = ioManager->getDbType();
    if (!type.compare(c_ioManager::postgresql)) {
        m_db.close();
        m_db = QSqlDatabase::addDatabase("QPSQL");
        m_db.setHostName(ioManager->getHost());
        m_db.setPort(ioManager->getPort());
        m_db.setDatabaseName(ioManager->getDbName());
        m_db.setUserName(ioManager->getUser());
        m_db.setPassword(ioManager->getPassword());
        instanciated = true;
    }
    else if (!type.compare(c_ioManager::sqlite)) {
        m_db.close();
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setDatabaseName(ioManager->getDbName());
    }
}

c_dbManager::~c_dbManager() {
    QString database_name = m_db.connectionName();
    m_db.close();
    QSqlDatabase::removeDatabase(database_name);
}

bool c_dbManager::connect() {
    bool flag = m_db.open();
    qInfo() << "Openning database : " << flag;
    qDebug() << m_db.lastError();
    return flag;
}

void c_dbManager::initDatabase() {
    connect();
    QSqlQuery query(m_db);
    QStringList dbTables = m_db.tables();
    foreach (QString key, c_dbManager::tables.keys()) {
        if (!dbTables.contains(key)) {
            query.exec(c_dbManager::tablesCreate.value(key));
            foreach (QString indexes, c_dbManager::indexes[key]) {
                query.exec(indexes);
            }
        }
    }
}

void c_dbManager::clear() {
    QList<QString> keys = c_dbManager::tables.keys();
    QSqlQuery query(m_db);
    for (QList<QString>::iterator it = keys.begin(); it != keys.end(); ++it) {
        query.exec(QString("DELETE FROM %1").arg(*it));
    }
}

int c_dbManager::getLastInsertId(QString table, QSqlQuery query) {
    if (!type.compare(c_ioManager::postgresql)) {
        query.prepare("SELECT LASTVAL()");
    }
    else if (!type.compare(c_ioManager::sqlite)) {
        query.prepare("SELECT last_insert_rowid()");
    }
    if (query.exec()) {
        if (!type.compare(c_ioManager::postgresql)) {
            int id = query.record().indexOf("lastval");
            while (query.next()) {
                return query.value(id).toInt();
            }
        }
        else if (!type.compare(c_ioManager::sqlite)) {
            int id = query.record().indexOf("last_insert_rowid()");
            while (query.next()) {
                int rowid = query.value(id).toInt();
                query.exec(QString("UPDATE %1 SET id=%2 WHERE ROWID = %2").arg(table).arg(rowid));
                return rowid;
            }
        }
    }
    return 0;
}

bool c_dbManager::enablePragma(bool flag) {
    QSqlQuery query;
    if (flag) {
       return query.exec("PRAGMA foreign_keys = ON");
    } else {
       return query.exec("PRAGMA foreign_keys = OFF");
    }
}

//************* ElementTypes *************
bool c_dbManager::addElementType(QString type) {
    if (!elementTypes.contains(type.toLower())) {
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO element_types (name) VALUES (:name)");
        query.bindValue(":name",type.toLower());
        bool flag = query.exec();
        if (flag) {
            elementTypes.insert(type.toLower());
            return true;
        }
    }
    return false;
}

bool c_dbManager::loadElementTypes() {
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM element_types");
    if (query.exec()) {
        int id = query.record().indexOf("name");
        while (query.next()) {
            elementTypes.insert(query.value(id).toString());
        }
        return true;
    }
    return false;
}

bool c_dbManager::removeElementType(QString type) {
    QSqlQuery query(m_db);
    elementTypes.remove(type.toLower());
    query.prepare("DELETE FROM element_types WHERE name = :name");
    query.bindValue(":name",type.toLower());
    return query.exec();
}

bool c_dbManager::clearElementTypes() {
    QSqlQuery query(m_db);
    elementTypes.clear();
    query.prepare("DELETE FROM element_types");
    return query.exec();
}

QSet<QString> c_dbManager::getElementTypes() {
    if (elementTypes.isEmpty()) {
        loadElementTypes();
    }
    return elementTypes;
}

//************* ProcessTypes *************
bool c_dbManager::addProcessType(QString type) {
    if(!processTypes.contains(type.toLower())) {
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO process_types (name) VALUES (:name)");
        query.bindValue(":name",type.toLower());
        bool flag = query.exec();
        if (flag) {
            processTypes.insert(type.toLower());
            return true;
        }
    }
    return false;
}

bool c_dbManager::loadProcessTypes() {
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM process_types");
    if (query.exec()) {
        int id = query.record().indexOf("name");
        while (query.next()) {
            processTypes.insert(recipe::toCapitalised(query.value(id).toString()));
        }
        return true;
    }
    return false;
}

bool c_dbManager::removeProcessType(QString type) {
    QSqlQuery query(m_db);
    processTypes.remove(type.toLower());
    query.prepare("DELETE FROM process_types WHERE name = :name");
    query.bindValue(":name",type.toLower());
    return query.exec();
}

QSet<QString> c_dbManager::getProcessTypes() {
    if (processTypes.isEmpty()) {
        loadProcessTypes();
    }
    return processTypes;
}

bool c_dbManager::clearProcessTypes() {
    QSqlQuery query(m_db);
    processTypes.clear();
    query.prepare("DELETE FROM process_types");
    return query.exec();
}

//************* Equipements *************
bool c_dbManager::addEquipment(QString equipment) {
    if(!equipments.contains(equipment.toLower())) {
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO equipments (name) VALUES (:name)");
        query.bindValue(":name",equipment.toLower());
        bool flag = query.exec();
        if (flag) {
            equipments.insert(equipment.toLower());
            return true;
        }
    }
    return false;
}

bool c_dbManager::loadEquipments() {
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM equipments");
    if (query.exec()) {
        int id = query.record().indexOf("name");
        while (query.next()) {
            equipments.insert(recipe::toCapitalised(query.value(id).toString()));
        }
        return true;
    }
    return false;
}

bool c_dbManager::removeEquipment(QString equipment) {
    QSqlQuery query(m_db);
    equipments.remove(equipment.toLower());
    query.prepare("DELETE FROM equipments WHERE name = :name");
    query.bindValue(":name",equipment);
    return query.exec();
}

bool c_dbManager::clearEquipments() {
    QSqlQuery query(m_db);
    equipments.clear();
    query.prepare("DELETE FROM equipments");
    return query.exec();
}

QSet<QString> c_dbManager::getEquipments() {
    if (equipments.isEmpty()) {
        loadEquipments();
    }
    return equipments;
}

//************* Ingredients *************
int c_dbManager::addIngredient(c_ingredient &ingredient) {
    if (ingredients.key(ingredient,-1) != -1) {
        return  ingredients.key(ingredient);
    }
    if (!ingredients.contains(ingredient.getId()) || ingredient.getId() == -1) {
        if (!ingredient.getSubRecipe().isEmpty()) {
            addRecipe(ingredient.getSubRecipe());
        }
        QSqlQuery query(m_db);
        query.prepare("INSERT INTO ingredients (type,name,recipe_id) VALUES (:type,:name,:recipe_id) ");
        query.bindValue(":type",ingredient.getType());
        query.bindValue(":name",ingredient.getName());
        query.bindValue(":recipe_id",ingredient.getSubRecipe().getId());
        if (query.exec()) {
            int id = getLastInsertId("ingredients",query);
            if (ingredients.contains(ingredient.getId())) {
                ingredients[id] = ingredient;
            } else {
                ingredients.insert(id,ingredient);
            }
            ingredients[id].setId(id);
            if (!elementTypes.contains(ingredient.getType())) {
                addElementType(ingredient.getType());
            }
            return id;
        } else {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
    }
    return ingredient.getId();
}

int c_dbManager::updateIngredient(const c_ingredient &ingredient) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE ingredients "
                  "SET type = :type, recipe_id = :recipe_id, name = :name "
                  "WHERE id = :id ");
    query.bindValue(":type",ingredient.getType());
    query.bindValue(":recipe_id",ingredient.getSubRecipe().getId());
    query.bindValue(":name",ingredient.getName());
    query.bindValue(":id",ingredient.getId());
    if (query.exec()) {
        ingredients[ingredient.getId()] = ingredient;
        return ingredient.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return 0;
}

c_ingredient c_dbManager::getIngredient(int id, bool cache) {
    if (ingredients.contains(id) && cache) {
        return ingredients[id];
    }
    c_ingredient res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM ingredients WHERE id = :id ");
    query.bindValue(":id",id);
    if (query.exec()) {
        int id = query.record().indexOf("id");
        int idName = query.record().indexOf("name");
        int idType = query.record().indexOf("type");
        int idRecipeId = query.record().indexOf("recipe_id");
        while (query.next()) {
            res.setId(query.value(id).toInt());
            res.setType(query.value(idType).toString());
            res.setName(query.value(idName).toString());
            int recipeId = query.value(idRecipeId).toInt();
            if (recipeId==-1) {
                res.setSubRecipe(c_recipe());
            } else {
                res.setSubRecipe(getRecipe(recipeId));
            }
            ingredients.insert(query.value(id).toInt(),res);
            return res;
        }
    }
    return res;
}

QMap<int, c_ingredient> c_dbManager::getIngredients() {
    return ingredients;
}

QList<c_ingredient> c_dbManager::getAllIngredients() {
    QList<c_ingredient> res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM ingredients");
    if (query.exec()) {
        int id = query.record().indexOf("id");
        int idType = query.record().indexOf("type");
        int idName = query.record().indexOf("name");
        int idRecipeId = query.record().indexOf("recipe_id");
        while (query.next()) {
            res.push_back(c_ingredient(query.value(idType).toString(),query.value(idName).toString(),c_recipe::creatEmpty(query.value(idRecipeId).toInt()),query.value(id).toInt()));
        }
    }
    return res;
}

bool c_dbManager::removeIngredient(int id) {
    QSqlQuery query(m_db);
    ingredients.remove(id);
    query.prepare("DELETE FROM ingredients WHERE id = :id ");
    query.bindValue(":id",id);
    return query.exec();
}

bool c_dbManager::clearIngredients() {
    QSqlQuery query(m_db);
    ingredients.clear();
    query.prepare("DELETE FROM ingredients");
    return query.exec();
}

//************* Recipes *************
int c_dbManager::addRecipe(c_recipe &recipe) {
    if (recipe.getId() != -1) {
        if (updateRecipe(recipe) == -1) {
            qDebug() << "[addRecipe] error in updating recipe";
            return -1;
        } else {
            return recipe.getId();
        }
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO recipes (image_url,setting_up_image_url,servings) "
                  "VALUES (:image_url,:setting_up_image_url,:servings)");
    query.bindValue(":image_url", recipe.getImageUrl());
    query.bindValue(":setting_up_image_url",recipe.getSettingUpImageUrl());
    query.bindValue(":servings",recipe.getServings());
    if (query.exec()) {
        recipe.setId(getLastInsertId("recipes",query));
        QList<c_note> notes = recipe.getNotes();
        for (int i = 0; i < notes.size(); ++i) {
            addNote(notes[i],recipe.getId(),0);
        }
        QList<c_milestone> planning = recipe.getPlanning();
        for (int i = 0; i < planning.size(); ++i) {
            addMilestone(planning[i],recipe.getId());
        }
        QList<c_process> processings = recipe.getGlobalProcessing();
        for (int i = 0; i < processings.size(); ++i) {
            addProcess(processings[i],recipe.getId(),0);
        }
        return recipe.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return -1;
}

int c_dbManager::updateRecipe(c_recipe &recipe) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE recipes "
                  "SET image_url = :image_url, setting_up_image_url = :setting_up_image_url, servings = :servings "
                  "WHERE id = :id");
    query.bindValue(":image_url", recipe.getImageUrl());
    query.bindValue(":setting_up_image_url",recipe.getSettingUpImageUrl());
    query.bindValue(":servings",recipe.getServings());
    query.bindValue(":id",recipe.getId());
    if (query.exec()) {
        if (query.exec(QString("DELETE FROM notes WHERE recipe_id = %1").arg(recipe.getId()))) {
            QList<c_note> notes = recipe.getNotes();
            for (int i = 0; i < notes.size(); ++i) {
                addNote(notes[i],recipe.getId(),0);
            }
        } else {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
        if (query.exec(QString("DELETE FROM processes WHERE recipe_id = %1").arg(recipe.getId()))) {
            QList<c_process> processings = recipe.getGlobalProcessing();
            for (int i = 0; i < processings.size(); ++i) {
                addProcess(processings[i],recipe.getId(),0);
            }
        } else {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
        QList<int> milestonesIds = recipe.getMilestonesIds();
        QString queryStr = "DELETE FROM milestones WHERE recipe_id = :id AND";
        for (int i = 0; i < milestonesIds.size(); ++i) {
            queryStr.append(QString(" id != %1 AND").arg(milestonesIds[i]));
        }
        queryStr.remove(QRegularExpression("AND$"));
        query.prepare(queryStr);
        query.bindValue(":id",recipe.getId());
        if (query.exec()) {
            QList<c_milestone> planning = recipe.getPlanning();
            for (int i = 0; i < planning.size(); ++i) {
                addMilestone(planning[i],recipe.getId());
            }
        } else {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
        return recipe.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return -1;
}

c_recipe c_dbManager::getRecipe(int id) {
    c_recipe res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * from recipes WHERE id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        int idImageUrl = query.record().indexOf("image_url");
        int idSettingUp = query.record().indexOf("setting_up_image_url");
        int idServings = query.record().indexOf("servings");
        while (query.next()) {
            res.setId(id);
            res.setImageUrl(query.value(idImageUrl).toString());
            res.setSettingUpImageUrl(query.value(idSettingUp).toString());
            res.setServings(query.value(idServings).toInt());
            res.setComplete(false);
        }
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return res;
}

bool c_dbManager::removeRecipe(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM recipes WHERE id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        return true;
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return false;
}

bool c_dbManager::clearRecipes() {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM recipes");
    if (query.exec()) {
        return true;
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return false;
}

//************* Processes *************
int c_dbManager::addProcess(c_process &process, int idRecipe, int idStep) {
    if (process.getId() != -1) {
        if (updateProcess(process) == -1) {
            qDebug() << "[addProcess] error in updating process";
            return -1;
        } else {
            return process.getId();
        }
    }
    addProcessType(process.getType());
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO processes (type,duration,temperature,recipe_id,step_id) "
                  "VALUES (:type,:duration,:temperature,:recipe_id,:step_id)");
    query.bindValue(":type",process.getType().toLower());
    query.bindValue(":duration",process.getDuration());
    query.bindValue(":temperature",process.getTemperature());
    query.bindValue(":recipe_id",idRecipe?idRecipe:QMetaType::Int);
    query.bindValue(":step_id",idStep?idStep:QMetaType::Int);
    if (query.exec()) {
        process.setId(getLastInsertId("processes",query));
        return process.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return -1;
}

int c_dbManager::updateProcess(c_process &process) {
    if (process.getId() == -1) {
        return -1;
    }
    addProcessType(process.getType());
    QSqlQuery query(m_db);
    query.prepare("UPDATE processes "
                  "SET type = :type, duration = :duration, temperature = :temperature "
                  "WHERE id = :id");
    query.bindValue(":type",process.getType().toLower());
    query.bindValue(":duration",process.getDuration());
    query.bindValue(":temperature",process.getTemperature());
    query.bindValue(":id",process.getId());
    if (query.exec()) {
        return process.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return -1;
}

QList<c_process> c_dbManager::getRecipeProcesses(int id) {
    QList<c_process> res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM processes WHERE recipe_id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        int id = query.record().indexOf("id");
        int idType = query.record().indexOf("type");
        int idDuration = query.record().indexOf("duration");
        int idTemperature = query.record().indexOf("temperature");
        while (query.next()) {
            res.push_back(c_process(query.value(idType).toString(),
                                    query.value(idDuration).toInt(),
                                    query.value(idTemperature).toInt(),
                                    query.value(id).toInt()));
        }
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return res;
}

QList<c_process> c_dbManager::getStepProcesses(int id) {
    QList<c_process> res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM processes WHERE step_id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        int id = query.record().indexOf("id");
        int idType = query.record().indexOf("type");
        int idDuration = query.record().indexOf("duration");
        int idTemperature = query.record().indexOf("temperature");
        while (query.next()) {
            res.push_back(c_process(query.value(idType).toString(),
                                    query.value(idDuration).toInt(),
                                    query.value(idTemperature).toInt(),
                                    query.value(id).toInt()));
        }
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return res;
}

bool c_dbManager::removeProcess(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM processes WHERE id = :id");
    query.bindValue(":id",id);
    return query.exec();
}

bool c_dbManager::clearProcesses() {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM processes");
    return query.exec();
}

//************* Milestones *************
int c_dbManager::addMilestone(c_milestone &milestone, int idRecipe) {
    if (milestone.getId() != -1) {
        if (updateMilestone(milestone) == -1) {
            qDebug() << "[addMilestone] error in updating milestone";
            return -1;
        } else {
            return milestone.getId();
        }
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO milestones (rank,recipe_id,name) "
                  "VALUES (:rank,:recipe_id,:name)");
    query.bindValue(":rank",milestone.getRank());
    query.bindValue(":recipe_id",idRecipe);
    query.bindValue(":name",milestone.getName());
    if (query.exec()) {
        milestone.setId(getLastInsertId("milestones",query));
        QList<c_step> steps = milestone.getSteps();
        for (int i = 0; i < steps.size(); ++i) {
            addStep(steps[i],milestone.getId());
        }
        return milestone.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return -1;
}

int c_dbManager::updateMilestone(c_milestone &milestone) {
    if (milestone.getId() == -1) {
        return -1;
    }
    QSqlQuery query(m_db);
    query.prepare("UPDATE milestones "
                  "SET rank = :rank, name = :name "
                  "WHERE id = :id");
    query.bindValue(":rank",milestone.getRank());
    query.bindValue(":id",milestone.getId());
    query.bindValue(":name",milestone.getName());
    if (query.exec()) {
        QList<int> stepsIds = milestone.getStepsIds();
        QString queryStr = "DELETE FROM steps WHERE milestone_id = :id AND ";
        for (int i = 0; i < stepsIds.size(); ++i) {
            queryStr.append(QString(" id != %1 AND").arg(stepsIds[i]));
        }
        queryStr.remove(QRegularExpression("AND$"));
        query.prepare(queryStr);
        query.bindValue(":id",milestone.getId());
        if (query.exec()) {
            QList<c_step> steps = milestone.getSteps();
            for (int i = 0; i < steps.size(); ++i) {
                addStep(steps[i],milestone.getId());
            }
        } else {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
        return milestone.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return -1;
}

QList<c_milestone> c_dbManager::getMilestones(int id) {
    QList<c_milestone> res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM milestones WHERE recipe_id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        int idRank = query.record().indexOf("rank");
        int id = query.record().indexOf("id");
        int idName = query.record().indexOf("name");
        while (query.next()) {
            res.push_back(c_milestone(query.value(idRank).toInt(),
                                      QList<c_step>(),
                                      query.value(idName).toString(),
                                      query.value(id).toInt()));
            res.last().setComplete(false);
        }
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return res;
}

bool c_dbManager::removeMilestone(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM milestones WHERE id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        return true;
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return false;
}

bool c_dbManager::clearMilestones() {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM milestones");
    if (query.exec()) {
        return true;
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return false;
}

//************* Notes *************
int c_dbManager::addNote(c_note &note, int idRecipe, int idStep) {
    if (note.getId() != -1) {
        if (updateNote(note) == -1) {
            qDebug() << "[addNote] error in updating note";
            return -1;
        } else {
            return note.getId();
        }
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO notes (text,date,recipe_id,step_id) "
                  "VALUES (:text,:date,:recipe_id,:step_id)");
    query.bindValue(":text",note.getText());
    query.bindValue(":date",note.getDate());
    query.bindValue(":recipe_id",idRecipe?idRecipe:QMetaType::Int);
    query.bindValue(":step_id",idStep?idStep:QMetaType::Int);
    if (query.exec()) {
        note.setId(getLastInsertId("notes",query));
        return note.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return -1;
}

int c_dbManager::updateNote(c_note &note) {
    QSqlQuery query(m_db);
    query.prepare("UPDATE notes "
                  "SET text = :text, date = :date "
                  "WHERE id = :id");
    query.bindValue(":text",note.getText());
    query.bindValue(":date",note.getDate());
    query.bindValue(":id",note.getId());
    if (query.exec()) {
        return note.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return -1;
}

QList<c_note> c_dbManager::getRecipeNotes(int id) {
    QList<c_note> res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM notes WHERE recipe_id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        int id = query.record().indexOf("id");
        int idText = query.record().indexOf("text");
        int idDate = query.record().indexOf("date");
        while (query.next()) {
            res.push_back(c_note(query.value(idText).toString(),
                                 query.value(idDate).toDateTime(),
                                 query.value(id).toInt()));
        }
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return res;
}

QList<c_note> c_dbManager::getStepNotes(int id) {
    QList<c_note> res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM notes WHERE step_id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        int id = query.record().indexOf("id");
        int idText = query.record().indexOf("text");
        int idDate = query.record().indexOf("date");
        while (query.next()) {
            res.push_back(c_note(query.value(idText).toString(),
                                 query.value(idDate).toDateTime(),
                                 query.value(id).toInt()));
        }
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return res;
}

bool c_dbManager::removeNote(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM notes WHERE id = :id");
    query.bindValue(":id",id);
    return query.exec();
}

bool c_dbManager::clearNotes() {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM notes");
    return query.exec();
}

//************* Steps *************
int c_dbManager::addStep(c_step &step, int idMilestone) {
    if (step.getId() != -1) {
        if (updateStep(step) == -1) {
            qDebug() << "[addStep] error in updating step";
            return -1;
        } else {
            return step.getId();
        }
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO steps (rank, description, imagesurls, milestone_id) "
                  "VALUES (:rank, :description, :imagesurls, :milestone_id)");
    query.bindValue(":rank",step.getRank());
    query.bindValue(":description",step.getDescription());
    query.bindValue(":imagesurls",step.getImagesUrlsStr());
    query.bindValue(":milestone_id",idMilestone);
    if (query.exec()) {
        step.setId(getLastInsertId("steps",query));
        QList<c_component> components = step.getComponents();
        for (int i = 0; i < components.size(); ++i) {
            addComponent(components[i],step.getId());
        }
        QList<c_note> notes = step.getNotes();
        for (int i = 0; i < notes.size(); ++i) {
            addNote(notes[i],0,step.getId());
        }
        QList<c_process> processes = step.getProcessings();
        for (int i = 0; i < notes.size(); ++i) {
            addProcess(processes[i],0,step.getId());
        }
        QList<QString> equipements = step.getEquipments();
        for (int i = 0; i < equipements.size(); ++i) {
            addEquipment(equipements[i]);
            query.prepare("INSERT INTO rel_step_equi (step_id,equipment) VALUES (:step_id,:equipement)");
            query.bindValue(":step_id",step.getId());
            query.bindValue(":equipement",equipements[i]);
            if (!query.exec()) {
                qDebug() << query.lastError();
                qDebug() << query.lastQuery();
            }
        }
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return step.getId();
}

int c_dbManager::updateStep(c_step& step) {
    QSqlQuery query(m_db);
    if (step.getId() == -1) {
        return -1;
    }
    query.prepare("UPDATE steps "
                  "SET rank = :rank, description = :description, imagesurls = :imagesurls "
                  "WHERE id = :id");
    query.bindValue(":rank",step.getRank());
    query.bindValue(":description",step.getDescription());
    query.bindValue(":imagesurls",step.getImagesUrlsStr());
    query.bindValue(":id",step.getId());
    if (query.exec()) {
        if (query.exec(QString("DELETE FROM components WHERE step_id = %1").arg(step.getId()))) {
            QList<c_component> components = step.getComponents();
            for (int i = 0; i < components.size(); ++i) {
                addComponent(components[i],step.getId());
            }
        }  else {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
        if (query.exec(QString("DELETE FROM notes WHERE step_id = %1").arg(step.getId()))) {
            QList<c_note> notes = step.getNotes();
            for (int i = 0; i < notes.size(); ++i) {
                addNote(notes[i],0,step.getId());
            }
        } else {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }
        if (query.exec(QString("DELETE FROM processes WHERE step_id = %1").arg(step.getId()))) {
            QList<c_process> processes = step.getProcessings();
            for (int i = 0; i < processes.size(); ++i) {
                addProcess(processes[i],0,step.getId());
            }
        }
        if (query.exec(QString("DELETE FROM rel_step_equi WHERE step_id = %1").arg(step.getId()))) {
            QList<QString> equipements = step.getEquipments();
            for (int i = 0; i < equipements.size(); ++i) {
                addEquipment(equipements[i]);
                query.prepare("INSERT INTO rel_step_equi (step_id,equipment) VALUES (:step_id,:equipement)");
                query.bindValue(":step_id",step.getId());
                query.bindValue(":equipement",equipements[i]);
                if (!query.exec()) {
                    qDebug() << query.lastError();
                    qDebug() << query.lastQuery();
                }
            }
        } else {
            qDebug() << query.lastError();
            qDebug() << query.lastQuery();
        }

    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return step.getId();
}

QList<c_step> c_dbManager::getSteps(int id) {
    QList<c_step> res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM steps WHERE milestone_id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        int idId = query.record().indexOf("id");
        int idRank = query.record().indexOf("rank");
        int idDescription = query.record().indexOf("description");
        int idImages = query.record().indexOf("imagesurls");
        while (query.next()) {
            res.push_back(c_step(query.value(idRank).toInt(),
                                 query.value(idDescription).toString()));
            res.last().setId(query.value(idId).toInt());
            res.last().setImagesUrlStr(query.value(idImages).toString());
            res.last().setComplete(false);
        }
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return res;
}

QList<QString> c_dbManager::getEquipments(int id) {
    QList<QString> res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM rel_step_equi WHERE step_id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        int idName = query.record().indexOf("equipment");
        while (query.next()) {
            res.push_back(query.value(idName).toString());
        }
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return res;
}

bool c_dbManager::removeStep(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM steps WHERE id = :id");
    query.bindValue(":id",id);
    return query.exec();
}

bool c_dbManager::clearSteps() {
    QSqlQuery query(m_db);
    if (query.exec("DELETE FROM rel_step_equi")) {
        return query.exec("DELETE FROM steps");
    }
    return false;
}

//************* Components *************
int c_dbManager::addComponent(c_component &component, int idStep) {
    if (component.getIngredient().getId() == -1) {
        component.getIngredient().registerIngredient();
    }
    if (component.getId() != -1) {
        if (updateComponent(component) == -1) {
            qDebug() << "[addComponent] error in updating component";
            return -1;
        } else {
            return component.getId();
        }
    }
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO components (ingredient_id,quantity,unit,step_id) "
                  "VALUES (:ingredient_id,:quantity,:unit,:step_id) ");
    query.bindValue(":ingredient_id",component.getIngredient().getId());
    query.bindValue(":quantity",component.getQuantity());
    query.bindValue(":unit",component.getUnit());
    query.bindValue(":step_id",idStep);
    if (query.exec()) {
        component.setId(getLastInsertId("components",query));
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return component.getId();
}

int c_dbManager::updateComponent(c_component &component) {
    if (component.getIngredient().registerIngredient() == -1) {
        qDebug() << "[Component Update][" << component.getId() << "]error while updating or adding ingredient : "
                 << component.getIngredient().getName();
        return -1;
    }
    QSqlQuery query(m_db);
    query.prepare("UPDATE components "
                  "SET ingredient_id = :ingredient_id ,quantity = :quantity ,unit = :unit "
                  "WHERE id = :id");
    query.bindValue(":ingredient_id",component.getIngredient().getId());
    query.bindValue(":quantity",component.getQuantity());
    query.bindValue(":unit",component.getUnit());
    query.bindValue(":id",component.getId());
    if (query.exec()) {
        return component.getId();
    } else {
        qDebug() << query.lastError();
        qDebug() << query.lastQuery();
    }
    return -1;
}

bool c_dbManager::removeComponent(int id) {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM components WHERE id = :id ");
    query.bindValue(":id",id);
    return query.exec();
}

QList<c_component> c_dbManager::getComponents(int id) {
    QList<c_component> res;
    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM components WHERE step_id = :id");
    query.bindValue(":id",id);
    if (query.exec()) {
        int id = query.record().indexOf("id");
        int idIngredient = query.record().indexOf("ingredient_id");
        int idQuantity = query.record().indexOf("quantity");
        int idUnit = query.record().indexOf("unit");
        while (query.next()) {
            res.push_back(c_component(c_ingredient(),query.value(idQuantity).toInt(),
                                      recipe::unit(query.value(idUnit).toInt()),query.value(id).toInt()));
            int ingredientId = query.value(idIngredient).toInt();
            if (!ingredients.contains(ingredientId)) {
                qDebug() << "ingredient missing in cache, loading from database...";
                c_ingredient ingredient = getIngredient(ingredientId);
                if (ingredient.getId() == ingredientId) {
                    ingredients.insert(ingredientId,ingredient);
                    qDebug() << "loaded from database : id|" << ingredientId << " name|" << ingredient.getName();
                    res.last().setIngredient(ingredient);
                } else {
                    qDebug() << "error loading ingredient";
                }
            } else {
                res.last().setIngredient(ingredients[ingredientId]);
            }
        }
    }
    return res;
}

bool c_dbManager::clearComponents() {
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM components");
    return query.exec();
}
