/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2015, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */

/*===========================================================================*\
 *                                                                           *             
 *   $Revision: 1278 $                                                         *
 *   $Date: 2015-06-09 10:58:41 +0200 (Di, 09 Jun 2015) $                   *
 *                                                                           *
\*===========================================================================*/

//== INCLUDES =================================================================

#ifdef _MSC_VER
#  pragma warning(disable: 4267 4311 4305)
#endif

#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
// --------------------
#ifdef ARCH_DARWIN
#  include <glut.h>
#else
#  include <GL/glut.h>
#endif
// --------------------
#include <QApplication>
#include <QMenu>
#include <QCursor>
#include <QImage>
#include <QDateTime>
#include <QMouseEvent>
// --------------------
#include "QGLViewerWidget.h"
#include <OpenMesh/Tools/Utils/Timer.hh>

#if !defined(M_PI)
#  define M_PI 3.1415926535897932
#endif

const double TRACKBALL_RADIUS = 0.6;


using namespace Qt;
using namespace OpenMesh;


//== IMPLEMENTATION ========================================================== 

std::string QGLViewerWidget::nomode_ = "";

//----------------------------------------------------------------------------

QGLViewerWidget::QGLViewerWidget( QWidget* _parent )
  : QGLWidget( _parent )
{    
  init();
}

//----------------------------------------------------------------------------

QGLViewerWidget::
QGLViewerWidget( QGLFormat& _fmt, QWidget* _parent )
  : QGLWidget( _fmt, _parent )
{
  init();
}


//----------------------------------------------------------------------------

void 
QGLViewerWidget::init(void)
{
  // qt stuff
  setAttribute(Qt::WA_NoSystemBackground, true);
  setFocusPolicy(Qt::ClickFocus);
  setAcceptDrops( true );  
  setCursor(PointingHandCursor);
  // init coordinates
  arma::fmat coordinates(&coord_[0],6,3,false,true);
  coordinates.rows(0,2).fill(0.0);
  coordinates.rows(3,5) = 0.3*radius_*arma::eye<arma::fmat>(3,3);
  arma::Mat<uint8_t> coord_color(&coord_color_[0],6,3,false,true);
  coord_color.rows(0,2) = arma::eye< arma::Mat<uint8_t> >(3,3);
  coord_color.rows(3,5) = arma::eye< arma::Mat<uint8_t> >(3,3);
  coord_color *= 255;
  arma::Col<GLushort> coord_index(&coord_index_[0],6,false,true);
  coord_index = arma::linspace<arma::Col<GLushort>>(0,5,6);
  // popup menu

  popup_menu_ = new QMenu(this);
  draw_modes_group_ = new QActionGroup(this);

  connect( draw_modes_group_, SIGNAL(triggered(QAction*)),
	   this, SLOT(slotDrawMode(QAction*)));

  point_size_ = 5.0;

  rect_select_ = false;
  rect_selecting_ = false;
  // init draw modes
  n_draw_modes_ = 0;
  //draw_mode_ = 4;
  QAction *a;
  a = add_draw_mode("Wireframe");
  a->setShortcutContext(Qt::WidgetShortcut);
  a->setShortcut(QKeySequence(SHIFT|Key_W));
  add_draw_mode("Solid Flat");
  a = add_draw_mode("Solid Smooth");
  a->setShortcutContext(Qt::WidgetShortcut);
  a->setShortcut(QKeySequence(SHIFT|Key_S));
  a->setChecked(true);

  slotDrawMode(a);
}


//----------------------------------------------------------------------------

QGLViewerWidget::~QGLViewerWidget()
{

}


//----------------------------------------------------------------------------

void 
QGLViewerWidget::setDefaultMaterial(void)
{
  GLfloat mat_a[] = {0.250000, 0.148000, 0.064750, 1.000000};
  GLfloat mat_d[] = {0.400000, 0.236800, 0.103600, 1.000000};
  GLfloat mat_s[] = {0.774597, 0.458561, 0.200621, 1.000000};
  GLfloat shine[] = {76.800003};
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mat_a);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_d);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_s);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
}


