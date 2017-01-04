#ifndef MESHTYPE_H
#define MESHTYPE_H
#include <memory>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Utils/StripifierT.hh>
#include <armadillo>
#include "MeshColor.h"
#include "voxelgraph.h"
struct Traits : public OpenMesh::DefaultTraits
{
  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge);
};

typedef OpenMesh::PolyMesh_ArrayKernelT<Traits>  DefaultMesh;

template<typename M>
class MeshBundle
{
public:
    typedef typename std::shared_ptr<MeshBundle<M>> Ptr;
    typedef typename std::vector<Ptr> PtrList;
    typedef typename std::shared_ptr<M> MeshPtr;
    MeshBundle():
        custom_color_(mesh_),
        graph_(mesh_),
        strips_(mesh_)
    {}
    MeshPtr mesh_ptr(){return std::shared_ptr<M>(&mesh_);}
    std::string name_;
    arma::fmat                  p_feature_;
    M                           mesh_;
    MeshColor<M>        custom_color_;
    VoxelGraph<M>              graph_;
    OpenMesh::StripifierT<M>  strips_;
};
#endif // MESHTYPE

