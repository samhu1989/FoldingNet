#ifndef MESHLISTVIEWERWIDGET_H
#define MESHLISTVIEWERWIDGET_H
#include "visualizationcore_global.h"
#include "MeshType.h"
#include "MeshListViewerWidgetT.h"
#include <QMessageBox>
#include <QFileDialog>
class VISUALIZATIONCORESHARED_EXPORT MeshListViewerWidget:public MeshListViewerWidgetT<DefaultMesh>
{
    Q_OBJECT
public:
    using MeshListViewerWidgetT<DefaultMesh>::Mesh;
    MeshListViewerWidget(QWidget* parent=0) : MeshListViewerWidgetT<DefaultMesh>(parent)
    {}
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
        std::cout << "Saved mesh to ~" << t.as_string() << std::endl;
    }
    uint32_t current_mesh_start(){return current_mesh_start_;}
    uint32_t current_visible_num(){return current_visible_num_;}
    std::vector<arma::uword>& current_selected(){return current_selected_;}
public slots:

    void query_open_file() {
        QStringList fileNames = QFileDialog::getOpenFileNames(this,
            tr("Open Source file"),
            tr("../../Dev_Data/"),
            tr("OBJ Files (*.obj);;"
            "OFF Files (*.off);;"
            "STL Files (*.stl);;"
            "PLY Files (*.ply);;"
            "All Files (*)"));
        if (!fileNames.isEmpty())
        {
            mesh_list_.clear();
            foreach(QString fname,fileNames)
            {
                mesh_list_.push_back(MeshBundle<DefaultMesh>::Ptr(new MeshBundle<DefaultMesh>));
                open_mesh_gui(fname,*mesh_list_.back());
                set_center_at_mesh(mesh_list_.back()->mesh_);
            }
        }
    }

    void query_save_file() {
        QString fileName = QFileDialog::getSaveFileName(this,
            tr("Open Source file"),
            tr("../../Dev_Data/"),
            tr("OBJ Files (*.obj);;"
            "OFF Files (*.off);;"
            "STL Files (*.stl);;"
            "PLY Files (*.ply);;"
            "All Files (*)"));
        if (!fileName.isEmpty())
        {
            save_mesh_gui(fileName,*mesh_list_[current_mesh_start_]);
        }
    }

    void use_custom(bool use){custom_color_=use;}

private:
    OpenMesh::IO::Options _options;
};

#endif // MESHLISTVIEWERWIDGET_H
