#ifndef PLANEGRAPH_H
#define PLANEGRAPH_H
#include "common.h"
struct node
{
    typedef std::shared_ptr<node> Ptr;
    typedef std::vector<Ptr> PtrLst;
    int idx_;
    std::vector<int> indices_;
    std::vector<int> is_dash_;
};
class PlaneGraph
{
public:
    typedef node Node;
    PlaneGraph(const DefaultMesh& mesh,const arma::ivec& is_dash);
    arma::uvec get_side_a(int axis_id);
    arma::uvec get_side_b(int axis_id);
    std::vector<std::pair<int,arma::fvec>> axis_;
protected:
    void recover_planes(const DefaultMesh& mesh,const arma::ivec& is_dash);
    void recover_axis(const DefaultMesh& mesh, const Node&, const Node&, const arma::ivec& is_dash);
    void get_connect_points(
            const DefaultMesh& mesh,
            const Node& n0,
            const Node& n1,
            arma::fmat& c,
            arma::ivec& is_dash
            );
    bool get_axis_from_dash_points(
            const arma::fmat& connect,
            const arma::ivec& is_dash
            );
private:
    Node::PtrLst nodes_;
    arma::sp_imat edges_;
    float same_vertex_th_;
};

#endif // PLANEGRAPH_H
