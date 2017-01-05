#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    geo_view_ = new MeshPairViewerWidget();
    geo_view_->first_ptr().reset(new MeshBundle<DefaultMesh>());
    geo_view_->set_draw_mode("Flat Colored Vertices");
    ui->formLayout->addWidget(geo_view_);
    OpenMesh::IO::Options options;
    options += IO::Options::Binary;
    options += IO::Options::VertexColor;
    options += IO::Options::VertexNormal;
    options += IO::Options::FaceColor;
    options += IO::Options::FaceNormal;
    geo_view_->setOptions(options);
    geo_view_->setMinimumSize(320,240);
    addAction(ui->actionAngleUp);
    addAction(ui->actionAngleDown);

    connect(ui->actionLastInput,SIGNAL(triggered(bool)),this,SLOT(last_input()));
    connect(ui->actionNextInput,SIGNAL(triggered(bool)),this,SLOT(next_input()));
    connect(ui->actionLastPlane,SIGNAL(triggered(bool)),this,SLOT(last_plane()));
    connect(ui->actionNext_Plane,SIGNAL(triggered(bool)),this,SLOT(next_plane()));
    connect(ui->actionLast_Axis,SIGNAL(triggered(bool)),this,SLOT(last_axis()));
    connect(ui->actionNext_Axist,SIGNAL(triggered(bool)),this,SLOT(next_axis()));
    connect(ui->actionAngleUp,SIGNAL(triggered(bool)),ui->doubleSpinBox,SLOT(stepUp()));
    connect(ui->actionAngleDown,SIGNAL(triggered(bool)),ui->doubleSpinBox,SLOT(stepDown()));
    connect(ui->doubleSpinBox,SIGNAL(valueChanged(double)),this,SLOT(rotate_to(double)));

}

void MainWindow::load_current()
{
    QString path;
    QDir dir;
    dir = dir.current();
    //try to load a file with same filename from output path
    path = dir.relativeFilePath(output_path_.absoluteFilePath(*input_current_));
    QFileInfo info(path);
    //if not exist then load the file from original input path
    if(!info.exists())
    {
        path = dir.relativeFilePath(input_path_.absoluteFilePath(*input_current_));
    }
    geo_view_->open_mesh_gui(path,geo_view_->first());
    geo_view_->set_center_at_mesh(geo_view_->first().mesh_);
    geo_view_->updateGL();
    recover_plates(geo_view_->first().mesh_);
}

void MainWindow::save_current()
{
    QFileInfo info(*input_current_);
    QDir dir;
    dir = dir.current();
    QString path = dir.relativeFilePath(output_path_.absoluteFilePath(info.baseName() + "." +info.suffix()));
    geo_view_->save_mesh_gui(path,geo_view_->first());
}

void MainWindow::get_input_lst()
{
    if(input_list_.empty())
    {
        QStringList namefilter;
        namefilter<<"*.ply"<<"*.obj";
        input_list_ = input_path_.entryList(namefilter,QDir::Files);
    }
    if(input_list_.empty())
    {
        std::cerr<<"empty input at"<<std::endl;
        std::cerr<<input_path_.path().toStdString()<<std::endl;
        return;
    }
    input_current_ = input_list_.begin();
    load_current();
}

void MainWindow::next_input()
{
    if(input_list_.empty()){
        get_input_lst();
        return;
    }
    save_current();
    ++ input_current_;
    if(input_current_==input_list_.end())input_current_=input_list_.begin();
    load_current();
}

void MainWindow::last_input()
{
    if(input_list_.empty()){
        get_input_lst();
        return;
    }
    save_current();
    if(input_current_==input_list_.begin())input_current_=input_list_.end();
    -- input_current_;
    load_current();
}

void MainWindow::next_plane()
{
    ui->statusBar->showMessage(tr("next plane"),5000);
}

void MainWindow::last_plane()
{
    ui->statusBar->showMessage(tr("last plane"),5000);
}

void MainWindow::next_axis()
{
    ui->statusBar->showMessage(tr("next axis"),5000);
}

void MainWindow::last_axis()
{
    ui->statusBar->showMessage(tr("last axis"),5000);
}

void MainWindow::rotate_to(double angle)
{
    QString msg;
    msg = msg.sprintf("rotate to %lf",angle);
    ui->statusBar->showMessage(msg,5000);
}

void MainWindow::recover_plates(const DefaultMesh& mesh)
{

    for (DefaultMesh::VertexIter v_it=mesh.vertices_sbegin(); v_it!=mesh.vertices_end(); ++v_it)
    {
      for (DefaultMesh::ConstVertexVertexIter vv_it=mesh.cvv_iter(*v_it); vv_it.is_valid(); ++vv_it)
      {
          ;
      }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
