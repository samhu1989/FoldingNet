#include "planegraph.h"
#include <QRgb>
#include "nanoflann.hpp"
#include <iomanip>
PlaneGraph::PlaneGraph(
        const DefaultMesh& mesh,
        const arma::sp_imat& connection,
        float th=5.0*std::numeric_limits<float>::epsilon()
        ):same_vertex_th_(th),mesh_(mesh),connection_(connection)
{
    recover_planes(mesh,connection);
}

arma::uvec PlaneGraph::get_side_a(int axis_id,std::vector<int>& side_a_axis)
{
    std::vector<bool> visited(nodes_.size(),false);
    std::queue<int> node_to_be;
    std::vector<int> result_planes;
    //find the edge
    for(arma::sp_imat::iterator iter=edges_.begin();iter!=edges_.end();++iter)
    {
        if(axis_id==*iter)
        {
            node_to_be.push(iter.row());
            break;
        }
    }
    while(!node_to_be.empty())
    {
        int current = node_to_be.front();
        node_to_be.pop();
        if(visited[current])continue;
        result_planes.push_back(current);
        visited[current] = true;
        for(int i=0;i<nodes_.size();++i)
        {
            if(edges_(current,i)>0&&!visited[i]&&edges_(current,i)!=axis_id)
            {
                side_a_axis.push_back(edges_(current,i));
                node_to_be.push(i);
            }
        }
    }
    std::vector<arma::uword> result;
    for(std::vector<int>::iterator iter=result_planes.begin();iter!=result_planes.end();++iter)
    {
        result.insert(std::end(result),std::begin(nodes_[*iter]->indices_),std::end(nodes_[*iter]->indices_));
    }
    return arma::uvec(result);
}

arma::uvec PlaneGraph::get_side_b(int axis_id, std::vector<int> &side_b_axis)
{
    std::vector<bool> visited(nodes_.size(),false);
    std::queue<int> node_to_be;
    std::vector<int> result_planes;
    //find the edge
    for(arma::sp_imat::iterator iter=edges_.begin();iter!=edges_.end();++iter)
    {
        if(axis_id==*iter)
        {
            node_to_be.push(iter.col());
            break;
        }
    }
    while(!node_to_be.empty())
    {
        int current = node_to_be.front();
        node_to_be.pop();
        if(visited[current])continue;
        result_planes.push_back(current);
        visited[current] = true;
        for(int i=0;i<nodes_.size();++i)
        {
            if(edges_(current,i)>0&&!visited[i]&&edges_(current,i)!=axis_id)
            {
                side_b_axis.push_back(edges_(current,i));
                node_to_be.push(i);
            }
        }
    }
    std::vector<arma::uword> result;
    for(std::vector<int>::iterator iter=result_planes.begin();iter!=result_planes.end();++iter)
    {
        result.insert(std::end(result),std::begin(nodes_[*iter]->indices_),std::end(nodes_[*iter]->indices_));
    }
    return arma::uvec(result);
}

void PlaneGraph::recover_planes(const DefaultMesh& mesh, const arma::sp_imat& connection)
{
    std::cout<<"PlaneGraph::recover_planes"<<std::endl;
    start_plane_ = 0;
    __gnu_cxx::hash_map<uint32_t,uint32_t> color_to_plane;
    std::cout<<"there should be "<<connection.n_rows<<" planes"<<std::endl;
    //recover planes from color
    for(int i = 0; i < connection.n_rows ;i++)
    {
        uint32_t ckey = connection(i,connection.n_cols-1);
        if(ckey==qRgb(0,0,0)){
            start_plane_++;
            std::cout<<"there should be "<<connection.n_rows-1<<" planes"<<std::endl;
            continue;
        }
        std::cout<<"("<<qRed(ckey)<<","<<qGreen(ckey)<<","<<qBlue(ckey)<<")"<<std::endl;
        color_to_plane[ckey] = nodes_.size();
        nodes_.emplace_back(new Node());
        nodes_.back()->idx_ = nodes_.size() - 1;
    }
    for(DefaultMesh::VertexIter v_it=mesh.vertices_sbegin(); v_it!=mesh.vertices_end(); ++v_it)
    {
        DefaultMesh::Color c = mesh.color(*v_it);
        uint8_t* p = (uint8_t*)c.data();
        uint32_t ckey = qRgb(p[0],p[1],p[2]);
        if(color_to_plane.end()==color_to_plane.find(ckey))
        {
            std::cout<<"new color"<<std::endl;
            std::cout<<"("<<qRed(ckey)<<","<<qGreen(ckey)<<","<<qBlue(ckey)<<")"<<std::endl;
            color_to_plane[ckey] = nodes_.size();
            nodes_.emplace_back(new Node());
            nodes_.back()->idx_ = nodes_.size() - 1;
        }
        uint32_t plane_idx = color_to_plane[ckey];
        if(!nodes_[plane_idx])nodes_[plane_idx].reset(new Node());
        nodes_[plane_idx]->indices_.push_back(v_it->idx());
    }
    std::cerr<<nodes_.size()<<" planes found"<<std::endl;
    edges_ = arma::sp_imat(nodes_.size(),nodes_.size());
    for(Node::PtrLst::iterator iter0=nodes_.begin();iter0!=nodes_.end();++iter0)
    {
        for(Node::PtrLst::iterator iter1=iter0;iter1!=nodes_.end();++iter1)
        {
            if(iter0==iter1)continue;
            recover_axis(mesh,**iter0,**iter1,connection);
        }
    }
    std::cerr<<axis_.size()<<" axis found"<<std::endl;
}

