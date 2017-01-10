#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "Parameters.h"
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

    if(!g_config)g_config.reset(new Config("./Default.config"));
    if(!g_config->has("Configure"))g_config->reload("../Default.config");
    if(!g_config->has("Configure"))
    {
        QString msg = "Please Mannually Configure\n";
        QMessageBox::critical(this, windowTitle(), msg);
    }

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

    connect(ui->actionConfigure,SIGNAL(triggered(bool)),this,SLOT(configure()));

    connect(ui->actionLastInput,SIGNAL(triggered(bool)),this,SLOT(last_input()));
    connect(ui->actionNextInput,SIGNAL(triggered(bool)),this,SLOT(next_input()));
    connect(ui->actionLastPlane,SIGNAL(triggered(bool)),this,SLOT(last_plane()));
    connect(ui->actionNext_Plane,SIGNAL(triggered(bool)),this,SLOT(next_plane()));
    connect(ui->actionLast_Axis,SIGNAL(triggered(bool)),this,SLOT(last_axis()));
    connect(ui->actionNext_Axist,SIGNAL(triggered(bool)),this,SLOT(next_axis()));
    connect(ui->actionAngleUp,SIGNAL(triggered(bool)),ui->doubleSpinBox,SLOT(stepUp()));
    connect(ui->actionAngleDown,SIGNAL(triggered(bool)),ui->doubleSpinBox,SLOT(stepDown()));
    connect(ui->doubleSpinBox,SIGNAL(valueChanged(double)),this,SLOT(rotate_to(double)));

    connect(ui->actionView_Dash,SIGNAL(toggled(bool)),this,SLOT(show_dash(bool)));
    connect(ui->actionView_Dash_All,SIGNAL(toggled(bool)),this,SLOT(show_dash_all(bool)));
    connect(ui->actionCalc_Connected,SIGNAL(triggered(bool)),this,SLOT(test_calc_connected()));

    //building a cube as axis
    DefaultMesh& mesh = geo_view_->second().mesh_;
    if(mesh.n_vertices()!=8)
    {
        std::vector<DefaultMesh::VertexHandle> vhandle;
        vhandle.reserve(8);
        std::vector<DefaultMesh::VertexHandle> face_vhandles;
        for(int i=0;i<8;++i)vhandle.push_back(mesh.add_vertex(DefaultMesh::Point(0,0,0)));
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[0]);
        face_vhandles.push_back(vhandle[1]);
        face_vhandles.push_back(vhandle[2]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[2]);
        face_vhandles.push_back(vhandle[3]);
        face_vhandles.push_back(vhandle[0]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[7]);
        face_vhandles.push_back(vhandle[6]);
        face_vhandles.push_back(vhandle[5]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[5]);
        face_vhandles.push_back(vhandle[4]);
        face_vhandles.push_back(vhandle[7]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[1]);
        face_vhandles.push_back(vhandle[0]);
        face_vhandles.push_back(vhandle[4]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[4]);
        face_vhandles.push_back(vhandle[5]);
        face_vhandles.push_back(vhandle[1]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[2]);
        face_vhandles.push_back(vhandle[1]);
        face_vhandles.push_back(vhandle[5]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[5]);
        face_vhandles.push_back(vhandle[6]);
        face_vhandles.push_back(vhandle[2]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[3]);
        face_vhandles.push_back(vhandle[2]);
        face_vhandles.push_back(vhandle[6]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[6]);
        face_vhandles.push_back(vhandle[7]);
        face_vhandles.push_back(vhandle[3]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[0]);
        face_vhandles.push_back(vhandle[3]);
        face_vhandles.push_back(vhandle[7]);
        mesh.add_face(face_vhandles);
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[7]);
        face_vhandles.push_back(vhandle[4]);
        face_vhandles.push_back(vhandle[0]);
        mesh.add_face(face_vhandles);

        mesh.request_vertex_colors();
        mesh.request_vertex_normals();

        arma::Mat<uint8_t> c((uint8_t*)mesh.vertex_colors(),3,mesh.n_vertices(),false,true);
        arma::fmat n((float*)mesh.vertex_normals(),3,mesh.n_vertices(),false,true);
        n.fill(0.0);
        n.row(2).fill(1.0);
        c.fill(0);
        c.row(0).fill(255);
    }
}

