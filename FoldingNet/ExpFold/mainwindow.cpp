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
    geo_view_->second_ptr().reset(new MeshBundle<DefaultMesh>());
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

    DefaultMesh& mesh = geo_view_->second().mesh_;
    if(mesh.n_vertices()!=4)
    {
        std::vector<DefaultMesh::VertexHandle> vhandle;
        vhandle.reserve(4);
        for(int i=0;i<4;++i)vhandle.push_back(mesh.add_vertex(DefaultMesh::Point(0,0,0)));

        std::vector<DefaultMesh::VertexHandle> vhandle_face;
        vhandle_face.push_back(vhandle[0]);
        vhandle_face.push_back(vhandle[1]);
        vhandle_face.push_back(vhandle[2]);
        mesh.add_face(vhandle_face);
        vhandle_face.clear();
        vhandle_face.push_back(vhandle[3]);
        vhandle_face.push_back(vhandle[0]);
        vhandle_face.push_back(vhandle[2]);
        mesh.add_face(vhandle_face);

        mesh.request_vertex_colors();
        mesh.request_vertex_normals();

        arma::Mat<uint8_t> c((uint8_t*)mesh.vertex_colors(),3,4,false,true);
        arma::fmat n((float*)mesh.vertex_normals(),3,4,false,true);
        n.fill(0.0);
        n.row(2).fill(1.0);
        c.fill(255);
    }
}

void MainWindow::load_current()
{
    QString mesh_path,additional_path;
    QDir dir;
    dir = dir.current();
    //try to load a file with same filename from output path
    QFileInfo tmp_info(*input_current_);
    mesh_path = dir.relativeFilePath(output_path_.absoluteFilePath(*input_current_));
    additional_path = dir.relativeFilePath(output_path_.absoluteFilePath(tmp_info.baseName()+".fvec.arma"));
    QFileInfo info(mesh_path);
    QFileInfo add_info(additional_path);
    //if not exist then load the file from original input path
    if(!info.exists() || !add_info.exists() )
    {
        mesh_path = dir.relativeFilePath(input_path_.absoluteFilePath(*input_current_));
        additional_path = dir.relativeFilePath(input_path_.absoluteFilePath(tmp_info.baseName()+".fvec.arma"));
    }
    geo_view_->open_mesh_gui(mesh_path,geo_view_->first());
    if( !is_dash_.load(additional_path.toStdString()) )
    {
        std::cerr<<"Failed to load from:"<<additional_path.toStdString()<<std::endl;
    }
    if(geo_view_->first().mesh_.n_vertices()!=is_dash_.size())
    {
        std::cerr<<"The mesh size("<<geo_view_->first().mesh_.n_vertices()<<") is not consistent with the dash state("<<is_dash_.size()<<")"<<std::endl;
    }
    geo_view_->set_center_at_mesh(geo_view_->first().mesh_);
    geo_view_->updateGL();
    input_current_color_ = arma::Mat<uint8_t>((uint8_t*)geo_view_->first().mesh_.vertex_colors(),3,geo_view_->first().mesh_.n_vertices(),false,true);
    recover_axis();
    side_current_ = 0;
}

void MainWindow::save_current()
{
    side_current_ = 0;
    show_side();
    QFileInfo info(*input_current_);
    QDir dir;
    dir = dir.current();
    QString mesh_path = dir.relativeFilePath(output_path_.absoluteFilePath(info.baseName() + "." +info.suffix()));
    QString additional_path = dir.relativeFilePath(output_path_.absoluteFilePath(info.baseName() + ".fvec.arma"));
    geo_view_->save_mesh_gui(mesh_path,geo_view_->first());
    is_dash_.save(additional_path.toStdString(),arma::arma_binary);
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
    ++ side_current_  ;
    if(side_current_>1)side_current_=-1;
    show_side();
}

void MainWindow::last_plane()
{
    -- side_current_ ;
    if(side_current_<-1)side_current_=1;
    show_side();
}

void MainWindow::next_axis()
{
    ui->statusBar->showMessage(tr("next axis"),5000);
    ++ axis_current_;
    if(axis_current_==plane_graph_->axis_.end())axis_current_=plane_graph_->axis_.begin();
    show_axis(axis_current_->second);
    side_current_ = 0;
    show_side();
}

void MainWindow::last_axis()
{
    ui->statusBar->showMessage(tr("last axis"),5000);
    if(axis_current_==plane_graph_->axis_.begin())axis_current_=plane_graph_->axis_.end();
    -- axis_current_;
    show_axis(axis_current_->second);
    side_current_ = 0;
    show_side();
}

void MainWindow::show_axis(const arma::fvec& axis)
{
    DefaultMesh& mesh = geo_view_->second().mesh_;
    arma::fmat v((float*)mesh.points(),3,4,false,true);
    arma::fvec pos((float*)axis.memptr(),3,true,true);
    arma::fvec dir((float*)axis.memptr()+3,3,true,true);

    v.each_col() = pos;
    arma::fvec w = {0,0,geo_view_->radius()/100.0};
    v.col(0) += dir;
    v.col(1) += dir;
    v.col(2) -= dir;
    v.col(3) -= dir;
    v.col(0) += w;
    v.col(3) += w;
    v.col(1) -= w;
    v.col(2) -= w;

    mesh.update_normals();
    geo_view_->updateGL();
}

void MainWindow::rotate_to(double angle)
{
    QString msg;
    msg = msg.sprintf("rotate to %lf",angle);
    ui->statusBar->showMessage(msg,5000);
}

void MainWindow::recover_axis()
{
    DefaultMesh& mesh = geo_view_->first().mesh_;
    plane_graph_.reset(new PlaneGraph(mesh,is_dash_));
    axis_current_ = plane_graph_->axis_.begin();
    show_axis(axis_current_->second);
}

void MainWindow::show_side(void)
{
    DefaultMesh& mesh = geo_view_->first().mesh_;
    arma::Mat<uint8_t> mesh_color((uint8_t*)mesh.vertex_colors(),3,mesh.n_vertices(),false,true);
    arma::uvec dark_side,color_side;
    switch(side_current_)
    {
    case 1:
        color_side = plane_graph_->get_side_a(axis_current_->first);
        dark_side = plane_graph_->get_side_b(axis_current_->first);
        break;
    case -1:
        color_side = plane_graph_->get_side_b(axis_current_->first);
        dark_side = plane_graph_->get_side_a(axis_current_->first);
        break;
    case 0:
        color_side = arma::linspace<arma::uvec>(0,mesh.n_vertices()-1,mesh.n_vertices());
        dark_side.clear();
    }
    QString msg;
    msg = msg.sprintf("Current Side %d",side_current_);
    ui->statusBar->showMessage(msg,5000);
    mesh_color.cols(color_side) = input_current_color_.cols(color_side);
    mesh_color.cols(dark_side).fill(0);
    geo_view_->updateGL();
}

MainWindow::~MainWindow()
{
    delete ui;
}
