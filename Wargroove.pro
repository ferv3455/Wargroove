QT       += core gui
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aiprocessor.cpp \
    battlewidget.cpp \
    block.cpp \
    building.cpp \
    descriptionwidget.cpp \
    gameinfo.cpp \
    gamemodewidget.cpp \
    gameprocessor.cpp \
    gamestats.cpp \
    gamewidget.cpp \
    main.cpp \
    mainwidget.cpp \
    map.cpp \
    settings.cpp \
    settingswidget.cpp \
    tipslabel.cpp \
    unit.cpp \
    unitmover.cpp \
    unitselectionwidget.cpp

HEADERS += \
    aiprocessor.h \
    battlewidget.h \
    block.h \
    building.h \
    descriptionwidget.h \
    gameinfo.h \
    gamemodewidget.h \
    gameprocessor.h \
    gamestats.h \
    gamewidget.h \
    mainwidget.h \
    map.h \
    settings.h \
    settingswidget.h \
    tipslabel.h \
    unit.h \
    unitmover.h \
    unitselectionwidget.h

FORMS += \
    battlewidget.ui \
    descriptionwidget.ui \
    gamemodewidget.ui \
    gamewidget.ui \
    mainwidget.ui \
    settingswidget.ui \
    unitselectionwidget.ui

TRANSLATIONS += \
    Wargroove_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