//----------------------------------------------------------------------------

void
QGLViewerWidget::setDefaultLight(void)
{
  GLfloat pos1[] = { 0.1,  0.1, -0.02, 0.0};
  GLfloat pos2[] = {-0.1,  0.1, -0.02, 0.0};
  GLfloat pos3[] = { 0.0,  0.0,  0.1,  0.0};
  GLfloat col1[] = { 0.7,  0.7,  0.8,  1.0};
  GLfloat col2[] = { 0.8,  0.7,  0.7,  1.0};
  GLfloat col3[] = { 1.0,  1.0,  1.0,  1.0};
 
  glEnable(GL_LIGHT0);    
  glLightfv(GL_LIGHT0,GL_POSITION, pos1);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,  col1);
  glLightfv(GL_LIGHT0,GL_SPECULAR, col1);
  
  glEnable(GL_LIGHT1);  
  glLightfv(GL_LIGHT1,GL_POSITION, pos2);
  glLightfv(GL_LIGHT1,GL_DIFFUSE,  col2);
  glLightfv(GL_LIGHT1,GL_SPECULAR, col2);
  
  glEnable(GL_LIGHT2);  
  glLightfv(GL_LIGHT2,GL_POSITION, pos3);
  glLightfv(GL_LIGHT2,GL_DIFFUSE,  col3);
  glLightfv(GL_LIGHT2,GL_SPECULAR, col3);
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::initializeGL()
{  
  // OpenGL state
  glClearColor(0.7, 0.7, 0.7, 0.0);
  glDisable( GL_DITHER );
  glEnable( GL_DEPTH_TEST );

  // Material
  setDefaultMaterial();
  
  // Lighting
  glLoadIdentity();
  setDefaultLight();  
  
  // Fog
  GLfloat fogColor[4] = { 0.3, 0.3, 0.4, 1.0 };
  glFogi(GL_FOG_MODE,    GL_LINEAR);
  glFogfv(GL_FOG_COLOR,  fogColor);
  glFogf(GL_FOG_DENSITY, 0.35);
  glHint(GL_FOG_HINT,    GL_DONT_CARE);
  glFogf(GL_FOG_START,    8.0f);
  glFogf(GL_FOG_END,     50.0f);

  // scene pos and size
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
  set_scene_pos(Vec3f(0.0, 0.0, 0.0), 1.0);
}

