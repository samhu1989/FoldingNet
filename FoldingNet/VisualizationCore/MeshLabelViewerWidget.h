#ifndef MESHLABELVIEWERWIDGET_H
#define MESHLABELVIEWERWIDGET_H
#include <QWidget>
#include "MeshLabelViewerWidgetPrivate.h"
#include "visualizationcore_global.h"
namespace Ui {
class MeshLabelViewerWidget;
}

class VISUALIZATIONCORESHARED_EXPORT MeshLabelViewerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MeshLabelViewerWidget(QWidget *parent = 0);
    void set_mesh(MeshBundle<DefaultMesh>::Ptr);
    virtual ~MeshLabelViewerWidget();
public slots:
    void use_custom_color(bool use){gl_view_->use_custom_color(use);gl_view_->updateGL();}
    void updateGL(){gl_view_->updateGL();}
private:
    Ui::MeshLabelViewerWidget *ui;
    MeshLabelViewerWidgetPrivate* gl_view_;
};

#endif // MESHLABELVIEWERWIDGET_H
