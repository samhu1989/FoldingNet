#ifndef VOXELGRAPH_H
#define VOXELGRAPH_H
#include <armadillo>
#include <memory>
template <typename M>
class VoxelGraph
{
public:
    typedef M Mesh;
    typedef std::shared_ptr<VoxelGraph<M>> Ptr;
    VoxelGraph(M&m):Ref_(m){}
    static Ptr getSubGraphPtr(const VoxelGraph&,const arma::uvec&,Mesh&);
    bool empty(void)
    {
        return ( Ref_.n_vertices() != voxel_label.size() ) || ( 0==voxel_centers.n_cols );
    }
    size_t size(void)const
    {
        return voxel_centers.n_cols;
    }
    bool save(const std::string&);
    bool load(const std::string&);
    void sv2pix(const arma::uvec& sv,arma::uvec& pix);//supervoxel label to pixel label
    void sv2pix(arma::Col<uint32_t>&,arma::Col<uint32_t>&);//supervoxel color to pixel color
    void sv2pix(arma::Mat<uint8_t>&,arma::Mat<uint8_t>&);//supervoxel color to pixel color
    void getSvIndex(const arma::uvec& pix,arma::uvec& sv);//pixel index to supervoxel index-result index start from one input index start from zero
    void getSvIndexv0(const arma::uvec& pix,arma::uvec& sv);//pixel index to supervoxel index
    void getPixIndex(const arma::uvec& sv,arma::uvec& pix);//supervoxel index to pixel index
    void getPixFunc(const arma::vec& voxFunc,arma::vec& pixFunc);
    void getVoxFunc(const arma::vec& pixFunc,arma::vec& voxFunc);
    void match(M&mesh,
            arma::fvec &gscore,
            arma::fvec &nscore,
            arma::fvec &cscore,
            arma::vec&score,
            double dist_th=0.05,
            double color_var=30.0
            );
    void match2(M&mesh,
            arma::fvec &gscore,
            arma::fvec &nscore,
            arma::fvec &cscore,
            arma::vec&score,
            double dist_th=0.05,
            double color_var=30.0
            );
    double voxel_similarity(size_t v1,size_t v2,double dist_th=0.05,double color_var=30.0);
    double voxel_similarity2(size_t v1,size_t v2,double dist_th=0.05,double color_var=30.0);
    inline void get_XYZLab(arma::fmat&voxels,const arma::uvec&indices = arma::uvec());
    inline void get_Lab(arma::fmat&voxels,const arma::uvec&indices = arma::uvec());

    arma::fmat voxel_centers;
    arma::Mat<uint8_t> voxel_colors;
    arma::fmat voxel_normals;
    arma::uvec voxel_size;
    arma::uvec voxel_label;//start from one
    arma::Mat<uint16_t> voxel_neighbors;//start from zero
    arma::Mat<uint8_t> voxel_edge_colors;
private:
    const Mesh& Ref_;//the ref Mesh that bound with this custom color
};

#endif // VOXELGRAPH_H