void QGLViewerWidget::setBackgroundColor(QColor c)
{
    glClearColor(c.redF(),c.greenF(),c.blueF(), 0.0);
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::resizeGL( int _w, int _h )
{
  update_projection_matrix();
  glViewport(0, 0, _w, _h);
  updateGL();
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode( GL_PROJECTION );
  glLoadMatrixd( projection_matrix_ );
  glMatrixMode( GL_MODELVIEW );
  glLoadMatrixd( modelview_matrix_ );

  if (draw_mode_)
  {
    assert(draw_mode_ <= n_draw_modes_);
    draw_coordinate();
    draw_scene(draw_mode_names_[draw_mode_-1]);
    if(!selections_.empty())processSelections();

  }

}


//----------------------------------------------------------------------------


void
QGLViewerWidget::draw_scene(const std::string& _draw_mode)
{  
  if (_draw_mode == "Wireframe")
  {
    glDisable(GL_LIGHTING);
    glutWireTeapot(0.5);
  }

  else if (_draw_mode == "Solid Flat")
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
    glutSolidTeapot(0.5);
  }

  else if (_draw_mode == "Solid Smooth")
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glutSolidTeapot(0.5);
  }
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::mousePressEvent( QMouseEvent* _event )
{
  // popup menu
  if(_event->button() == RightButton && _event->buttons()== RightButton )
  {
     popup_menu_->exec(QCursor::pos());
  }
  else if(_event->button() == LeftButton && _event->modifiers() == ShiftModifier)
  {
      Vec3f  newPoint3D;
      map_to_far(_event->pos(), newPoint3D );
      arma::fvec p0(3,arma::fill::zeros);
      arma::fvec p1(3);
      p1(0) = newPoint3D[0];
      p1(1) = newPoint3D[1];
      p1(2) = newPoint3D[2];
      arma::fvec w0,w1;
      map_to_world(p0,w0);
      map_to_world(p1,w1);
      if(rect_select_)
      {
          BoxPointsSelection* box_select = new BoxPointsSelection();
          box_select->setNear(w0);
          rect_point_2D_ = _event->pos();
          selections_.emplace_back(box_select);
          rect_selecting_ = true;
      }else selections_.emplace_back(new RayPointSelection(w0,w1));
  }else
  {
    last_point_ok_ = map_to_sphere( last_point_2D_=_event->pos(),
				    last_point_3D_ );
  }
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::mouseMoveEvent( QMouseEvent* _event )
{  
  QPoint newPoint2D = _event->pos(); 
  
  // Left button: rotate around center_
  // Middle button: translate object
  // Left & middle button: zoom in/out
  

  Vec3f  newPoint3D;
  bool   newPoint_hitSphere = map_to_sphere( newPoint2D, newPoint3D );

  float dx = newPoint2D.x() - last_point_2D_.x();
  float dy = newPoint2D.y() - last_point_2D_.y();
  
  float w  = width();
  float h  = height();



  // enable GL context
  makeCurrent();
  if(rect_selecting_)
  {
      if(_event->button() == LeftButton && _event->modifiers() == ShiftModifier)
      {
          arma::fmat::fixed<3,4> rect;
          Vec3f  tmp3d;
          arma::fvec tmp3d_v(tmp3d.data(),3,false,true);
          QPoint p0 = rect_point_2D_;
          QPoint p2 = _event->pos();
          QPoint p1(p0.x(),p2.y());
          QPoint p3(p2.x(),p0.y());
          arma::fvec w0(rect.colptr(0),3,false,true);
          arma::fvec w1(rect.colptr(1),3,false,true);
          arma::fvec w2(rect.colptr(2),3,false,true);
          arma::fvec w3(rect.colptr(3),3,false,true);
          map_to_far(p0,tmp3d);
          map_to_world(tmp3d_v,w0);
          map_to_far(p1,tmp3d);
          map_to_world(tmp3d_v,w1);
          map_to_far(p2,tmp3d);
          map_to_world(tmp3d_v,w2);
          map_to_far(p3,tmp3d);
          map_to_world(tmp3d_v,w3);
          if(rect_select_)
          {
              BoxPointsSelection* box_select =  dynamic_cast<BoxPointsSelection*>(selections_.back().get());
              box_select->setRect(rect);
          }
      }
  }else
  {
      // move in z direction
      if ( (_event->buttons() == (LeftButton+MidButton)) ||
           (_event->buttons() == LeftButton && _event->modifiers() == ControlModifier))
      {
          float value_y = radius_ * dy * 3.0 / h;
          translate(Vec3f(0.0, 0.0, value_y));
      }
      // move in x,y direction
      else if ( (_event->buttons() == MidButton) ||
                (_event->buttons() == LeftButton && _event->modifiers() == AltModifier) )
      {
          float z = - (modelview_matrix_[ 2]*center_[0] +
                  modelview_matrix_[ 6]*center_[1] +
                  modelview_matrix_[10]*center_[2] +
                  modelview_matrix_[14]) /
                  (modelview_matrix_[ 3]*center_[0] +
                  modelview_matrix_[ 7]*center_[1] +
                  modelview_matrix_[11]*center_[2] +
                  modelview_matrix_[15]);

          float aspect     = w / h;
          float near_plane = 0.01 * radius_;
          float top        = tan(fovy()/2.0f*M_PI/180.0f) * near_plane;
          float right      = aspect*top;

          translate(Vec3f( 2.0*dx/w*right/near_plane*z,
                           -2.0*dy/h*top/near_plane*z,
                           0.0f));
      }
      // rotate
      else if (_event->buttons() == LeftButton) {
          if (last_point_ok_) {
              if ((newPoint_hitSphere = map_to_sphere(newPoint2D, newPoint3D))) {
                  Vec3f axis = last_point_3D_ % newPoint3D;
                  if (axis.sqrnorm() < 1e-7) {
                      axis = Vec3f(1, 0, 0);
                  } else {
                      axis.normalize();
                  }
                  // find the amount of rotation
                  Vec3f d = last_point_3D_ - newPoint3D;
                  float t = 0.5 * d.norm() / TRACKBALL_RADIUS;
                  if (t < -1.0)
                      t = -1.0;
                  else if (t > 1.0)
                      t = 1.0;
                  float phi = 2.0 * asin(t);
                  float angle = phi * 180.0 / M_PI;
                  rotate(axis, angle);
              }
          }
      }
  }
  // remember this point
  last_point_2D_ = newPoint2D;
  last_point_3D_ = newPoint3D;
  last_point_ok_ = newPoint_hitSphere;
  // trigger redraw
  updateGL();
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::mouseReleaseEvent( QMouseEvent* _event )
{  
    rect_selecting_ = false;
    last_point_ok_ = false;
}


//-----------------------------------------------------------------------------


void QGLViewerWidget::wheelEvent(QWheelEvent* _event)
{
  // Use the mouse wheel to zoom in/out
  if( _event->modifiers() & AltModifier )
  {
      float d = -(float)_event->delta() / 120.0 * 0.05 * 30.0;
      point_size_ += d;
      if(point_size_>30.0)point_size_=30.0;
      if(point_size_<1.0)point_size_=1.0;
  }else
  {
      float d = -(float)_event->delta() / 120.0 * 0.2 * radius_;
      translate(Vec3f(0.0, 0.0, d));
  }
  updateGL();
  _event->accept();
}


//----------------------------------------------------------------------------


void QGLViewerWidget::keyPressEvent( QKeyEvent* _event)
{
  switch( _event->key() )
  {
    case Key_Print:
      slotSnapshot();
      _event->accept();
      break;

    case Key_H:
      std::cerr << "Keys:\n";
      std::cerr << "  Print\tMake snapshot\n";
      std::cerr << "  Shift C\tenable/disable back face culling\n";
      std::cerr << "  F\tenable/disable fog\n";
      std::cerr << "  I\tDisplay information\n";
      std::cerr << "  N\tenable/disable display of vertex normals\n";
      std::cerr << "  Shift N\tenable/disable display of face normals\n";
      std::cerr << "  Shift P\tperformance check\n";
      _event->accept();
      break;

    case Key_C:
      if (_event->modifiers() & ShiftModifier)
      {
          if ( glIsEnabled( GL_CULL_FACE ) )
          {
              glDisable( GL_CULL_FACE );
              std::cerr << "Culling: disabled\n";
          }
          else
          {
              glEnable( GL_CULL_FACE );
              std::cerr << "Culling: enabled\n";
          }
          updateGL();
          _event->accept();
      }
      break;

    case Key_F:
      if ( glIsEnabled( GL_FOG ) )
      {
          glDisable( GL_FOG );
          std::cerr << "Fog: disabled\n";
      }
      else
      {
          glEnable( GL_FOG );
          std::cerr << "Fog: enabled\n";
      }
      _event->accept();
      updateGL();
      break;

    case Key_I:
      std::cerr << "Scene radius: " << radius_ << std::endl;
      std::cerr << "Scene center: " << center_ << std::endl;
      _event->accept();
      break;

    case Key_P:
      if (_event->modifiers() & ShiftModifier)
      {
          double fps = performance();
          std::cerr << "fps: "
                    << std::setiosflags (std::ios_base::fixed)
                    << fps << std::endl;
      }
      _event->accept();
    break;
    case Key_Q:
    case Key_Escape:
      close();
  }
  _event->ignore();
}

void QGLViewerWidget::processSelections()
{
    selections_.debugSelections();
}

//----------------------------------------------------------------------------

void QGLViewerWidget::draw_coordinate()
{
    glDisable(GL_LIGHTING);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &coord_[0]);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_UNSIGNED_BYTE, 0, &coord_color_[0]);
    glLineWidth(6.0);
    glDrawElements(GL_LINES,6,GL_UNSIGNED_SHORT,&coord_index_[0]);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}
