#include "planegraph.h"
#include <QRgb>
PlaneGraph::PlaneGraph(const DefaultMesh& mesh,const arma::ivec& is_dash)
{
    assert(mesh.n_vertices()==is_dash.size());
    recover_planes(mesh,is_dash);
}

void PlaneGraph::recover_planes(const DefaultMesh& mesh,const arma::ivec& is_dash)
{
    __gnu_cxx::hash_map<uint32_t,uint32_t> color_to_plane;
    for(DefaultMesh::VertexIter v_it=mesh.vertices_sbegin(); v_it!=mesh.vertices_end(); ++v_it)
    {
        DefaultMesh::Color c = mesh.color(*v_it);
        uint8_t* p = (uint8_t*)c.data();
        uint32_t ckey = qRgb(p[0],p[1],p[2]);
        if(color_to_plane.end()==color_to_plane.find(ckey))
        {
            color_to_plane[ckey] = nodes_.size();
            nodes_.emplace_back();
        }
        uint32_t plane_idx = color_to_plane[ckey];
        if(!nodes_[plane_idx])nodes_[plane_idx].reset(new PlaneGraph::Node());
        nodes_[plane_idx]->indices_.push_back(v_it->idx());
        nodes_[plane_idx]->is_dash_.push_back(is_dash(v_it->idx()));
    }
    std::cerr<<nodes_.size()<<" planes found"<<std::endl;
}
