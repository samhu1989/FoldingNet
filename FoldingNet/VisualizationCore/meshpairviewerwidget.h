#ifndef MESHPAIRVIEWERWIDGET_H
#define MESHPAIRVIEWERWIDGET_H
#include "MeshType.h"
//== INCLUDES =================================================================
#include <iostream>
#include <visualizationcore_global.h>
#include <QWidget>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <OpenMesh/Tools/Utils/getopt.h>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include "MeshType.h"
#include "MeshPairViewerWidgetT.h"


//== CLASS DEFINITION =========================================================
class VISUALIZATIONCORESHARED_EXPORT MeshPairViewerWidget : public MeshPairViewerWidgetT<DefaultMesh>
{
    Q_OBJECT
public:
    using MeshPairViewerWidgetT<DefaultMesh>::Mesh;
    /// default constructor
    MeshPairViewerWidget(QWidget* parent=0) : MeshPairViewerWidgetT<DefaultMesh>(parent)
    {
        ;
    }
    virtual ~MeshPairViewerWidget(){}
    OpenMesh::IO::Options& options() { return _options; }
    const OpenMesh::IO::Options& options() const { return _options; }
    void setOptions(const OpenMesh::IO::Options& opts) { _options = opts; }

    void open_mesh_gui(QString fname,MeshBundle<Mesh>&bundle)
    {
        OpenMesh::Utils::Timer t;
        t.start();
        if ( fname.isEmpty() || !open_mesh( fname.toStdString().c_str(),bundle.mesh_,bundle.strips_,_options) )
        {
            QString msg = "Cannot read mesh from file:\n '";
            msg += fname;
            msg += "'";
            QMessageBox::critical( NULL, windowTitle(), msg);
        }
        t.stop();
        std::cout << "Loaded mesh in ~" << t.as_string() << std::endl;
    }

    void save_mesh_gui(QString fname,MeshBundle<Mesh>&bundle)
    {
        OpenMesh::Utils::Timer t;
        t.start();
        if ( fname.isEmpty() || !save_mesh( fname.toStdString().c_str(),bundle.mesh_, _options) )
        {
            QString msg = "Cannot save mesh to file:\n '";
            msg += fname;
            msg += "'";
            QMessageBox::critical( NULL, windowTitle(), msg);
        }
        t.stop();
        std::cerr << "Saved mesh to ~" << t.as_string() << std::endl;
    }

public slots:
    void query_open_source_file() {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Source file"),
            tr("../../Dev_Data/"),
            tr("OBJ Files (*.obj);;"
            "OFF Files (*.off);;"
            "STL Files (*.stl);;"
            "PLY Files (*.ply);;"
            "All Files (*)"));
        if (!fileName.isEmpty())
            open_mesh_gui(fileName,*first_);
        set_center_at_mesh(first_->mesh_);
    }
    void query_open_target_file() {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Target file"),
            tr("../../Dev_Data/"),
            tr("OBJ Files (*.obj);;"
            "OFF Files (*.off);;"
            "STL Files (*.stl);;"
            "PLY Files (*.ply);;"
            "All Files (*)"));
        if (!fileName.isEmpty())
            open_mesh_gui(fileName,*second_);
    }
    void query_save_mesh_file() {
        QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save mesh file"),
            tr("../../Dev_Data/"),
            tr("OBJ Files (*.obj);;"
            "OFF Files (*.off);;"
            "STL Files (*.stl);;"
            "PLY Files (*.ply);;"
            "All Files (*)"));
        if (!fileName.isEmpty())
            save_mesh_gui(fileName,*first_);
    }
    void use_custom_color(bool isUsing){custom_color_=isUsing;updateGL();}
private:
    OpenMesh::IO::Options _options;
};

#endif // MESHPAIRVIEWERWIDGET

