#include "c_test.h"

c_test::c_test() {
    iomanager = new c_ioManager("config_test.json");
    dbmanager = new c_dbManager(iomanager);
    dbmanager->initDatabase();
}

void c_test::types() {
    c_dbManager::clearElementTypes();
    c_dbManager::clearProcessTypes();
    c_dbManager::clearEquipments();

    c_dbManager::addElementType("pâte salée");
    c_dbManager::addElementType("Pâte sucrée");
    c_dbManager::addElementType("condiment");
    c_dbManager::addElementType("fruit");
    QSet<QString> set = QSet<QString>{"pâte salée","pâte sucrée","condiment","fruit"};
    QCOMPARE(set.size(),c_dbManager::getElementTypes().size());
    for (QSet<QString>::iterator it = set.begin(); it != set.end(); ++it) {
        QVERIFY(c_dbManager::getElementTypes().contains(*it));
    }

    c_dbManager::addProcessType("cuisson four");
    c_dbManager::addProcessType("cuisson poël");
    c_dbManager::addProcessType("Pétrissage");
    set = QSet<QString>{"cuisson four","cuisson poël","pétrissage"};
    QCOMPARE(set.size(),c_dbManager::getProcessTypes().size());
    for (QSet<QString>::iterator it = set.begin(); it != set.end(); ++it) {
        QVERIFY(c_dbManager::getProcessTypes().contains(*it));
    }

    c_dbManager::addEquipment("four");
    c_dbManager::addEquipment("maryse");
    c_dbManager::addEquipment("Batteur");
    set = QSet<QString>{"four","maryse","batteur"};
    QCOMPARE(set.size(),c_dbManager::getEquipments().size());
    for (QSet<QString>::iterator it = set.begin(); it != set.end(); ++it) {
        QVERIFY(c_dbManager::getEquipments().contains(*it));
    }

    c_dbManager::removeElementType("pâte sucrée");
    QVERIFY(!c_dbManager::getElementTypes().contains("pâte sucrée"));

    c_dbManager::removeProcessType("cuisson four");
    QVERIFY(!c_dbManager::getProcessTypes().contains("cuisson four"));

    c_dbManager::removeEquipment("maryse");
    QVERIFY(!c_dbManager::getEquipments().contains("maryse"));

    c_dbManager::clearElementTypes();
    c_dbManager::clearProcessTypes();
    c_dbManager::clearEquipments();

    c_dbManager::loadElementTypes();
    QVERIFY(c_dbManager::getElementTypes().isEmpty());
    c_dbManager::loadProcessTypes();
    QVERIFY(c_dbManager::getProcessTypes().isEmpty());
    c_dbManager::loadEquipments();
    QVERIFY(c_dbManager::getEquipments().isEmpty());
}

void c_test::ingredient() {
    c_dbManager::clearIngredients();

    // Constructor
    c_ingredient ingredient("crême","crême patissière");

    // Getters
    QCOMPARE("crême",ingredient.getType());
    QCOMPARE(-1,ingredient.getSubRecipe().getId());
    QCOMPARE("crême patissière",ingredient.getName());
    QCOMPARE(-1,ingredient.getId());

    // Setters
    ingredient.setId(2);
    QCOMPARE(2,ingredient.getId());
    ingredient.setType("condiment");
    QCOMPARE("condiment",ingredient.getType());
    ingredient.setName("sel");
    QCOMPARE("sel",ingredient.getName());

    ingredient.setId(-1);
    ingredient.registerIngredient();
    QVERIFY(ingredient.getId() != -1);
    c_ingredient retrieved_ingredient = c_dbManager::getIngredient(ingredient.getId(),false);
    QCOMPARE(ingredient,retrieved_ingredient);
    ingredient.setName("poivre");
    ingredient.registerIngredient();
    retrieved_ingredient = c_dbManager::getIngredient(ingredient.getId());
    QCOMPARE(ingredient,retrieved_ingredient);
    retrieved_ingredient = c_dbManager::getIngredient(ingredient.getId(),false);
    QCOMPARE(ingredient,retrieved_ingredient);
    QVERIFY(c_dbManager::getIngredients().contains(ingredient.getId()));
    QVERIFY(c_dbManager::getIngredients().size() == 1);
    QVERIFY(c_dbManager::getAllIngredients().contains(ingredient));
    QVERIFY(c_dbManager::getAllIngredients().size() == 1);
    c_dbManager::removeIngredient(ingredient.getId());
    QVERIFY(c_dbManager::getIngredients().isEmpty());
    QVERIFY(c_dbManager::getAllIngredients().isEmpty());
}

