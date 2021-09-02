QT       += core gui
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    block.cpp \
    gameinfo.cpp \
    gameprocessor.cpp \
    gamewidget.cpp \
    main.cpp \
    mainwidget.cpp \
    map.cpp \
    settings.cpp \
    tipslabel.cpp \
    unit.cpp \
    unitmover.cpp

HEADERS += \
    block.h \
    gameinfo.h \
    gameprocessor.h \
    gamewidget.h \
    mainwidget.h \
    map.h \
    settings.h \
    tipslabel.h \
    unit.h \
    unitmover.h

FORMS += \
    gamewidget.ui \
    mainwidget.ui

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
