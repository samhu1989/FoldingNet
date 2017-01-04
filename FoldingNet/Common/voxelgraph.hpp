#include "voxelgraph.h"
#include "common.h"
#include "nanoflann.hpp"
#include "KDtree.hpp"
#include "extractmesh.hpp"
#include "MeshType.h"
#include <hash_fun.h>
#include <ext/hash_map>
template <typename M>
bool VoxelGraph<M>::save(const std::string&path)
{
    if(!voxel_centers.save(path+"\\centers.fvec.arma",arma::arma_binary))return false;
    if(!voxel_normals.save(path+"\\normals.fvec.arma",arma::arma_binary))return false;
    if(!voxel_colors.save(path+"\\colors.Mat_uint8_t.arma",arma::arma_binary))return false;
    if(!voxel_size.save(path+"\\sizes.uvec.arma",arma::arma_binary))return false;
    if(!voxel_neighbors.save(path+"\\neighbors.Mat_uint16_t.arma",arma::arma_binary))return false;
    if(!voxel_label.save(path+"\\labels.uvec.arma",arma::arma_binary))return false;
    M output;
    for( int i=0 ; i < voxel_centers.n_cols ; ++i )
    {
        output.add_vertex(typename M::Point(voxel_centers(0,i),voxel_centers(1,i),voxel_centers(2,i)));
    }
    output.request_vertex_colors();
    arma::Mat<uint8_t> cOut((uint8_t*)output.vertex_colors(),3,output.n_vertices(),false,true);
    cOut = voxel_colors;
    output.request_vertex_normals();
    arma::fmat nOut((float*)output.vertex_normals(),3,output.n_vertices(),false,true);
    nOut = voxel_normals;
    OpenMesh::IO::Options opt;
    opt+=OpenMesh::IO::Options::Binary;
    opt+=OpenMesh::IO::Options::VertexColor;
    opt+=OpenMesh::IO::Options::VertexNormal;
    OpenMesh::IO::write_mesh(output,path+"\\mesh.ply",opt,13);
    return true;
}

template <typename M>
bool VoxelGraph<M>::load(const std::string&path)
{
    if(!voxel_centers.load(path+"\\centers.fvec.arma"))return false;
    if(!voxel_normals.load(path+"\\normals.fvec.arma"))return false;
    if(!voxel_colors.load(path+"\\colors.Mat_uint8_t.arma"))return false;
    if(!voxel_size.load(path+"\\sizes.uvec.arma"))return false;
    if(!voxel_neighbors.load(path+"\\neighbors.Mat_uint16_t.arma"))return false;
    if(!voxel_label.load(path+"\\labels.uvec.arma"))return false;
    if(voxel_centers.n_cols!=voxel_size.size())return false;
    if(voxel_centers.n_cols!=voxel_colors.n_cols)return false;
    if(voxel_centers.n_cols!=voxel_normals.n_cols)return false;
    return true;
}

template <typename M>
void VoxelGraph<M>::sv2pix(const arma::uvec& sv,arma::uvec& pix)
{
    if(sv.size()!=voxel_centers.n_cols){
        std::cerr<<"Can't translate a supervoxel label that was not based on this graph"<<std::endl;
        throw std::logic_error("sv.size()!=voxel_centers.n_cols");
    }
    if(pix.size()!=voxel_label.size())pix = arma::uvec(voxel_label.size(),arma::fill::zeros);
    else pix.fill(0);
    for(int l = 1 ; l <= voxel_centers.n_cols ; ++l )
    {
        arma::uvec indices = arma::find(voxel_label==l);
        pix(indices).fill( sv(l-1) );
    }
}

template <typename M>
void VoxelGraph<M>::sv2pix(arma::Col<uint32_t>&sv,arma::Col<uint32_t>&pix)
{
    arma::uvec vl = voxel_label;
    vl -= arma::uvec(vl.size(),arma::fill::ones);
    arma::uvec out_of_bounds = arma::find( vl >= sv.size() || vl < 0 );
    if(!out_of_bounds.is_empty())std::logic_error("!out_of_bounds.is_empty()");
    if(pix.size()!=vl.size())std::logic_error("pix.size()!=vl.size()");
    pix = sv(vl);
}
template <typename M>
void VoxelGraph<M>::sv2pix(arma::Mat<uint8_t>&sv,arma::Mat<uint8_t>&pix)
{
    arma::uvec vl = voxel_label;
    vl -= arma::uvec(vl.size(),arma::fill::ones);
    arma::uvec out_of_bounds = arma::find( vl >= sv.size() || vl < 0 );
    if(!out_of_bounds.is_empty())std::logic_error("!out_of_bounds.is_empty()");
    if(pix.size()!=vl.size())std::logic_error("pix.size()!=vl.size()");
    pix.rows(0,2) = sv.cols(vl);
    pix.row(3).fill(255);
}

