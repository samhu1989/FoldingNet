#ifndef REGION_H
#define REGION_H
#include <vector>
#include "LineSegment.h"
struct Plane
{
    typedef std::shared_ptr<Plane> Ptr;
    typedef std::vector<Ptr> PtrLst;
    std::vector<int> line_id_lst_;
    int id_;
};

struct Region
{
    typedef std::shared_ptr<Region> Ptr;
    typedef std::vector<Ptr> PtrLst;
    inline size_t size(){return end_plane_ - start_plane_;}
    int start_plane_;
    int end_plane_;
    Plane::PtrLst plane_lst_;
    arma::sp_imat plane_connetion_;
};

#endif // REGION_H
