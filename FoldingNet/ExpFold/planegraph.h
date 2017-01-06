#ifndef PLANEGRAPH_H
#define PLANEGRAPH_H
#include "common.h"
struct node
{
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
private:

    std::vector<std::shared_ptr<Node>> nodes_;
    arma::sp_mat edges_;
};

#endif // PLANEGRAPH_H