void c_test::component() {

    c_dbManager::clearComponents();
    // Constructor
    c_component c1(c_ingredient("sel"),1,recipe::unit::pincee);

    // Getters
    QCOMPARE(c_ingredient("sel"),c1.getIngredient());
    QCOMPARE(1,c1.getQuantity());
    QCOMPARE(-1,c1.getId());
    QCOMPARE(recipe::unit::pincee,c1.getUnit());
    QCOMPARE(c1.getIngredient(),c_ingredient("sel"));

    //Setters
    c1.setId(-2);
    QCOMPARE(-2,c1.getId());
    c1.setIngredient(c_ingredient("légume","poivron"));
    QCOMPARE(c_ingredient("légume","poivron"),c1.getIngredient());
    c1.setQuantity(5);
    QCOMPARE(5,c1.getQuantity());
    c1.setUnit(recipe::unit::L);
    QCOMPARE(recipe::unit::L,c1.getUnit());

    c1.setId(-1);
    c_dbManager::addComponent(c1,1);
    c_component c2 = c_dbManager::getComponents(1)[0];
    QCOMPARE(c1,c2);

    c1.setQuantity(10);
    c1.setIngredient(c_ingredient("légume","poirot"));
    c1.setUnit(recipe::unit::none);
    c_dbManager::addComponent(c1,1);
    c_component c3 = c_dbManager::getComponents(1)[0];
    QCOMPARE(c1,c3);

    c_dbManager::removeComponent(c1.getId());
    QVERIFY(c_dbManager::getComponents(1).empty());

    c_dbManager::clearComponents();
}

void c_test::step() {
    c_dbManager::clearSteps();
    c_step step = c_step(0,"Première étape",QList<QString>{"image1.png","image2.png"},
                         QList<QString>{"Maryse","bol"},
                         QList<c_component>{c_component(c_ingredient("Légume","poirot"),1),
                                            c_component(c_ingredient("Légume","tomate"),1)},
                         QList<c_note>());
    QCOMPARE(step.getDescription(),"Première étape");
    QCOMPARE(step.getEquipments(),(QList<QString>{"Maryse","bol"}));
    QCOMPARE(step.getComponents(),(QList<c_component>{c_component(c_ingredient("Légume","poirot"),1),
                                                     c_component(c_ingredient("Légume","tomate"),1)}));

    c_dbManager::addStep(step,1);

    c_step step2 = c_dbManager::getSteps(1)[0];
    step2.completeStep();
    QCOMPARE(step.getComponents(),step2.getComponents());
    QCOMPARE(step.getRank(),step2.getRank());
    QCOMPARE(step.getEquipments(),step2.getEquipments());
    QCOMPARE(step.getImagesUrl(),step2.getImagesUrl());
    QCOMPARE(step,step2);

    step.setRank(1);
    step.setDescription("Seconde étape");
    step.setComponents(QList<c_component>{c_component(c_ingredient("Légume","chou-fleur"),1),
                                          c_component(c_ingredient("Légume","Pomme de terre"),1)});
    step.setEquipments(QList<QString>{"hachoir","couteau de chef"});
    c_dbManager::updateStep(step);

    c_step step3 = c_dbManager::getSteps(1)[0];
    step3.completeStep();
    QCOMPARE(step,step3);

    c_dbManager::removeComponent(step.getId());
    QVERIFY(c_dbManager::getSteps(1).size());
    c_dbManager::clearSteps();

    c_dbManager::clearSteps();
}