//----------------------------------------------------------------------------

void
QGLViewerWidget::translate( const OpenMesh::Vec3f& _trans )
{
  // Translate the object by _trans
  // Update modelview_matrix_
  makeCurrent();
  glLoadIdentity();
  glTranslated( _trans[0], _trans[1], _trans[2] );
  glMultMatrixd( modelview_matrix_ );
  glGetDoublev( GL_MODELVIEW_MATRIX, modelview_matrix_);
}


//----------------------------------------------------------------------------


void
QGLViewerWidget::rotate( const OpenMesh::Vec3f& _axis, float _angle )
{
  // Rotate around center center_, axis _axis, by angle _angle
  // Update modelview_matrix_

  Vec3f t( modelview_matrix_[0]*center_[0] + 
	   modelview_matrix_[4]*center_[1] +
	   modelview_matrix_[8]*center_[2] + 
	   modelview_matrix_[12],
	   modelview_matrix_[1]*center_[0] + 
	   modelview_matrix_[5]*center_[1] +
	   modelview_matrix_[9]*center_[2] + 
	   modelview_matrix_[13],
	   modelview_matrix_[2]*center_[0] + 
	   modelview_matrix_[6]*center_[1] +
	   modelview_matrix_[10]*center_[2] + 
	   modelview_matrix_[14] );
  
  makeCurrent();
  glLoadIdentity();
  glTranslatef(t[0], t[1], t[2]);
  glRotated( _angle, _axis[0], _axis[1], _axis[2]);
  glTranslatef(-t[0], -t[1], -t[2]); 
  glMultMatrixd(modelview_matrix_);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);
}


