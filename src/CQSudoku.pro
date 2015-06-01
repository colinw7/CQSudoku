TEMPLATE = app

QT += widgets

TARGET = CQSudoku

DEPENDPATH += .

#CONFIG += debug

# Input
SOURCES += \
CSudoku.cpp \
CQSudoku.cpp

HEADERS += \
CSudoku.h \
CQSudoku.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
../include \
.

unix:LIBS += \
-L$$LIB_DIR \