void c_test::process() {
    c_dbManager::clearProcesses();

    c_process proc("Cuisson four",60,180);
    c_process proc1("repos",60);
    QVERIFY(proc.getId() == -1);
    QCOMPARE("cuisson four",proc.getType());
    QCOMPARE(60, proc.getDuration());
    QCOMPARE(180,proc.getTemperature());

    proc.setDuration(45);
    proc.setTemperature(220);
    proc.setType("Préparation");
    QCOMPARE("préparation",proc.getType());
    QCOMPARE(45, proc.getDuration());
    QCOMPARE(220,proc.getTemperature());

    c_dbManager::addProcess(proc,1,0);
    c_dbManager::addProcess(proc1,0,1);
    c_process proc2 = c_dbManager::getRecipeProcesses(1)[0];
    c_process proc3 = c_dbManager::getStepProcesses(1)[0];
    QCOMPARE(proc,proc2);
    QCOMPARE(proc1,proc3);

    proc.setDuration(60);
    proc.setTemperature(180);
    proc.setType("mijoter");
    c_dbManager::updateProcess(proc);
    proc2 = c_dbManager::getRecipeProcesses(1)[0];
    QCOMPARE(proc,proc2);

    c_dbManager::removeProcess(proc.getId());
    QVERIFY(c_dbManager::getRecipeProcesses(1).isEmpty());

    c_dbManager::clearProcesses();
}

void c_test::notes() {
    c_dbManager::clearNotes();

    QDateTime now = QDateTime::currentDateTime();
    c_note note("Super note 1",now);

    QCOMPARE(note.getDate(),now);
    QCOMPARE(note.getText(),"Super note 1");
    QVERIFY(note.getId() == -1);

    note.setText("Super note 2");
    QCOMPARE(note.getText(),"Super note 2");

    QThread::msleep(100);
    QDateTime now2 = QDateTime::currentDateTime();
    note.setDate(now2);
    QCOMPARE(note.getDate(),now2);
    QVERIFY(note.getDate() != now);

    c_dbManager::addNote(note,1,0);
    c_note note1("Super note 1");
    c_dbManager::addNote(note1,0,1);

    c_note note2 = c_dbManager::getRecipeNotes(1)[0];
    c_note note3 = c_dbManager::getStepNotes(1)[0];
    QCOMPARE(note,note2);
    QCOMPARE(note1,note3);

    c_dbManager::removeNote(note.getId());
    QVERIFY(c_dbManager::getRecipeNotes(1).isEmpty());
    c_dbManager::clearNotes();
}

void c_test::milestone() {
    c_dbManager::clearMilestones();
    c_dbManager::clearSteps();

    c_milestone milestone(1,QList<c_step>{
                                c_step(0,"step 1",QList<QString>(),QList<QString>{"cuillère","four","fouet"}),
                                c_step(1,"step 2")});
    QCOMPARE(milestone.getRank(),1);
    QCOMPARE(milestone.getSteps(),(QList<c_step>{
                                       c_step(0,"step 1",QList<QString>(),QList<QString>{"cuillère","four","fouet"}),
                                       c_step(1,"step 2")}));
    c_dbManager::addMilestone(milestone,1);
    QVERIFY(milestone.getId()!=0);

    c_milestone milestone2 = c_dbManager::getMilestones(1)[0];
    milestone2.completeMilestone();
    QCOMPARE(milestone,milestone2);

    milestone.setSteps(QList<c_step>{
                           c_step(0,"step 3",QList<QString>(),QList<QString>{"thermomètre","poche à douille","douille 8mm"}),
                           c_step(1,"step 4")});
    milestone.setRank(2);
    c_dbManager::updateMilestone(milestone);
    c_milestone milestone3 = c_dbManager::getMilestones(1)[0];
    milestone3.completeMilestone();
    QCOMPARE(milestone,milestone3);

    c_dbManager::clearMilestones();
    c_dbManager::clearSteps();
}