//----------------------------------------------------------------------------


bool
QGLViewerWidget::map_to_sphere( const QPoint& _v2D, OpenMesh::Vec3f& _v3D )
{
    // This is actually doing the Sphere/Hyperbolic sheet hybrid thing,
    // based on Ken Shoemake's ArcBall in Graphics Gems IV, 1993.
    double x =  (2.0*_v2D.x() - width())/width();
    double y = -(2.0*_v2D.y() - height())/height();
    double xval = x;
    double yval = y;
    double x2y2 = xval*xval + yval*yval;

    const double rsqr = TRACKBALL_RADIUS*TRACKBALL_RADIUS;
    _v3D[0] = xval;
    _v3D[1] = yval;
    if (x2y2 < 0.5*rsqr) {
        _v3D[2] = sqrt(rsqr - x2y2);
    } else {
        _v3D[2] = 0.5*rsqr/sqrt(x2y2);
    }
    
    return true;
}
//----------------------------------------------------------------------------

bool
QGLViewerWidget::map_to_far(const QPoint& _v2D, OpenMesh::Vec3f& _v3D)
{
    double x =  (2.0*_v2D.x() - width())/width();
    double y = -(2.0*_v2D.y() - height())/height();
    double xval = x;
    double yval = y;
    _v3D[0] = xval;
    _v3D[1] = yval;
    _v3D[2] = 1.0;
    return true;
}

void
QGLViewerWidget::map_to_world(const arma::fvec& nds,arma::fvec& w)
{
    arma::vec clip(4);
    clip.head(3) = arma::conv_to<arma::vec>::from(nds);
    clip(3) = 1.0;
    arma::mat proj(&projection_matrix_[0],4,4,false,true);
    arma::vec eye = arma::inv(proj)*clip;
    arma::mat mv(&modelview_matrix_[0],4,4,false,true);
    arma::vec world = arma::inv(mv)*eye;
    w = arma::fvec(3);
    w(0) = world(0)/world(3);
    w(1) = world(1)/world(3);
    w(2) = world(2)/world(3);
}