template <typename M>
typename VoxelGraph<M>::Ptr VoxelGraph<M>::getSubGraphPtr(const VoxelGraph& parent_graph,const arma::uvec& indices,Mesh& sub_mesh)
{
    extractMesh<M,M>(parent_graph.Ref_,indices,sub_mesh);
    typename VoxelGraph<M>::Ptr sub_graph(new VoxelGraph<M>(sub_mesh));
    size_t Nsv = parent_graph.voxel_centers.n_cols;
    arma::sp_umat neighborhood(Nsv,Nsv);
    for(size_t i = 0 ; i < parent_graph.voxel_neighbors.n_cols ; ++i)
    {
        assert(parent_graph.voxel_neighbors(0,i)<Nsv);
        assert(parent_graph.voxel_neighbors(1,i)<Nsv);
        if( parent_graph.voxel_neighbors(0,i) <= parent_graph.voxel_neighbors(1,i) )neighborhood( parent_graph.voxel_neighbors(0,i) , parent_graph.voxel_neighbors(1,i) ) = 1;
        else neighborhood(parent_graph.voxel_neighbors(1,i),parent_graph.voxel_neighbors(0,i)) = 1;
    }
    __gnu_cxx::hash_map<int,int> voxel_map;
    std::vector<size_t> voxel_added;
    sub_graph->voxel_label = arma::uvec(indices.size());
    for(size_t i = 0 ; i < indices.size() ; ++ i )
    {
        size_t c = indices(i);
        size_t sv = parent_graph.voxel_label(c) - 1;
        if(voxel_map.find(sv)==voxel_map.end())
        {
            size_t N = sub_graph->voxel_centers.n_cols;
            voxel_map[sv] = N;
            voxel_added.push_back(sv);
            sub_graph->voxel_centers.insert_cols( N , parent_graph.voxel_centers.col(sv) );
            sub_graph->voxel_colors.insert_cols( N , parent_graph.voxel_colors.col(sv) );
            sub_graph->voxel_normals.insert_cols( N , parent_graph.voxel_normals.col(sv) );
            sub_graph->voxel_size.insert_rows(N,1);
            sub_graph->voxel_size(N) = 1;
            sub_graph->voxel_label(i) = N + 1;
        }else{
            sub_graph->voxel_size( voxel_map[sv] ) += 1;
            sub_graph->voxel_label(i) = voxel_map[sv] + 1;
        }
    }
//    std::cerr<<"neighbors"<<std::endl;
    std::sort(voxel_added.begin(),voxel_added.end(),std::less<size_t>());
    std::vector<size_t>::iterator iter;
    std::vector<size_t>::iterator niter;
    for(iter=voxel_added.begin();iter!=voxel_added.end();++iter)
    {
        for( niter = iter + 1 ; niter!=voxel_added.end() ; ++niter)
        {
            if(*iter<*niter)std::logic_error("*iter<*niter");
            if(*iter >= neighborhood.n_cols)std::logic_error("*iter >= neighborhood.n_cols");
            if(*niter >= neighborhood.n_rows)std::logic_error("*niter >= neighborhood.n_rows");
            if(1!=neighborhood(*iter,*niter))continue;
            size_t N_neighbor = sub_graph->voxel_neighbors.n_cols;
            if(sub_graph->voxel_neighbors.is_empty())
            {
                sub_graph->voxel_neighbors = arma::Mat<uint16_t>(2,1);
            }else{
                sub_graph->voxel_neighbors.insert_cols(N_neighbor,1);
            }
            size_t n0 = voxel_map[*iter];
            size_t n1 = voxel_map[*niter];
            if( n0 < n1 )
            {
                sub_graph->voxel_neighbors(0,N_neighbor) = n0;
                sub_graph->voxel_neighbors(1,N_neighbor) = n1;
            }else{
                sub_graph->voxel_neighbors(0,N_neighbor) = n1;
                sub_graph->voxel_neighbors(1,N_neighbor) = n0;
            }
        }
    }
//    std::cerr<<"done"<<std::endl;
    return sub_graph;
}
//resulted index start from one
template <typename M>
void VoxelGraph<M>::getSvIndex(const arma::uvec& pix,arma::uvec& sv)
{
    if( arma::max(pix)>voxel_label.size()-1 )
    {
        std::cerr<<"void VoxelGraph<M>::getSvIndex(invalid pixel indices)"<<std::endl;
    }
    sv = voxel_label(pix);
    __gnu_cxx::hash_map<int,int> order_hash;
    int index = 0;
    std::vector<arma::uword> sv_index;
    sv_index.reserve(sv.size());
    for(arma::uvec::iterator iter = sv.begin() ; iter != sv.end() ; ++iter )
    {
        if(order_hash.find(int(*iter))==order_hash.end())
        {
            order_hash[int(*iter)]= index;
            sv_index.push_back(arma::uword(*iter));
            ++index;
        }
    }
    sv = arma::uvec(sv_index);
}

