#-------------------------------------------------
#
# Project created by QtCreator 2014-06-23T12:06:55
#
#-------------------------------------------------

QT       += core gui opengl network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Neck_Visual
TEMPLATE = app

INCLUDEPATH += .//3dsReader//

SOURCES += main.cpp\
        mainwindow.cpp \
    GLWindow/glwindow.cpp \
    3dsReader/texture.cpp \
    3dsReader/3dsloader.cpp \
    GLWindow/modelEdit.cpp \
    mat/mat.cpp \
    SocketUDP/socketudp.cpp \
    fileProc/iniFileProc.cpp \
    fileProc/logfileproc.cpp \
    glText/Text.cpp

HEADERS  += mainwindow.h \
    glwindow.h \
    GLWindow/glwindow.h \
    3dsReader/texture.h \
    3dsReader/3dsloader.h \
    3dsReader/3dsReader.h \
    GLWindow/modelEdit.h \
    mat/mat.h \
    SocketUDP/socketudp.h \
    fileProc/fileproc.h \
    fileProc/iniFileProc.h \
    logfileproc.h \
    fileProc/logfileproc.h \
    glText/Text.h

FORMS    += mainwindow.ui

RC_FILE += RTC_RMV.rc

QMAKE_CXXFLAGS+=-g3
