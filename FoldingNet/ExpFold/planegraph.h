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
    PlaneGraph(
            const DefaultMesh& mesh,
            const arma::sp_imat& connection,
            const arma::Col<int>& dash,
            float th
            );
    arma::uvec get_side_a(int axis_id,std::vector<int>& side_a_axis);
    arma::uvec get_side_b(int axis_id, std::vector<int>& side_b_axis);
    void test_connect_points(int axis_id);
    arma::uvec get_side_a_dash(int axis_id);
    arma::uvec get_side_b_dash(int axis_id);
    std::vector<std::pair<int,arma::fvec>> axis_;
protected:
    void recover_planes(
            const DefaultMesh& mesh,
            const arma::sp_imat& connection,
            const arma::Col<int>& dash
            );
    void recover_axis(
            const DefaultMesh& mesh,
            const Node&,
            const Node&,
            const arma::sp_imat& connection,
            const arma::Col<int>& dash
            );
    void get_connect_points(const DefaultMesh& mesh,
            const Node& n0,
            const Node& n1,
            arma::fmat& c,
            arma::Col<int> &is_dash
            );
    bool get_axis_from_points(
            const arma::fmat& connect,
            const arma::Col<int> &is_dash
            );
private:
    int start_plane_;
    Node::PtrLst nodes_;
    arma::sp_imat edges_;
    float same_vertex_th_;
    const DefaultMesh& mesh_;
    const arma::sp_imat& connection_;
    const arma::Col<int> dash_;
};

#endif // PLANEGRAPH_H