template <typename M>
void VoxelGraph<M>::getSvIndexv0(const arma::uvec& pix,arma::uvec& sv)
{
    if( arma::max(pix)>voxel_label.size()-1 )
    {
        std::cerr<<"void VoxelGraph<M>::getSvIndex(invalid pixel indices)"<<std::endl;
    }
    sv = voxel_label(pix);
    std::vector<arma::uword> sv_index = arma::conv_to<std::vector<arma::uword>>::from(sv);
    std::sort(sv_index.begin(),sv_index.end());
    sv_index.erase(std::unique(sv_index.begin(),sv_index.end()),sv_index.end());
    sv = arma::uvec(sv_index);
}

template <typename M>
void VoxelGraph<M>::getPixIndex(const arma::uvec& sv,arma::uvec& pix)
{
    if( arma::max(sv) >= voxel_centers.n_cols )
    {
        std::cerr<<"void VoxelGraph<M>::getPixIndex(invalid supervoxel indices)"<<std::endl;
    }
    pix.clear();
    for(arma::uword i=0;i<sv.size();++i)
    {
        arma::uword k = sv(i);
        pix = arma::join_cols(pix,arma::find(voxel_label==k));
    }
}

template <typename M>
void VoxelGraph<M>::getPixFunc(const arma::vec& voxFunc,arma::vec& pixFunc)
{
    pixFunc = arma::vec(Ref_.n_vertices(),arma::fill::zeros);
    arma::uvec indices = voxel_label;
    arma::uvec zidx = arma::find(indices>0);
    indices(zidx) -= 1;
    pixFunc = voxFunc( indices );
}

template <typename M>
void VoxelGraph<M>::getVoxFunc(const arma::vec& pixFunc,arma::vec& voxFunc)
{
    voxFunc = arma::vec(size(),arma::fill::zeros);
    arma::vec voxCunt = arma::vec(size(),arma::fill::zeros);
    const arma::uvec& indices = voxel_label;
    arma::vec::const_iterator piter = pixFunc.cbegin();
    for(arma::uvec::const_iterator iiter=indices.cbegin();iiter!=indices.cend();++iiter)
    {
        if(*iiter==0){
            ++piter;
            if( piter == pixFunc.cend() )break;
            continue;
        }
        assert( (*iiter >= 1) && (*iiter <= voxFunc.size()) );
        voxFunc( (*iiter - 1) ) += *piter;
        voxCunt( (*iiter - 1) ) += 1.0;
        ++ piter;
        if( piter == pixFunc.cend() )break;
    }
    voxFunc /= voxCunt;
}

