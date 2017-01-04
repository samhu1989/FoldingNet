#-------------------------------------------------
#
# Project created by QtCreator 2015-10-23T16:39:51
#
#-------------------------------------------------

QT       += widgets opengl

TARGET = VisualizationCore
TEMPLATE = lib
CONFIG += c++11

DEFINES += VISUALIZATIONCORE_LIBRARY
DESTDIR = $$OUT_PWD/../../../Dev_RunTime/bin

SOURCES += visualizationcore.cpp \
    QGLViewerWidget.cpp \
    labspace.cpp \
    qglpointselection.cpp \
    featureviewerwidget.cpp \
    segview.cpp \
    MeshLabelViewerWidget.cpp \
    MeshLabelViewerWidgetPrivate.cpp

HEADERS += visualizationcore.h\
        visualizationcore_global.h \
    MeshPairViewerWidgetT.h \
    MeshViewerWidgetT.h \
    MeshViewerWidget.h \
    QGLViewerWidget.h \
    MeshViewerWidgetT.hpp \
    MeshPairViewerWidgetT.hpp \
    MeshPairViewerWidget.h \
    MeshListViewerWidget.h \
    MeshListViewerWidgetT.h \
    MeshListViewerWidgetT.hpp \
    labspace.h \
    qglpointselection.h \
    qglpointselection.hpp \
    featureviewerwidget.h \
    segview.h \
    MeshLabelViewerWidget.h \
    MeshLabelViewerWidgetPrivate.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}


win32: LIBS += -L$$DESTDIR/ -lCommon

INCLUDEPATH += $$PWD/../../Common
DEPENDPATH += $$PWD/../../Common

win32:CONFIG(release, debug|release): LIBS += -lfreeglut
else:win32:CONFIG(debug, debug|release): LIBS += -lfreeglut


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../3rdParty/OpenMesh/lib/ -lOpenMeshCore.dll
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../3rdParty/OpenMesh/lib/ -lOpenMeshCored.dll

INCLUDEPATH += $$PWD/../../../3rdParty/OpenMesh/include
DEPENDPATH += $$PWD/../../../3rdParty/OpenMesh/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../3rdParty/OpenMesh/lib/ -lOpenMeshTools.dll
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../3rdParty/OpenMesh/lib/ -lOpenMeshToolsd.dll

INCLUDEPATH += $$PWD/../../../3rdParty/SuperLU/include
DEPENDPATH += $$PWD/../../../3rdParty/SuperLU/include

FORMS += \
    labspace.ui \
    featureviewerwidget.ui \
    segview.ui \
    meshlabelviewerwidget.ui

INCLUDEPATH += $$PWD/../../../3rdParty/NanoFlann/include
DEPENDPATH += $$PWD/../../../3rdParty/NanoFlann/include

LIBS += -lopenblas