//----------------------------------------------------------------------------
void
QGLViewerWidget::update_projection_matrix()
{
  makeCurrent();
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluPerspective(45.0, (GLfloat) width() / (GLfloat) height(),
		 0.01*radius_, 100.0*radius_);
  glGetDoublev( GL_PROJECTION_MATRIX, projection_matrix_);
  glMatrixMode( GL_MODELVIEW );
  arma::fmat coordinates(&coord_[0],6,3,false,true);
  coordinates.rows(3,5) = 0.3*radius_*arma::eye<arma::fmat>(3,3);
}


//----------------------------------------------------------------------------

void
QGLViewerWidget::view_all()
{  
  translate( Vec3f( -(modelview_matrix_[0]*center_[0] + 
		      modelview_matrix_[4]*center_[1] +
		      modelview_matrix_[8]*center_[2] + 
		      modelview_matrix_[12]),
		    -(modelview_matrix_[1]*center_[0] + 
		      modelview_matrix_[5]*center_[1] +
		      modelview_matrix_[9]*center_[2] + 
		      modelview_matrix_[13]),
		    -(modelview_matrix_[2]*center_[0] + 
		      modelview_matrix_[6]*center_[1] +
		      modelview_matrix_[10]*center_[2] + 
		      modelview_matrix_[14] +
		      3.0*radius_) ) );
}


//----------------------------------------------------------------------------

void
QGLViewerWidget::set_scene_pos( const OpenMesh::Vec3f& _cog, float _radius )
{
    std::cout<<"radius:"<<_radius<<std::endl;
    center_ = _cog;
    radius_ = _radius;
    glFogf( GL_FOG_START,      1.5*_radius );
    glFogf( GL_FOG_END,        3.0*_radius );

    update_projection_matrix();
    view_all();
}


//----------------------------------------------------------------------------


QAction*
QGLViewerWidget::add_draw_mode(const std::string& _s)
{
  ++n_draw_modes_;
  draw_mode_names_.push_back(_s);

  QActionGroup *grp = draw_modes_group_;
  QAction* act = new QAction(tr(_s.c_str()), this);
  act->setCheckable(true);
  act->setData(n_draw_modes_);
  act->setShortcutContext(Qt::WidgetShortcut);

  grp->addAction(act);
  popup_menu_->addAction(act);
  addAction(act, _s.c_str());
  
  return act;
}

void QGLViewerWidget::addAction(QAction* act, const char * name)
{
    names_to_actions[name] = act;
    Super::addAction(act);
}
void QGLViewerWidget::removeAction(QAction* act)
{
    ActionMap::iterator it = names_to_actions.begin(), e = names_to_actions.end();
    ActionMap::iterator found = e;
    for(; it!=e; ++it) {
        if (it->second == act) {
            found = it;
            break;
        }
    }
    if (found != e) {
        names_to_actions.erase(found);
    }
    popup_menu_->removeAction(act);
    draw_modes_group_->removeAction(act);
    Super::removeAction(act);
}

void QGLViewerWidget::removeAction(const char* name)
{
    QString namestr = QString(name);
    ActionMap::iterator e = names_to_actions.end();

    ActionMap::iterator found = names_to_actions.find(namestr);
    if (found != e) {
        removeAction(found->second);
    }
}

QAction* QGLViewerWidget::findAction(const char* name)
{
    QString namestr = QString(name);
    ActionMap::iterator e = names_to_actions.end();
    ActionMap::iterator found = names_to_actions.find(namestr);
    if (found != e) {
        return found->second;
    }
    return NULL;
}

//----------------------------------------------------------------------------


void 
QGLViewerWidget::del_draw_mode(const std::string& _s)
{
    QAction* act = findAction(_s.c_str());
    if(act->data().toUInt()!=n_draw_modes_)
    {
        for(QList<QAction*>::reverse_iterator iter=draw_modes_group_->actions().rbegin();iter!=draw_modes_group_->actions().rend();++iter)
        {
            QAction* tmp = *iter;
            if(tmp->data().toUInt()==n_draw_modes_)
            {
                tmp->setData(act->data().toUInt());
            }
        }
    }
    removeAction(act);
    -- n_draw_modes_;
}