void PlaneGraph::recover_axis(const DefaultMesh& mesh, const Node& n0, const Node& n1,const arma::sp_imat& connection)
{
    std::cout<<"PlaneGraph::recover_axis"<<std::endl;
    //if the plane is not connected don't need to recover the axis
    if(!connection(n0.idx_+start_plane_,n1.idx_+start_plane_))return;
    arma::fmat connect_points;
    get_connect_points(mesh,n0,n1,connect_points);
    if(get_axis_from_points(connect_points))
    {
        edges_(n0.idx_,n1.idx_) = axis_.size();
        edges_(n1.idx_,n0.idx_) = axis_.size();
    }else{
        std::cerr<<"Failed to recover axis for "<<n0.idx_<<"->"<<n1.idx_<<std::endl;
        edges_(n0.idx_,n1.idx_) = -1;
        edges_(n1.idx_,n0.idx_) = -1;
    }
}

void PlaneGraph::get_connect_points(
        const DefaultMesh& mesh,
        const Node& n0,
        const Node& n1,
        arma::fmat& c
        )
{
    std::cout<<"PlaneGraph::get_connect_points"<<std::endl;
    arma::fmat v((float*)mesh.points(),3,mesh.n_vertices(),false,true);
    arma::uvec idxa,idxb;
    arma::fmat va,vb;
    if(n0.indices_.size()<n1.indices_.size())
    {
        idxa = arma::conv_to<arma::uvec>::from(n0.indices_);
        idxb = arma::conv_to<arma::uvec>::from(n1.indices_);
    }else{
        idxa = arma::conv_to<arma::uvec>::from(n1.indices_);
        idxb = arma::conv_to<arma::uvec>::from(n0.indices_);
    }
    va = v.cols(idxa);
    vb = v.cols(idxb);
    ArmaKDTreeInterface<arma::fmat> arma_points(vb);
    nanoflann::KDTreeSingleIndexAdaptor<
            nanoflann::L2_Simple_Adaptor<float,ArmaKDTreeInterface<arma::fmat>>,
            ArmaKDTreeInterface<arma::fmat>,
            3,arma::uword>
            kdtree(3,arma_points,nanoflann::KDTreeSingleIndexAdaptorParams(3));
    kdtree.buildIndex();
    std::vector<float> c_pts_;//shared_points
    float* point_ptr = va.memptr();
    for(int i=0;i<va.n_cols;++i)
    {
        arma::uword index = std::numeric_limits<arma::uword>::max();
        float dist = std::numeric_limits<float>::quiet_NaN();
        kdtree.knnSearch(point_ptr,1,&index,&dist);
        if(dist<=same_vertex_th_)//consider as same vertex
        {
            arma::fvec tmp = va.col(i);
            tmp += vb.col(index);
            tmp /= 2;
            c_pts_.push_back(tmp(0));
            c_pts_.push_back(tmp(1));
            c_pts_.push_back(tmp(2));
        }
        point_ptr += 3;
    }
    //output
    c = arma::fmat(c_pts_.data(),3,c_pts_.size()/3,false,true);
}

bool PlaneGraph::get_axis_from_points(
        const arma::fmat& connect
        )
{
    std::cout<<"PlaneGraph::get_axis_from_points"<<std::endl;
    if(connect.n_cols<2)return false;
    arma::fmat dash_points = connect;
    arma::fvec pos = arma::mean(dash_points,1);
    arma::fvec dir;
    arma::fmat c_dash_points = dash_points.each_col() - pos;
    //caculate the direction by PCA
    if(dash_points.n_cols>2)
    {
        arma::mat A = arma::conv_to<arma::mat>::from(dash_points*dash_points.t());
        arma::mat eigvec;
        arma::vec eigval;
        arma::eig_sym(eigval,eigvec,A);
        dir = arma::conv_to<arma::fvec>::from(eigvec.col(2));
    }else{
        dir = c_dash_points.col(0);
        dir = arma::normalise(dir);
    }
    arma::frowvec cos = dir.t()*c_dash_points;
    arma::uword imax,imin;
    float max = cos.max(imax);
    float min = cos.min(imin);

    //recaculate the direction using two points at two ends
    dir = c_dash_points.col(imax) - c_dash_points.col(imin);
    dir = arma::normalise(dir);

    cos = dir.t()*c_dash_points;
    max = cos.max(imax);
    min = cos.min(imin);

    pos += dir*( max + min )*0.5; //correct the center
    dir*= ( max - min )*0.5; // scale the direction

    arma::fvec axis(6);
    axis.head(3) = pos;
    axis.tail(3) = dir;
    axis_.emplace_back();
    axis_.back().first = axis_.size();
    axis_.back().second = axis;
    return true;
}

void PlaneGraph::test_connect_points(int axis_id)
{
    arma::fmat connect_points;
    for(arma::sp_imat::iterator iter=edges_.begin();iter!=edges_.end();++iter)
    {
        if(axis_id==*iter)
        {
            get_connect_points(
                    mesh_,
                    *nodes_[iter.col()],
                    *nodes_[iter.row()],
                    connect_points
                    );
            break;
        }
    }
    std::cerr<<"points:"<<std::endl;
    std::cerr<<std::setprecision(8)<<std::endl;
    for(int r=0;r<connect_points.n_rows;++r)
    {
        for(int c=0;c<connect_points.n_cols;++c)
        {
            std::cerr<<connect_points(r,c)<<",";
        }
        std::cerr<<std::endl;
    }
}
