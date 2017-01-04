#ifndef __KDTREE__
#define __KDTREE__
#include <armadillo>
template<typename M>
struct MeshKDTreeInterface{
    MeshKDTreeInterface(const M&mesh):mesh_(mesh){indices_.reset();}
    MeshKDTreeInterface(const M&mesh,const arma::uvec& indices):mesh_(mesh),indices_(indices){}
    inline size_t kdtree_get_point_count() const {
        if(indices_.is_empty())return mesh_.n_vertices();
        else return indices_.size();
    }
    inline float kdtree_distance(const float *p1, const size_t idx_p2,size_t /*size*/) const
    {
        int idx;
        if(indices_.is_empty()) idx = idx_p2;
        else idx = indices_(idx_p2);
        const float* p2 = (const float*)(mesh_.points());
        const float d0=p1[0]-p2[3*idx];
        const float d1=p1[1]-p2[3*idx+1];
        const float d2=p1[2]-p2[3*idx+2];
        return std::sqrt(d0*d0+d1*d1+d2*d2);
    }
    inline float kdtree_get_pt(const size_t idx, int dim) const
    {
        size_t i;
        if(indices_.is_empty()) i = idx;
        else i = indices_(idx);
        const float* p2 = (const float*)(mesh_.points());
        return p2[3*i+dim];
    }
    template <class BBOX>
    bool kdtree_get_bbox(BBOX&)const{return false;}
    const M& mesh_;
    arma::uvec indices_;
};
template<typename Mat>
struct ArmaKDTreeInterface{
    ArmaKDTreeInterface(const Mat&p):points_(p){indices_.reset();}
    inline size_t kdtree_get_point_count() const {
        if(indices_.is_empty())return points_.n_cols;
        else return indices_.size();
    }
    inline float kdtree_distance(const float *p1, const size_t idx_p2,size_t /*size*/) const
    {
        int idx;
        if(indices_.is_empty()) idx = idx_p2;
        else idx = indices_(idx_p2);
        const float* p2 = (const float*)(points_.memptr());
        const float d0=p1[0]-p2[3*idx];
        const float d1=p1[1]-p2[3*idx+1];
        const float d2=p1[2]-p2[3*idx+2];
        return std::sqrt(d0*d0+d1*d1+d2*d2);
    }
    inline float kdtree_get_pt(const size_t idx, int dim) const
    {
        size_t i;
        if(indices_.is_empty()) i = idx;
        else i = indices_(idx);
        const float* p2 = (const float*)(points_.memptr());
        return p2[3*i+dim];
    }
    template <class BBOX>
    bool kdtree_get_bbox(BBOX&)const{return false;}
    const Mat& points_;
    arma::uvec indices_;
};
#endif // KDTREE