//----------------------------------------------------------------------------
void
QGLViewerWidget::set_draw_mode(const std::string& _s)
{
    QAction* act = findAction(_s.c_str());
    if(act){
        slotDrawMode(act);
    }
    else {
        std::cerr<<"Can't find action named:"<<_s<<std::endl;
    }
}

void
QGLViewerWidget::slotDrawMode(QAction* _mode)
{
  if( !_mode->isChecked() ) _mode->setChecked(true);//use as normal function
  draw_mode_ = _mode->data().toUInt();//use as slot
//  std::cout<<"Draw Modes:"<<std::endl;
//  for(std::vector<std::string>::iterator iter = draw_mode_names_.begin();iter!=draw_mode_names_.end();++iter)
//  {
//      std::cout<<*iter<<std::endl;
//  }
//  foreach(QAction* act,draw_modes_group_->actions())
//  {
//      std::cout<<act->data().toUInt()<<":"<<act->text().toStdString()<<std::endl;
//  }
  updateGL();
}


//----------------------------------------------------------------------------


double 
QGLViewerWidget::performance()
{
  setCursor( Qt::WaitCursor );

  double fps(0.0);

  makeCurrent();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  OpenMesh::Utils::Timer timer;

  unsigned int  frames = 60;
  const float   angle  = 360.0/(float)frames;
  unsigned int  i;
  Vec3f         axis;

  glFinish();

  timer.start();
  for (i=0, axis=Vec3f(1,0,0); i<frames; ++i)
  { rotate(axis, angle); paintGL(); swapBuffers(); }
  timer.stop();

  qApp->processEvents();

  timer.cont();
  for (i=0, axis=Vec3f(0,1,0); i<frames; ++i)
  { rotate(axis, angle); paintGL(); swapBuffers(); }
  timer.stop();

  qApp->processEvents();

  timer.cont();
  for (i=0, axis=Vec3f(0,0,1); i<frames; ++i)
  { rotate(axis, angle); paintGL(); swapBuffers(); }
  timer.stop();

  glFinish();
  timer.stop();

  glPopMatrix();
  updateGL();

  fps = ( (3.0 * frames) / timer.seconds() );

  setCursor( PointingHandCursor );

  return fps;
}


void
QGLViewerWidget::slotSnapshot( void )
{  
  QImage image;
  size_t w(width()), h(height());
  GLenum buffer( GL_BACK );

  try
  {
    image = QImage(w, h, QImage::Format_RGB32);
    std::vector<GLubyte> fbuffer(3*w*h);
    qApp->processEvents();
    makeCurrent();
    updateGL();
    glFinish();
    
    glReadBuffer( buffer );
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    paintGL();
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, &fbuffer[0] );
    
    unsigned int x,y,offset;
    
    for (y=0; y<h; ++y) {
      for (x=0; x<w; ++x) {
        offset = 3*(y*w + x);
        image.setPixel(x, h-y-1, qRgb(fbuffer[offset],
                                      fbuffer[offset+1],
                                      fbuffer[offset+2]));
      }
    }
    
    
    QString name = "snapshot-";
#if defined(_MSC_VER)
    {
      std::stringstream s;
      QDateTime         dt = QDateTime::currentDateTime();
      s << dt.date().year() 
        << std::setw(2) << std::setfill('0') << dt.date().month() 
        << std::setw(2) << std::setfill('0') << dt.date().day()
        << std::setw(2) << std::setfill('0') << dt.time().hour()
        << std::setw(2) << std::setfill('0') << dt.time().minute()
        << std::setw(2) << std::setfill('0') << dt.time().second();
      name += QString(s.str().c_str());
    }
#else
    name += QDateTime::currentDateTime().toString( "yyMMddhhmmss" );
#endif
    name += ".png";

    image.save( name, "PNG");
  }
  catch( std::bad_alloc& )
  {
    qWarning("Mem Alloc Error");
  }
  
}



//=============================================================================
