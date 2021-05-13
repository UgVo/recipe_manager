QT       += core gui \
            sql \
            network 

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    c_mainwindow.cpp \
    ui/c_componentelemview.cpp \
    ui/c_componentview.cpp \
    ui/c_equipementsview.cpp \
    ui/c_image.cpp \
    ui/c_milestoneview.cpp \
    ui/c_notesdialog.cpp \
    ui/c_noteview.cpp \
    ui/c_processelemview.cpp \
    ui/c_processview.cpp \
    ui/c_stepview.cpp \
    ui/c_widget.cpp \
    utils/c_component.cpp \
    utils/c_dbmanager.cpp \
    utils/c_ingredient.cpp \
    utils/c_iomanager.cpp \
    utils/c_milestone.cpp \
    utils/c_note.cpp \
    utils/c_recipe.cpp \
    utils/c_step.cpp \
    utils/utils.cpp \
    utils/c_process.cpp

HEADERS += \
    c_mainwindow.h \
    ui/c_componentelemview.h \
    ui/c_componentview.h \
    ui/c_equipementsview.h \
    ui/c_image.h \
    ui/c_milestoneview.h \
    ui/c_notesdialog.h \
    ui/c_noteview.h \
    ui/c_processelemview.h \
    ui/c_processview.h \
    ui/c_stepview.h \
    ui/c_widget.h \
    utils/c_component.h \
    utils/c_dbmanager.h \
    utils/c_ingredient.h \
    utils/c_iomanager.h \
    utils/c_milestone.h \
    utils/c_note.h \
    utils/c_recipe.h \
    utils/c_step.h \
    utils/utils.h \
    utils/c_process.h

FORMS += \
    c_mainwindow.ui \
    ui/c_componentelemview.ui \
    ui/c_componentview.ui \
    ui/c_equipementsview.ui \
    ui/c_image.ui \
    ui/c_milestoneview.ui \
    ui/c_notesdialog.ui \
    ui/c_noteview.ui \
    ui/c_processelemview.ui \
    ui/c_processview.ui \
    ui/c_step_view.ui

TRANSLATIONS += \
    recipe_manager_fr_FR.ts

DESTDIR = $$PWD

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    rsc/rsc.qrc
