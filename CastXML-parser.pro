QT += core gui widgets
CONFIG += c++11

CONFIG(release, debug|release){
    DESTDIR = release
    MOC_DIR = release/moc
    OBJECTS_DIR = release/obj
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    DESTDIR = debug
    MOC_DIR = debug/moc
    OBJECTS_DIR = debug/obj
}

INCLUDEPATH += $$PWD/rapidxml-1.13/

SOURCES += \
    exportmodel.cpp \
    exportview.cpp \
    main.cpp \
    mainwindow.cpp \
    typestorage.cpp \
    xmlparsing.cpp

HEADERS += \
    exportmodel.h \
    exportview.h \
    mainwindow.h \
    test.h \
    type.h \
    typestorage.h