void MainWindow::load_current()
{
    QString mesh_path,additional_path;
    QDir dir;
    dir = dir.current();
    //try to load a file with same filename from output path
    QFileInfo tmp_info(*input_current_);
    ui->statusBar->showMessage(tmp_info.fileName(),0);
    mesh_path = dir.relativeFilePath(output_path_.absoluteFilePath(*input_current_));
    additional_path = dir.relativeFilePath(output_path_.absoluteFilePath(tmp_info.baseName()+".sp_imat.arma"));
    QFileInfo info(mesh_path);
    QFileInfo add_info(additional_path);
    //if not exist then load the file from original input path
    if(!info.exists() || !add_info.exists() )
    {
        mesh_path = dir.relativeFilePath(input_path_.absoluteFilePath(*input_current_));
        additional_path = dir.relativeFilePath(input_path_.absoluteFilePath(tmp_info.baseName()+".sp_imat.arma"));
    }
    geo_view_->open_mesh_gui(mesh_path,geo_view_->first());
    if( !connection_.load(additional_path.toStdString()) )
    {
        std::cerr<<"Failed to load from:"<<additional_path.toStdString()<<std::endl;
    }
    geo_view_->set_center_at_mesh(geo_view_->first().mesh_);
    geo_view_->updateGL();
    input_current_color_ = arma::Mat<uint8_t>((uint8_t*)geo_view_->first().mesh_.vertex_colors(),3,geo_view_->first().mesh_.n_vertices(),false,true);
    recover_axis();
    std::cerr<<"done axis"<<std::endl;
    ui->actionView_Dash_All->blockSignals(true);
    ui->actionView_Dash_All->setChecked(false);
    ui->actionView_Dash_All->blockSignals(false);
    side_current_ = 0;
//    std::cerr<<"is dash:"<<is_dash_.t()<<std::endl;
}

void MainWindow::save_current()
{
    side_current_ = 0;
    show_side();
    QFileInfo info(*input_current_);
    QDir dir;
    dir = dir.current();
    QString mesh_path = dir.relativeFilePath(output_path_.absoluteFilePath(info.baseName() + "." +info.suffix()));
    QString additional_path = dir.relativeFilePath(output_path_.absoluteFilePath(info.baseName() + ".sp_imat.arma"));
    geo_view_->save_mesh_gui(mesh_path,geo_view_->first());
    connection_.save(additional_path.toStdString(),arma::arma_binary);
}

void MainWindow::configure(void)
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Confugre"),
                                                    tr("./"),
                                                    tr("Configure (*.config);;"
                                                       "All Files (*)"));
    if (!fileName.isEmpty())
    {
        g_config->reload(fileName.toStdString());
    }
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
    recover_angle();
    show_axis(axis_current_->second);
    side_current_ = 0;
    show_side();
}

void MainWindow::last_axis()
{
    ui->statusBar->showMessage(tr("last axis"),5000);
    if(axis_current_==plane_graph_->axis_.begin())axis_current_=plane_graph_->axis_.end();
    -- axis_current_;
    recover_angle();
    show_axis(axis_current_->second);
    side_current_ = 0;
    show_side();
}

void MainWindow::show_axis(const arma::fvec& axis)
{
    DefaultMesh& mesh = geo_view_->second().mesh_;
    arma::fmat v((float*)mesh.points(),3,mesh.n_vertices(),false,true);
    if(axis.size()!=6){
        std::cerr<<"axis size with:"<<axis.size()<<std::endl;
        return;
    }
    arma::fvec pos((float*)axis.memptr(),3,true,true);
    arma::fvec dir((float*)axis.memptr()+3,3,true,true);
    //generate a box to represent axis
    float h = arma::norm(dir);
    float l = geo_view_->radius() / 100;
    float w =  geo_view_->radius() / 100;
    arma::fmat axis_shape =
    {
      { w, w,-w,-w, w, w,-w,-w},
      { l,-l,-l, l, l,-l,-l, l},
      { h, h, h, h,-h,-h,-h,-h}
    };
    arma::fvec zaxis = {0,0,1};
    v = axis_shape;
    arma::fmat R(3,3,arma::fill::eye);
    if( dir(0) !=0 || dir(1) != 0 )
    {
        getRotationFromZY(arma::normalise(dir),zaxis,R);
    }
    v = R.i()*v;
    v.each_col() += pos;
    mesh.update_normals();
    geo_view_->updateGL();
}

