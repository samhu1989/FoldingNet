#ifndef MESHLISTVIEWERWIDGETT_H
#define MESHLISTVIEWERWIDGETT_H
#include <memory>
#include <vector>
#include <string>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/Utils/GenProg.hh>
#include <OpenMesh/Core/Utils/color_cast.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Tools/Utils/StripifierT.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include "MeshType.h"
#include "QGLViewerWidget.h"
#include "MeshColor.h"
#include <QColor>
template <typename M>
class MeshListViewerWidgetT : public QGLViewerWidget
{
public:

  typedef M  Mesh;
  typedef OpenMesh::StripifierT<M> Stripifier;

public:

  /// default constructor
  MeshListViewerWidgetT(QWidget* _parent=0)
    : QGLViewerWidget(_parent),
      f_strips_(false),
      tex_id_(0),
      tex_mode_(GL_MODULATE),
      use_color_(true),
      custom_color_(false),
      show_vnormals_(true),
      show_fnormals_(false),
      current_mesh_start_(0),
      current_visible_num_(1)
  {
    QAction* a = add_draw_mode("Points");
    slotDrawMode(a);
    add_draw_mode("Flat Colored Vertices");
  }

  /// destructor
  ~MeshListViewerWidgetT() {}

public:

  /// open mesh
  virtual bool open_mesh(const char*,Mesh&,Stripifier&,OpenMesh::IO::Options);
  virtual bool save_mesh(const char*, Mesh& ,OpenMesh::IO::Options);

  ///set center of scene
  virtual void set_center_at_mesh(const Mesh&);
  /// load texture
  virtual bool open_texture( const char *_filename );
  bool set_texture( QImage& _texsrc );

  void enable_strips();
  void disable_strips();

  std::vector< typename MeshBundle<Mesh>::Ptr >& list() { return mesh_list_; }
  const std::vector< typename MeshBundle<Mesh>::Ptr >& list() const { return mesh_list_; }

  void show_back(){
      current_mesh_start_ = mesh_list_.size() - 1;
      updateGL();
  }

  void reset_center(){
      set_center_at_mesh(mesh_list_.back()->mesh_);
  }

protected:

  /// inherited drawing method
  virtual void draw_scene(const std::string& _draw_mode);

protected:

  /// draw the mesh
  virtual void draw_openmesh(MeshBundle<Mesh>& b ,const std::string& _drawmode);
  virtual void draw_selected();

  void glVertex( const Mesh& m,  const typename Mesh::VertexHandle _vh)
  { glVertex3fv( &m.point( _vh )[0] ); }

  void glVertex( const typename Mesh::Point& _p)
  { glVertex3fv( &_p[0] ); }

  void glNormal( const Mesh &m,  const typename Mesh::VertexHandle _vh )
  { glNormal3fv( &m.normal( _vh )[0] ); }

  void glTexCoord( const Mesh &m, const typename Mesh::VertexHandle _vh )
  { glTexCoord2fv( &m.texcoord(_vh)[0] ); }

  void glColor( const Mesh &m, const typename Mesh::VertexHandle _vh )
  { glColor3ubv( &m.color(_vh)[0] ); }

  // face properties

  void glNormal( const Mesh &m, const typename Mesh::FaceHandle _fh )
  { glNormal3fv( &m.normal( _fh )[0] ); }

  void glColor( const Mesh &m, const typename Mesh::FaceHandle _fh )
  { glColor3ubv( &m.color(_fh)[0] ); }

  void glMaterial( const Mesh &mesh, const typename Mesh::FaceHandle _fh,
           int _f=GL_FRONT_AND_BACK, int _m=GL_DIFFUSE )
  {
    OpenMesh::Vec3f c=OpenMesh::color_cast<OpenMesh::Vec3f>(mesh.color(_fh));
    OpenMesh::Vec4f m( c[0], c[1], c[2], 1.0f );

    glMaterialfv(_f, _m, &m[0]);
  }

  void glMaterial( const Mesh &mesh, const typename Mesh::VertexHandle _vh,
           int _f=GL_FRONT_AND_BACK, int _m=GL_AMBIENT_AND_DIFFUSE )
  {
    OpenMesh::Vec3f c=OpenMesh::color_cast<OpenMesh::Vec3f>(mesh.color(_vh));
    if(_m==GL_AMBIENT_AND_DIFFUSE)
    {
        OpenMesh::Vec4f m( c[0], c[1], c[2], 1.0f );
        glMaterialfv(_f, _m, &m[0]);
    }
    if(_m==GL_SPECULAR)
    {
        QColor color(c[0],c[1],c[2],1.0);
        color = color.lighter();
        OpenMesh::Vec4f m(color.redF(),color.greenF(),color.blueF(), 1.0f );
        glMaterialfv(_f, _m, &m[0]);
    }
  }

protected: // Strip support

  void compute_strips(OpenMesh::StripifierT<M>& strips)
  {
    if (f_strips_)
    {
      strips.clear();
      strips.stripify();
    }
  }

protected: // inherited

  virtual void keyPressEvent( QKeyEvent* _event);
  virtual void processSelections();
  std::vector<arma::uword> current_selected_;

protected:

  bool                   f_strips_; // enable/disable strip usage
  GLuint                 tex_id_;
  GLint                  tex_mode_;
  OpenMesh::IO::Options  opt_; // mesh file contained texcoords?

  std::vector< typename MeshBundle<Mesh>::Ptr > mesh_list_;

  uint32_t current_mesh_start_;
  uint32_t current_visible_num_;

  bool                   custom_color_;
  bool                   use_color_;
  bool                   show_vnormals_;
  bool                   show_fnormals_;
  float                  normal_scale_;
  OpenMesh::FPropHandleT< typename Mesh::Point > fp_normal_base_;
};
//=============================================================================
#include "MeshListViewerWidgetT.hpp"
//=============================================================================
#endif // MESHLISTVIEWERWIDGETT_H

