cmake_minimum_required(VERSION 3.5)

project(recipemanager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Sql REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        c_mainwindow.cpp
        ui/c_componentelemview.cpp
        ui/c_componentview.cpp
        ui/c_equipementsview.cpp
        ui/c_image.cpp
        ui/c_milestoneview.cpp
        ui/c_notesdialog.cpp
        ui/c_noteview.cpp
        ui/c_processelemview.cpp
        ui/c_processview.cpp
        ui/c_stepview.cpp
        ui/c_widget.cpp
        utils/c_component.cpp
        utils/c_dbmanager.cpp
        utils/c_ingredient.cpp
        utils/c_iomanager.cpp
        utils/c_milestone.cpp
        utils/c_note.cpp
        utils/c_recipe.cpp
        utils/c_step.cpp
        utils/utils.cpp
        utils/c_process.cpp
)

set(PROJECT_HEADERS
    c_mainwindow.h
    ui/c_componentelemview.h
    ui/c_componentview.h
    ui/c_equipementsview.h
    ui/c_image.h
    ui/c_milestoneview.h
    ui/c_notesdialog.h
    ui/c_noteview.h
    ui/c_processelemview.h
    ui/c_processview.h
    ui/c_stepview.h
    ui/c_widget.h
    utils/c_component.h
    utils/c_dbmanager.h
    utils/c_ingredient.h
    utils/c_iomanager.h
    utils/c_milestone.h
    utils/c_note.h
    utils/c_recipe.h
    utils/c_step.h
    utils/utils.h
    utils/c_process.h
)

set(PROJECT_FORM
    c_mainwindow.ui
    ui/c_componentelemview.ui
    ui/c_componentview.ui
    ui/c_equipementsview.ui
    ui/c_image.ui
    ui/c_milestoneview.ui
    ui/c_notesdialog.ui
    ui/c_noteview.ui
    ui/c_processelemview.ui
    ui/c_processview.ui
    ui/c_stepview.ui
)

set(RESOURCE_FILES
    rsc/rsc.qrc
    recipe_manager_fr_FR.ts
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(recipemanager
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${RESOURCE_FILES}
        ${PROJECT_FORM}
    )
else()
    if(ANDROID)
        add_library(recipemanager SHARED
            ${PROJECT_SOURCES}
            ${PROJECT_HEADERS}
            ${RESOURCE_FILES}
            ${PROJECT_FORM}
        )
    else()
        add_executable(recipemanager
            ${PROJECT_SOURCES}
            ${PROJECT_HEADERS}
            ${RESOURCE_FILES}
            ${PROJECT_FORM}
        )
    endif()
endif()

target_link_libraries(recipemanager PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

set_target_properties(recipemanager PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(recipemanager)
endif()