using namespace nanoflann;
template <typename M>
void VoxelGraph<M>::match(
        M&mesh,
        arma::fvec&gscore,
        arma::fvec&nscore,
        arma::fvec&cscore,
        arma::vec&score,
        double dist_th,
        double color_var
        )
{
    MeshKDTreeInterface<M> points(mesh);
    KDTreeSingleIndexAdaptor<
            L2_Simple_Adaptor<float,MeshKDTreeInterface<M>>,
            MeshKDTreeInterface<M>,
            3,arma::uword>
            kdtree(3,points,KDTreeSingleIndexAdaptorParams(5));
    kdtree.buildIndex();
    size_t sv_N = voxel_centers.n_cols;
    arma::uvec search_idx(5);
    arma::fvec search_dist(5);

    float max_geo_score = arma::max(gscore);
    arma::vec sv_match_score(sv_N,arma::fill::zeros);
    arma::vec sv_geo_score(sv_N,arma::fill::zeros);
    arma::vec sv_norm_score(sv_N,arma::fill::zeros);
    arma::vec sv_color_score(sv_N,arma::fill::zeros);
    score.resize(sv_N);

    float* pts = (float*)Ref_.points();
    arma::Mat<uint8_t> ref_c_mat((uint8_t*)Ref_.vertex_colors(),3,Ref_.n_vertices(),false,true);
    arma::Mat<uint8_t> m_c_mat((uint8_t*)mesh.vertex_colors(),3,mesh.n_vertices(),false,true);
    arma::fmat ref_n_mat((float*)Ref_.vertex_normals(),3,Ref_.n_vertices(),false,true);
    arma::fmat m_n_mat((float*)mesh.vertex_normals(),3,mesh.n_vertices(),false,true);
    double min_dist = std::numeric_limits<double>::max();
//    std::cerr<<"match 1"<<std::endl;
    for( size_t p_i = 0 ; p_i < Ref_.n_vertices() ; ++ p_i )
    {
        kdtree.knnSearch(&pts[3*p_i],5,search_idx.memptr(),search_dist.memptr());
        arma::fvec current_c;
        ColorArray::RGB2Lab(ref_c_mat.col(p_i),current_c);
        arma::fmat nearest_c;
        ColorArray::RGB2Lab(m_c_mat.cols(search_idx),nearest_c);
        nearest_c.each_col() -= current_c;
        arma::frowvec color_dist = arma::sqrt(arma::sum(arma::square(nearest_c)));
        arma::uword min_idx;
        color_dist.min(min_idx);
        arma::uword sv_idx = voxel_label(p_i) - 1;
        arma::uword match_idx = search_idx(min_idx);
        if(match_idx>gscore.size())std::logic_error("match_idx>gscore.size()");
        if(match_idx>cscore.size())std::logic_error("match_idx>cscore.size()");
        arma::fvec current_n = ref_n_mat.col(p_i);
        arma::fvec nearest_n = m_n_mat.col(match_idx);
        if( search_dist(min_idx) < dist_th )
        {
            double normal_sim = 1.0;
//            if(current_n.is_finite()&&nearest_n.is_finite())
//            {
//                normal_sim = std::abs(arma::dot(current_n,nearest_n));
//            }
            sv_match_score(sv_idx) += normal_sim / ( 1.0 + search_dist(min_idx) / dist_th ) / (1.0+ ( color_dist(min_idx) / color_var ));
        }
        if( min_dist > search_dist(min_idx) ) min_dist = search_dist(min_idx);
        if( gscore[match_idx] >= 0.6*max_geo_score)sv_geo_score(sv_idx) += gscore[match_idx];
        sv_norm_score(sv_idx) += nscore[match_idx];
        sv_color_score(sv_idx) += cscore[match_idx];
    }
//    std::cerr<<"match 2"<<std::endl;
    if( min_dist > dist_th )std::cerr<<"min_dist:"<<min_dist<<std::endl;
    for(size_t sv_i = 0 ; sv_i < voxel_centers.n_cols ; ++ sv_i )
    {
        assert(voxel_size(sv_i)>0);
        sv_match_score(sv_i) /= voxel_size(sv_i);
        sv_geo_score(sv_i) /= voxel_size(sv_i);
        sv_norm_score(sv_i) /= voxel_size(sv_i);
        sv_color_score(sv_i) /= voxel_size(sv_i);
    }
//    double match_max = arma::max(sv_match_score);
//    if(match_max!=0.0)sv_match_score /= match_max;
//    else {
//        std::cerr<<"All zeros in sv_match_score with dist th="<<dist_th<<std::endl;
//    }
//    sv_geo_score /= arma::max(sv_geo_score);
//    sv_color_score /= arma::max(sv_color_score);
//    std::cerr<<"match 3"<<std::endl;
    if(!sv_match_score.is_finite())std::cerr<<"Infinite in sv_match_score"<<std::endl;
    for(size_t sv_i = 0 ; sv_i < voxel_centers.n_cols ; ++ sv_i )
    {
        if(sv_norm_score(sv_i)<0)std::logic_error("sv_norm_score(sv_i)<0");
//        score(sv_i) = sv_match_score(sv_i)*sv_geo_score(sv_i)*sv_norm_score(sv_i)*sv_color_score(sv_i);
        score(sv_i) = sv_match_score(sv_i)*sv_geo_score(sv_i)*sv_norm_score(sv_i);
//        score(sv_i) = sv_match_score(sv_i)*sv_geo_score(sv_i);
    }
}


