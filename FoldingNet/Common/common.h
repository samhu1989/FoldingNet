#ifndef COMMON_H
#define COMMON_H
#include "common_global.h"
#include "extractmesh.hpp"
#include "MeshType.h"
#include "MeshColor.h"
#include "KDtree.hpp"
#include "Octree.hpp"
#include "configure.h"
#include "mbb.h"
#include "voxelgraph.h"
#include <cassert>
#ifndef M_PI
#  define M_PI 3.1415926535897932
#endif
namespace ColorArray {
void COMMONSHARED_EXPORT hsv2rgb(float h,float s,float v,RGB32&rgba);
}
template class COMMONSHARED_EXPORT MeshColor<DefaultMesh>;
template class COMMONSHARED_EXPORT VoxelGraph<DefaultMesh>;
template class COMMONSHARED_EXPORT MeshBundle<DefaultMesh>;
typedef MeshOctreeContainer<DefaultMesh> DefaultOctreeContainer;
template class COMMONSHARED_EXPORT unibn::Octree<arma::fvec,DefaultOctreeContainer>;
typedef unibn::Octree<arma::fvec,DefaultOctreeContainer> DefaultOctree;
void COMMONSHARED_EXPORT getRotationFromZY(const arma::fvec&,const arma::fvec&,arma::fmat&);
void COMMONSHARED_EXPORT getRotationFromXY(const arma::fvec&,const arma::fvec&,arma::fmat&);
void COMMONSHARED_EXPORT init_resouce();
arma::mat COMMONSHARED_EXPORT getRotationMatrix2D(const arma::vec& center,double theta,double scale);
arma::uvec COMMONSHARED_EXPORT randperm(arma::uword N);
inline void getRotation(float x, float y, float z, float roll, float pitch, float yaw,arma::fmat& t)
{
   float A=cosf(yaw),  B=sinf(yaw),  C=cosf(pitch), D=sinf(pitch),
         E=cosf(roll), F=sinf(roll), DE=D*E,        DF=D*F;
   t(0,0) = A*C;  t(0,1) = A*DF - B*E;  t(0,2) = B*F + A*DE;
   t(1,0) = B*C;  t(1,1) = A*E + B*DF;  t(1,2) = B*DE - A*F;
   t(2,0) = -D;   t(2,1) = C*F;         t(2,2) = C*E;
}

inline void fitPlane(
        arma::fvec &center,
        arma::fmat &neighbor,
        arma::fvec &normal,
        float &curvature,
        float& distToOrigin)
{
    neighbor.each_col() -= center;
    arma::fmat U,V;
    arma::fvec s;
    arma::fmat A = neighbor*neighbor.t();
    if(!arma::svd(U,s,V,A,"std"))
    {
        normal.fill(std::numeric_limits<float>::quiet_NaN());
        distToOrigin = std::numeric_limits<float>::quiet_NaN();
        curvature = std::numeric_limits<float>::quiet_NaN();
    }else{
        arma::uword minidx;
        curvature = s.min(minidx);
        normal = arma::normalise(V.col(minidx));
        distToOrigin = - arma::dot(normal,center);
    }
}
inline void fitPlane(
        arma::fmat &neighbor,
        arma::fvec &coeff
        )
{
    coeff = arma::fvec(4);
    arma::fmat center = arma::mean(neighbor,1);
    neighbor.each_col() -= center;
    arma::fmat U,V;
    arma::fvec s;
    arma::fmat A = neighbor*neighbor.t();
    if(!arma::svd(U,s,V,A,"std"))
    {
        coeff.fill(std::numeric_limits<float>::quiet_NaN());
    }else{
        arma::uword minidx;
        s.min(minidx);
        coeff.head(3) = arma::normalise(V.col(minidx));
        coeff(3) = - arma::dot(coeff.head(3),center);
    }
}
#include <fn_eigs_sym_custom.hpp>
#endif // COMMON_H
