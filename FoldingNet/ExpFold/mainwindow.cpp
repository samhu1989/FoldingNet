#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    geo_view_ = new MeshPairViewerWidget();
    geo_view_->first_ptr().reset(new MeshBundle<DefaultMesh>());
    geo_view_->set_draw_mode("Flat Colored Vertices");
    ui->layout->addWidget(geo_view_);
    OpenMesh::IO::Options options;
    options += IO::Options::Binary;
    options += IO::Options::VertexColor;
    options += IO::Options::VertexNormal;
    options += IO::Options::FaceColor;
    options += IO::Options::FaceNormal;
    geo_view_->setOptions(options);
    connect(ui->actionLastInput,SIGNAL(triggered(bool)),this,SLOT(last_input()));
    connect(ui->actionNextInput,SIGNAL(triggered(bool)),this,SLOT(next_input()));
}

void MainWindow::get_input_lst()
{
    if(input_list_.empty())
    {
        QStringList namefilter;
        namefilter<<"*.ply"<<"*.obj";
        input_list_ = input_path_.entryList(namefilter,QDir::Files);
    }
    input_current_ = input_list_.begin();
    QString path = input_path_.absoluteFilePath(*input_current_);
    geo_view_->open_mesh_gui(path,geo_view_->first());
    geo_view_->set_center_at_mesh(geo_view_->first().mesh_);
}

void MainWindow::next_input()
{
    if(input_list_.empty())get_input_lst();
    else
    {
        QFileInfo info(*input_current_);
        QString path = output_path_.absoluteFilePath(info.baseName() + "." +info.suffix());
        geo_view_->save_mesh_gui(path,geo_view_->first());
    }
    ++ input_current_;
    if(input_current_==input_list_.end())input_current_=input_list_.begin();
    QString path = input_path_.absoluteFilePath(*input_current_);
    geo_view_->open_mesh_gui(path,geo_view_->first());
    geo_view_->set_center_at_mesh(geo_view_->first().mesh_);
}

void MainWindow::last_input()
{
    if(input_list_.empty())get_input_lst();
    else
    {
        QFileInfo info(*input_current_);
        QString path = output_path_.absoluteFilePath(info.baseName() + "." + info.suffix());
        geo_view_->save_mesh_gui(path,geo_view_->first());
    }
    if(input_current_==input_list_.begin())input_current_=input_list_.end();
    -- input_current_;
    QString path = input_path_.absoluteFilePath(*input_current_);
    geo_view_->open_mesh_gui(path,geo_view_->first());
    geo_view_->set_center_at_mesh(geo_view_->first().mesh_);
}

MainWindow::~MainWindow()
{
    delete ui;
}
