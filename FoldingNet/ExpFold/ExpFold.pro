#-------------------------------------------------
#
# Project created by QtCreator 2017-01-04T09:57:22
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ExpFold
TEMPLATE = app
CONFIG += console
CONFIG += c++11
QMAKE_CXXFLAGS += -fopenmp
LIBS += -lgomp -lpthread

SOURCES += main.cpp\
        mainwindow.cpp \
    delaunay.cpp \
    LineSegment.cpp \
    Parameters.cpp \
    Point.cpp \
    Vertex.cpp \
    planegraph.cpp \
    designtomesh.cpp \
    gotodialog.cpp

HEADERS  += mainwindow.h \
    delaunay.h \
    LineSegment.h \
    Parameters.h \
    Point.h \
    Vertex.h \
    planegraph.h \
    designtomesh.h \
    region.h \
    gotodialog.h

FORMS    += mainwindow.ui \
    gotodialog.ui



DESTDIR = $$OUT_PWD/../../FoldingNet_RunTime/bin

LIBS += -lfreeglut

win32:CONFIG(release, debug|release): LIBS +=  -lopencv_core.dll
else:win32:CONFIG(debug, debug|release): LIBS += -lopencv_core.dll


win32:CONFIG(release, debug|release): LIBS += -lopencv_highgui.dll
else:win32:CONFIG(debug, debug|release): LIBS += -lopencv_highgui.dll

win32:CONFIG(release, debug|release): LIBS += -lopencv_imgproc.dll
else:win32:CONFIG(debug, debug|release): LIBS += -lopencv_imgproc.dll

win32: LIBS += -L$$DESTDIR/ -lVisualizationCore

INCLUDEPATH += $$PWD/../VisualizationCore
DEPENDPATH += $$PWD/../VisualizationCore

win32: LIBS += -L$$DESTDIR/ -lCommon

INCLUDEPATH += $$PWD/../Common
DEPENDPATH += $$PWD/../Common

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rdParty/SuperLU/lib/ -lsuperlu
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rdParty/SuperLU/lib/ -lsuperlu

INCLUDEPATH += $$PWD/../../3rdParty/SuperLU/include
DEPENDPATH += $$PWD/../../3rdParty/SuperLU/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rdParty/OpenMesh/lib/ -lOpenMeshCore.dll
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rdParty/OpenMesh/lib/ -lOpenMeshCored.dll

INCLUDEPATH += $$PWD/../../3rdParty/OpenMesh/include
DEPENDPATH += $$PWD/../../3rdParty/OpenMesh/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rdParty/OpenMesh/lib/ -lOpenMeshTools.dll
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rdParty/OpenMesh/lib/ -lOpenMeshToolsd.dll

INCLUDEPATH += $$PWD/../../3rdParty/NanoFlann/include
DEPENDPATH += $$PWD/../../3rdParty/NanoFlann/include

LIBS += -lopenblas