void c_test::recipe() {
    c_dbManager::clearRecipes();
    c_recipe recipe(10,QList<c_milestone>{c_milestone(0,QList<c_step>{c_step(0,"step 1"),c_step(1,"step2")}),
                                   c_milestone(1,QList<c_step>{c_step(0,"step 1"),c_step(1,"step 2")})},
             QList<c_process>{c_process("cuisson four",60,180),c_process("préparation",20)});
    c_dbManager::addRecipe(recipe);
    c_recipe recipe2 = c_dbManager::getRecipe(recipe.getId());
    recipe2.completeRecipe();
    QVERIFY(recipe2.getId()!=-1);
    QCOMPARE(recipe.getImageUrl(),recipe2.getImageUrl());
    QCOMPARE(recipe.getServings(),recipe2.getServings());
    QCOMPARE(recipe.getGlobalProcessing(),recipe2.getGlobalProcessing());
    QCOMPARE(recipe.getPlanning(),recipe2.getPlanning());
    QCOMPARE(recipe.getNotes(),recipe2.getNotes());
    QCOMPARE(recipe,recipe2);

    recipe.setGlobalProcessing(QList<c_process>{c_process("cuisson poël",60,180),c_process("repos",20)});
    QVERIFY(!recipe.getGlobalProcessing().isEmpty());
    recipe.setPlanning(QList<c_milestone>{c_milestone(0,QList<c_step>{c_step(0,"step 3"),c_step(1,"step 4")}),
                                          c_milestone(1,QList<c_step>{c_step(0,"step 3"),c_step(1,"step 4")}),
                                          c_milestone(2,QList<c_step>{c_step(0,"step 3"),c_step(1,"step 4")})});
    QVERIFY(!recipe.getPlanning().isEmpty());
    recipe.setServings(5);

    c_dbManager::enablePragma(true);

    c_dbManager::updateRecipe(recipe);
    c_recipe recipe3 = c_dbManager::getRecipe(recipe.getId());
    recipe3.completeRecipe();
    QVERIFY(recipe3.getId()!=-1);
    QVERIFY(!recipe3.getPlanning().isEmpty());
    QCOMPARE(recipe.getImageUrl(),recipe3.getImageUrl());
    QCOMPARE(recipe.getServings(),recipe3.getServings());
//    QList<c_milestone> planning = recipe.getPlanning();
//    qDebug() << "recipe";
//    for (auto it = planning.begin(); it != planning.end(); ++it) {
//        QList<c_step> steps = it->getSteps();
//        qDebug() << it->getRank() << it->getId();
//        for (auto it2 = steps.begin(); it2 != steps.end(); ++it2) {
//            qDebug() << it2->getDescription();
//        }
//    }
//    QList<c_milestone> planning3 = recipe3.getPlanning();
//    qDebug() << "recipe3";
//    for (auto it = planning3.begin(); it != planning3.end(); ++it) {
//        QList<c_step> steps = it->getSteps();
//        qDebug() << it->getRank() << it->getId();
//        for (auto it2 = steps.begin(); it2 != steps.end(); ++it2) {
//            qDebug() << it2->getDescription();
//        }
//    }
    QCOMPARE(recipe.getGlobalProcessing(),recipe3.getGlobalProcessing());
    QCOMPARE(recipe.getPlanning(),recipe3.getPlanning());
    QCOMPARE(recipe.getNotes(),recipe3.getNotes());
    QCOMPARE(recipe,recipe3);

    c_dbManager::removeRecipe(recipe.getId());

    QVERIFY(c_dbManager::getRecipe(recipe.getId()).isEmpty());

    c_dbManager::clearRecipes();

    c_dbManager::enablePragma(false);
}

QTEST_MAIN(c_test)