template <typename M>
void VoxelGraph<M>::match2(
        M&mesh,
        arma::fvec&gscore,
        arma::fvec&nscore,
        arma::fvec&cscore,
        arma::vec&score,
        double dist_th,
        double color_var
        )
{
    MeshKDTreeInterface<M> points(mesh);
    KDTreeSingleIndexAdaptor<
            L2_Simple_Adaptor<float,MeshKDTreeInterface<M>>,
            MeshKDTreeInterface<M>,
            3,arma::uword>
            kdtree(3,points,KDTreeSingleIndexAdaptorParams(5));
    kdtree.buildIndex();
    size_t sv_N = voxel_centers.n_cols;
    arma::uvec search_idx(5);
    arma::fvec search_dist(5);

    arma::vec sv_match_score(sv_N,arma::fill::zeros);
    arma::vec sv_geo_score(sv_N,arma::fill::zeros);
    arma::vec sv_norm_score(sv_N,arma::fill::zeros);
    arma::vec sv_color_score(sv_N,arma::fill::zeros);
    score.resize(sv_N);

    float* pts = (float*)Ref_.points();
    arma::Mat<uint8_t> ref_c_mat((uint8_t*)Ref_.vertex_colors(),3,Ref_.n_vertices(),false,true);
    arma::Mat<uint8_t> m_c_mat((uint8_t*)mesh.vertex_colors(),3,mesh.n_vertices(),false,true);
    arma::fmat ref_n_mat((float*)Ref_.vertex_normals(),3,Ref_.n_vertices(),false,true);
    arma::fmat m_n_mat((float*)mesh.vertex_normals(),3,mesh.n_vertices(),false,true);
    double min_dist = std::numeric_limits<double>::max();
//    std::cerr<<"match 1"<<std::endl;
    for( size_t p_i = 0 ; p_i < Ref_.n_vertices() ; ++ p_i )
    {
        kdtree.knnSearch(&pts[3*p_i],5,search_idx.memptr(),search_dist.memptr());
        arma::fvec current_c;
        ColorArray::RGB2Lab(ref_c_mat.col(p_i),current_c);
        arma::fmat nearest_c;
        ColorArray::RGB2Lab(m_c_mat.cols(search_idx),nearest_c);
        nearest_c.each_col() -= current_c;
        arma::frowvec color_dist = arma::sqrt(arma::sum(arma::square(nearest_c)));
        arma::uword min_idx;
        color_dist.min(min_idx);
        arma::uword sv_idx = voxel_label(p_i) - 1;
        arma::uword match_idx = search_idx(min_idx);
        if(match_idx>gscore.size())std::logic_error("match_idx>gscore.size()");
        if(match_idx>cscore.size())std::logic_error("match_idx>cscore.size()");
        arma::fvec current_n = ref_n_mat.col(p_i);
        arma::fvec nearest_n = m_n_mat.col(match_idx);
        if( search_dist(min_idx) < dist_th )
        {
            double normal_sim = 1.0;
//            if(current_n.is_finite()&&nearest_n.is_finite())
//            {
//                normal_sim = std::abs(arma::dot(current_n,nearest_n));
//            }
            sv_match_score(sv_idx) += normal_sim / (1.0+search_dist(min_idx) / dist_th) / ( 1.0+ ( color_dist(min_idx) / 255 ));
        }
        if( min_dist > search_dist(min_idx) ) min_dist = search_dist(min_idx);
        sv_geo_score(sv_idx) += gscore[match_idx];
        sv_norm_score(sv_idx) += nscore[match_idx];
        sv_color_score(sv_idx) += cscore[match_idx];
    }
//    std::cerr<<"match 2"<<std::endl;
    if( min_dist > dist_th )std::cerr<<"min_dist:"<<min_dist<<std::endl;
    for(size_t sv_i = 0 ; sv_i < voxel_centers.n_cols ; ++ sv_i )
    {
        assert(voxel_size(sv_i)>0);
        sv_match_score(sv_i) /= voxel_size(sv_i);
        sv_geo_score(sv_i) /= voxel_size(sv_i);
        sv_norm_score(sv_i) /= voxel_size(sv_i);
        sv_color_score(sv_i) /= voxel_size(sv_i);
    }
//    double match_max = arma::max(sv_match_score);
//    if(match_max!=0.0)sv_match_score /= match_max;
//    else {
//        std::cerr<<"All zeros in sv_match_score with dist th="<<dist_th<<std::endl;
//    }
//    sv_geo_score /= arma::max(sv_geo_score);
//    sv_color_score /= arma::max(sv_color_score);
//    std::cerr<<"match 3"<<std::endl;
    if(!sv_match_score.is_finite())std::cerr<<"Infinite in sv_match_score"<<std::endl;
    for(size_t sv_i = 0 ; sv_i < voxel_centers.n_cols ; ++ sv_i )
    {
        if(sv_norm_score(sv_i)<0)std::logic_error("sv_norm_score(sv_i)<0");
//        score(sv_i) = sv_match_score(sv_i)*sv_geo_score(sv_i)*sv_norm_score(sv_i)*sv_color_score(sv_i);
        score(sv_i) = sv_match_score(sv_i)*sv_geo_score(sv_i)*sv_norm_score(sv_i);
//        score(sv_i) = sv_match_score(sv_i)*sv_geo_score(sv_i);
    }
}

