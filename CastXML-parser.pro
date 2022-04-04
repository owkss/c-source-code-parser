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
    codeeditor.cpp \
    exportmodel.cpp \
    exportview.cpp \
    highlighter.cpp \
    main.cpp \
    mainwindow.cpp \
    typestorage.cpp \
    variabledialog.cpp \
    xmlparsing.cpp

HEADERS += \
    codeeditor.h \
    exportmodel.h \
    exportview.h \
    highlighter.h \
    mainwindow.h \
    test.h \
    type.h \
    typestorage.h \
    variabledialog.h