void MainWindow::rotate_to(double angle)
{
    QString msg;
    msg = msg.sprintf("rotate to %lf",angle);
    ui->statusBar->showMessage(msg,5000);
    float theta = ( angle - angle_current_ )*M_PI/180.0;
    arma::fmat T;
    getTransformFromAxis(axis_current_->second,theta,T);
    arma::fmat R = T.submat(0,0,2,2);
    arma::fvec t = T.submat(0,3,2,3);
    //updating points:
    DefaultMesh& mesh = geo_view_->first().mesh_;
    arma::fmat v((float*)mesh.points(),3,mesh.n_vertices(),false,true);
    arma::fmat n((float*)mesh.vertex_normals(),3,mesh.n_vertices(),false,true);
    arma::uvec dark_side;
    std::vector<int> side_axis_;
    switch(side_current_)
    {
    case 1:
        dark_side = plane_graph_->get_side_b(axis_current_->first,side_axis_);
        break;
    case -1:
        dark_side = plane_graph_->get_side_a(axis_current_->first,side_axis_);
        break;
    case 0:
        return;
    }
    n.cols(dark_side) = R*n.cols(dark_side);
    arma::fmat tmp = v.cols(dark_side);
    tmp = R*tmp;
    tmp.each_col() += t;
    v.cols(dark_side) = tmp;
    //updating axis
    for(std::vector<int>::iterator iter=side_axis_.begin();iter!=side_axis_.end();++iter)
    {
       arma::fvec pos = plane_graph_->axis_[*iter-1].second.head(3);
       arma::fvec dir = plane_graph_->axis_[*iter-1].second.tail(3);
       pos = R*pos + t;
       dir = R*dir;
       plane_graph_->axis_[*iter-1].second.head(3) = pos;
       plane_graph_->axis_[*iter-1].second.tail(3) = dir;
    }
    angle_current_ = angle;
    geo_view_->updateGL();
}

void MainWindow::recover_axis()
{
    std::cerr<<"recovering axis"<<std::endl;
    DefaultMesh& mesh = geo_view_->first().mesh_;
    plane_graph_.reset(new PlaneGraph(mesh,connection_,geo_view_->radius()*g_config->getDouble("same_vertex_threshod")));
    std::cerr<<"done plane graph"<<std::endl;
    axis_current_ = plane_graph_->axis_.begin();
    recover_angle();
    std::cerr<<"done recover angle"<<std::endl;
    show_axis(axis_current_->second);
    std::cerr<<"done show axis"<<std::endl;
}

void MainWindow::show_side(void)
{
    DefaultMesh& mesh = geo_view_->first().mesh_;
    arma::Mat<uint8_t> mesh_color((uint8_t*)mesh.vertex_colors(),3,mesh.n_vertices(),false,true);
    arma::uvec dark_side,color_side;
    std::vector<int> side_axis_;
    switch(side_current_)
    {
    case 1:
        color_side = plane_graph_->get_side_a(axis_current_->first,side_axis_);
        dark_side = plane_graph_->get_side_b(axis_current_->first,side_axis_);
        break;
    case -1:
        color_side = plane_graph_->get_side_b(axis_current_->first,side_axis_);
        dark_side = plane_graph_->get_side_a(axis_current_->first,side_axis_);
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

void MainWindow::recover_angle(void)
{
    ui->doubleSpinBox->blockSignals(true);
    ui->doubleSpinBox->setValue(0.0);
    angle_current_ = ui->doubleSpinBox->value();
    ui->doubleSpinBox->blockSignals(false);
}

void MainWindow::show_dash(bool show)
{
//    geo_view_->first_selected().clear();
//    arma::uvec dark_side_dash;
//    if(show)
//    {
//        switch(side_current_)
//        {
//        case 1:
//            dark_side_dash = plane_graph_->get_side_b_dash(axis_current_->first);
//            break;
//        case -1:
//            dark_side_dash = plane_graph_->get_side_a_dash(axis_current_->first);
//            break;
//        case 0:
//            dark_side_dash.clear();
//        }
//        geo_view_->first_selected().insert(
//                    geo_view_->first_selected().end(),
//                    std::begin(dark_side_dash),
//                    std::end(dark_side_dash)
//                    );
//    }
//    geo_view_->updateGL();
}

void MainWindow::show_dash_all(bool show)
{
//    DefaultMesh& mesh = geo_view_->first().mesh_;
//    arma::Mat<uint8_t> mesh_color((uint8_t*)mesh.vertex_colors(),3,mesh.n_vertices(),false,true);
//    if(show)
//    {
//        arma::uvec idx_dash = arma::find(is_dash_==1);
//        std::cerr<<"idx_dash:"<<idx_dash.t()<<std::endl;
//        arma::uvec idx_no_dash = arma::find(is_dash_==-1);
//        std::cerr<<"idx_no_dash:"<<idx_no_dash.t()<<std::endl;
//        arma::Mat<uint8_t> tmp_dash = mesh_color.cols(idx_dash);
//        arma::Mat<uint8_t> tmp_no_dash = mesh_color.cols(idx_no_dash);
//        tmp_dash.fill(0);
//        tmp_no_dash.fill(255);
//        mesh_color.cols(idx_dash) = tmp_dash;
//        mesh_color.cols(idx_no_dash) = tmp_no_dash;
//    }else{
//        mesh_color = input_current_color_;
//    }
//    geo_view_->updateGL();
}

void MainWindow::test_calc_connected(void)
{
    plane_graph_->test_connect_points(axis_current_->first);
}

MainWindow::~MainWindow()
{
    delete ui;
}