template <typename M>
double VoxelGraph<M>::voxel_similarity(size_t v1,size_t v2,double dist_th,double color_var)
{
    if(v1>=voxel_centers.n_cols)std::logic_error("v1>=voxel_centers.n_cols");
    if(v2>=voxel_centers.n_cols)std::logic_error("v2>=voxel_centers.n_cols");
    //use the distance between closest point as distance
    double spatial_dist = std::numeric_limits<double>::max();
    arma::fmat pts((float*)Ref_.points(),3,Ref_.n_vertices(),false,true);
    arma::uvec idx1 = arma::find( voxel_label == (v1+1) );
    arma::uvec idx2 = arma::find( voxel_label == (v2+1) );
    if(idx1.is_empty())std::logic_error("idx1.is_empty()");
    if(idx2.is_empty())std::logic_error("idx2.is_empty()");
    arma::fmat pts1;
    arma::fmat pts2;
    if( idx1.size() > idx2.size() )
    {
        pts1 = pts.cols(idx1);
        pts2 = pts.cols(idx2);
    }else{
        pts1 = pts.cols(idx2);
        pts2 = pts.cols(idx1);
    }
    ArmaKDTreeInterface<arma::fmat> search_pts(pts1);
    KDTreeSingleIndexAdaptor<
            L2_Simple_Adaptor<float,ArmaKDTreeInterface<arma::fmat>>,
            ArmaKDTreeInterface<arma::fmat>,
            3,arma::uword>
            kdtree(3,search_pts,KDTreeSingleIndexAdaptorParams(1));
    kdtree.buildIndex();
    float* p = (float*)pts2.memptr();
    arma::uword i;
    float d;
    for( size_t pi = 0 ; pi < pts2.n_cols ; ++ pi )
    {
        kdtree.knnSearch(&p[3*pi],1,&i,&d);
        if( d < spatial_dist )spatial_dist = d;
    }
    double color_dist;
    arma::fvec Lab1;
    arma::fvec ab1(2);
    ColorArray::RGB2Lab(voxel_colors.col(v1),Lab1);
    ab1(0) = Lab1(1);ab1(1) = Lab1(2);
    arma::fvec Lab2;
    arma::fvec ab2(2);
    ColorArray::RGB2Lab(voxel_colors.col(v2),Lab2);
    ab2(0) = Lab2(1);ab2(1) = Lab2(2);
    color_dist = arma::norm( ab1 - ab2 );

    arma::fvec norm1 = voxel_normals.col(v1);
    arma::fvec norm2 = voxel_normals.col(v2);
    double normal_sim = 1.0;
    if(norm1.is_finite()&&norm2.is_finite())
    {
        normal_sim = arma::dot(norm1,norm2);
    }
    if( spatial_dist > dist_th ) return 0.0;
//    else if( normal_sim > 0.99 ) return 2.0;
    else return normal_sim / (1.0 + spatial_dist / dist_th ) / (1.0+color_dist/color_var);
//    return normal_sim / (1.0+spatial_dist) ;
}

