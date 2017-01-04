#include "MeshLabelViewerWidget.h"
#include "ui_meshlabelviewerwidget.h"

MeshLabelViewerWidget::MeshLabelViewerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MeshLabelViewerWidget)
{
    ui->setupUi(this);
    gl_view_ = new MeshLabelViewerWidgetPrivate();
    ui->verticalLayout->addWidget(gl_view_);
    connect(ui->toolButton,SIGNAL(toggled(bool)),gl_view_,SLOT(rect_select(bool)));
}

void MeshLabelViewerWidget::set_mesh(MeshBundle<DefaultMesh>::Ptr ptr)
{
    gl_view_->first_ptr() = ptr;
    gl_view_->setMinimumSize(300,200);
    gl_view_->set_center_at_mesh(ptr->mesh_);
    setWindowTitle(QString::fromStdString(ptr->name_));
    update();
}

MeshLabelViewerWidget::~MeshLabelViewerWidget()
{
    gl_view_->deleteLater();
    delete ui;
}
