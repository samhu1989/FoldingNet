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


SOURCES += main.cpp\
        mainwindow.cpp \
    DashLine.cpp \
    delaunay.cpp \
    FoldingNet.cpp \
    Graph.cpp \
    GraphEdge.cpp \
    GraphNode.cpp \
    LineSegment.cpp \
    Parameters.cpp \
    Plane.cpp \
    Point.cpp \
    SolidLine.cpp \
    Vertex.cpp

HEADERS  += mainwindow.h \
    DashLine.h \
    delaunay.h \
    FoldingNet.h \
    Graph.h \
    GraphEdge.h \
    GraphNode.h \
    LineSegment.h \
    Parameters.h \
    Plane.h \
    Point.h \
    SolidLine.h \
    Vertex.h

FORMS    += mainwindow.ui



DESTDIR = $$OUT_PWD/../../FoldingNet_RunTime/bin

LIBS += -lfreeglut

win32:CONFIG(release, debug|release): LIBS +=  -lopencv_core.dll
else:win32:CONFIG(debug, debug|release): LIBS += -lopencv_core.dll


win32:CONFIG(release, debug|release): LIBS += -lopencv_highgui.dll
else:win32:CONFIG(debug, debug|release): LIBS += -lopencv_highgui.dll

win32:CONFIG(release, debug|release): LIBS += -lopencv_imgproc.dll
else:win32:CONFIG(debug, debug|release): LIBS += -lopencv_imgproc.dll