template <typename M>
double VoxelGraph<M>::voxel_similarity2(size_t v1,size_t v2,double dist_th,double color_var)
{
    if(v1>=voxel_centers.n_cols)std::logic_error("v1>=voxel_centers.n_cols");
    if(v2>=voxel_centers.n_cols)std::logic_error("v2>=voxel_centers.n_cols");
    //use the distance between closest point as distance
    double spatial_dist = std::numeric_limits<double>::max();
    arma::fmat pts((float*)Ref_.points(),3,Ref_.n_vertices(),false,true);
    arma::uvec idx1 = arma::find( voxel_label == (v1+1) );
    arma::uvec idx2 = arma::find( voxel_label == (v2+1) );
    if(idx1.is_empty())std::logic_error("idx1.is_empty()");
    if(idx2.is_empty())std::logic_error("idx2.is_empty()");
    arma::fmat pts1;
    arma::fmat pts2;
    if( idx1.size() > idx2.size() )
    {
        pts1 = pts.cols(idx1);
        pts2 = pts.cols(idx2);
    }else{
        pts1 = pts.cols(idx2);
        pts2 = pts.cols(idx1);
    }
    ArmaKDTreeInterface<arma::fmat> search_pts(pts1);
    KDTreeSingleIndexAdaptor<
            L2_Simple_Adaptor<float,ArmaKDTreeInterface<arma::fmat>>,
            ArmaKDTreeInterface<arma::fmat>,
            3,arma::uword>
            kdtree(3,search_pts,KDTreeSingleIndexAdaptorParams(1));
    kdtree.buildIndex();
    float* p = (float*)pts2.memptr();
    arma::uword i;
    float d;
    for( size_t pi = 0 ; pi < pts2.n_cols ; ++ pi )
    {
        kdtree.knnSearch(&p[3*pi],1,&i,&d);
        if( d < spatial_dist )spatial_dist = d;
    }
    double color_dist;
    arma::fvec Lab1;
    arma::fvec ab1(2);
    ColorArray::RGB2Lab(voxel_colors.col(v1),Lab1);
    ab1(0) = Lab1(1);ab1(1) = Lab1(2);
    arma::fvec Lab2;
    arma::fvec ab2(2);
    ColorArray::RGB2Lab(voxel_colors.col(v2),Lab2);
    ab2(0) = Lab2(1);ab2(1) = Lab2(2);
    color_dist = arma::norm( ab1 - ab2 );

    arma::fvec dir = voxel_centers.col(v2) - voxel_centers.col(v1);
    dir = arma::normalise(dir);
    arma::fvec norm1 = voxel_normals.col(v1);
    arma::fvec norm2 = voxel_normals.col(v2);
    double normal_sim = 1.0;
    if(norm1.is_finite()&&norm2.is_finite())
    {
        float theta1 = std::acos(arma::dot(norm1,dir));
        float theta2 = std::acos(arma::dot(norm2,dir));
        if( ( theta2 - theta1 ) > ( M_PI / 180.0 ) )return 0.0;
        else return std::numeric_limits<float>::max();
    }else return ( 1.0 + color_dist ) ;
}

template <typename M>
void VoxelGraph<M>::get_XYZLab(arma::fmat&voxels,const arma::uvec&indices)
{
    arma::fmat XYZ;
    arma::fmat Lab;
    if(!indices.is_empty())
    {
        XYZ = voxel_centers.cols(indices);
        ColorArray::RGB2Lab(voxel_colors.cols(indices),Lab);
    }else{
        XYZ = voxel_centers;
        ColorArray::RGB2Lab(voxel_colors,Lab);
    }
    voxels = arma::join_cols(XYZ,Lab);
}

template <typename M>
void VoxelGraph<M>::get_Lab(arma::fmat&voxels,const arma::uvec&indices)
{
    arma::fmat Lab;
    if(!indices.is_empty())
    {
        ColorArray::RGB2Lab(voxel_colors.cols(indices),Lab);
    }else{
        ColorArray::RGB2Lab(voxel_colors,Lab);
    }
    voxels = Lab;
}

