#ifndef MESHLABELVIEWERWIDGETPRIVATE_H
#define MESHLABELVIEWERWIDGETPRIVATE_H
#include "MeshPairViewerWidgetT.h"

class MeshLabelViewerWidgetPrivate:public MeshPairViewerWidgetT<DefaultMesh>
{
    Q_OBJECT
public:
    MeshLabelViewerWidgetPrivate();
    virtual ~MeshLabelViewerWidgetPrivate(){}
public slots:
    void use_custom_color(bool use){custom_color_=use;}
};

#endif // MESHLABELVIEWERWIDGETPRIVATE_H
