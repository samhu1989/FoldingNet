#-------------------------------------------------
#
# Project created by QtCreator 2015-10-27T10:26:41
#
#-------------------------------------------------

QT       += core gui

TARGET = Common
TEMPLATE = lib

DEFINES += COMMON_LIBRARY
CONFIG += c++11
QMAKE_CXXFLAGS += -fopenmp
LIBS += -lgomp -lpthread
SOURCES += common.cpp \
    MeshColor.cpp \
    configure.cpp \
    mbb.cpp

HEADERS += common.h\
        common_global.h \
    MeshColor.h \
    MeshColor.hpp \
    MeshType.h \
    Octree.hpp \
    KDtree.hpp \
    configure.h \
    mbb.h \
    voxelgraph.h \
    voxelgraph.hpp \
    extractmesh.hpp \
    fn_eigs_sym_custom.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    rs.qrc

DISTFILES += \
    rc.rc \
    icon.ico

RC_FILE += rc.rc

DESTDIR = $$OUT_PWD/../../FoldingNet_RunTime/bin

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rdParty/OpenMesh/lib/ -lOpenMeshCore.dll
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rdParty/OpenMesh/lib/ -lOpenMeshCored.dll

INCLUDEPATH += $$PWD/../../3rdParty/OpenMesh/include
DEPENDPATH += $$PWD/../../3rdParty/OpenMesh/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rdParty/OpenMesh/lib/ -lOpenMeshTools.dll
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rdParty/OpenMesh/lib/ -lOpenMeshToolsd.dll

LIBS += -lopenblas

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rdParty/SuperLU/lib/ -lsuperlu
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rdParty/SuperLU/lib/ -lsuperlu

INCLUDEPATH += $$PWD/../../3rdParty/SuperLU/include
DEPENDPATH += $$PWD/../../3rdParty/SuperLU/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../3rdParty/SuperLU/lib/libsuperlu.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../3rdParty/SuperLU/lib/libsuperlu.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../3rdParty/SuperLU/lib/superlu.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../3rdParty/SuperLU/lib/superlu.lib


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../3rdParty/SuperLU/lib/ -lblas
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../3rdParty/SuperLU/lib/ -lblas


win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../3rdParty/SuperLU/lib/libblas.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../3rdParty/SuperLU/lib/libblas.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../3rdParty/SuperLU/lib/blas.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../3rdParty/SuperLU/lib/blas.lib

INCLUDEPATH += $$PWD/../../3rdParty/NanoFlann/include
DEPENDPATH += $$PWD/../../3rdParty/NanoFlann/include


