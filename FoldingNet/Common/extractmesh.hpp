#ifndef EXTRACTMESH
#define EXTRACTMESH
#include <armadillo>
template <typename IM,typename OM>
inline void extractMesh(const IM& input,const arma::uvec& indices,OM& output)
{
    typedef typename OM::Point P;
    arma::fmat pts((float*)input.points(),3,input.n_vertices(),false,true);
    for( int i=0 ; i < indices.size() ; ++i )
    {
        arma::uword index = indices(i);
        output.add_vertex(P(pts(0,index),pts(1,index),pts(2,index)));
    }
    if(input.has_vertex_colors())
    {
        output.request_vertex_colors();
        arma::Mat<uint8_t> cIn((uint8_t*)input.vertex_colors(),3,input.n_vertices(),false,true);
        arma::Mat<uint8_t> cOut((uint8_t*)output.vertex_colors(),3,output.n_vertices(),false,true);
        cOut = cIn.cols(indices);
    }
    if(input.has_vertex_normals())
    {
        output.request_vertex_normals();
        arma::fmat nIn((float*)input.vertex_normals(),3,input.n_vertices(),false,true);
        arma::fmat nOut((float*)output.vertex_normals(),3,output.n_vertices(),false,true);
        nOut = nIn.cols(indices);
    }
}
template <typename OM>
inline void extractMesh(
        const arma::fmat& vv,
        const arma::fmat& vn,
        const arma::Mat<uint8_t>& vc,
        const arma::uvec& indices,
        OM& output
        )
{
    typedef typename OM::Point P;
    for( uint32_t i=0 ; i < indices.size() ; ++i )
    {
        arma::uword index = indices(i);
        output.add_vertex(P(vv(0,index),vv(1,index),vv(2,index)));
    }
    if(!vc.empty())
    {
        output.request_vertex_colors();
        arma::Mat<uint8_t> cOut((uint8_t*)output.vertex_colors(),3,output.n_vertices(),false,true);
        cOut = vc.cols(indices);
    }
    if(!vn.empty())
    {
        output.request_vertex_normals();
        arma::fmat nOut((float*)output.vertex_normals(),3,output.n_vertices(),false,true);
        nOut = vn.cols(indices);
    }
}
#endif // EXTRACTMESH

