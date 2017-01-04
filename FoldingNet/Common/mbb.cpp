#include "mbb.h"

void get2DMBB(arma::fmat& input,arma::uword axis,arma::fmat& box)
{
    if(3!=input.n_rows)std::logic_error("3!=input.n_rows");
    box = arma::fmat(2,4);
    arma::fmat pts2d(2,input.n_cols);
    size_t ii=0,ip;
    for(ip=0;ip<2;++ip)
    {
        if(ii==axis)continue;
        pts2d.row(ip) = input.row(ii);
        ++ii;
    }
    arma::fvec mean;
    mean = arma::mean(pts2d,1);
    pts2d.each_col() -= mean;
    float minArea = std::numeric_limits<float>::max();
    float currentArea = 0.0;
    for(float theta=0; theta<2*M_PI ;theta+=(M_PI/180))
    {
        float c = std::cos(theta);
        float s = std::sin(theta);
        arma::fmat rot = {{c,-s},{s,c}};
        arma::fmat invRot = {{c,s},{-s,c}};
        arma::fmat rotP = rot*pts2d;

        arma::fvec min = arma::min(rotP,1);
        arma::fvec max = arma::max(rotP,1);
        arma::fvec dif = max - min;

        currentArea = dif(0)*dif(1);
        if(minArea>currentArea)
        {
            box(0,0) = min(0);box(0,1)=max(0);box(0,2)=max(0);box(0,3)=min(0);
            box(1,0) = min(1);box(1,1)=min(1);box(1,2)=max(1);box(1,3)=max(1);
            box = invRot*box;
            box.each_col() += mean;
            minArea = currentArea;
        }
    }
}
//axis means the dimenssion according to the axis

bool in2DMBB(arma::fmat& box ,arma::fvec& p)
{
//    std::cerr<<"in2DMBB(arma::fmat& box ,arma::fvec& p)"<<std::endl;
    arma::fvec P2P(3,arma::fill::zeros);
    P2P.rows(0,1) = p - box.col(1);
    arma::fvec P1P2(3,arma::fill::zeros);
    P1P2.rows(0,1) = box.col(1) - box.col(0);
    arma::fvec P3P(3,arma::fill::zeros);
    P3P.rows(0,1) = p - box.col(2);
    arma::fvec P3P4(3,arma::fill::zeros);
    P3P4.rows(0,1) = box.col(3) - box.col(2);
    arma::fvec P1P(3,arma::fill::zeros);
    P1P.rows(0,1) = p - box.col(0);
    arma::fvec P1P4(3,arma::fill::zeros);
    P1P4.rows(0,1) = box.col(3) - box.col(0);
    arma::fvec P2P3(3,arma::fill::zeros);
    P2P3.rows(0,1) = box.col(2) - box.col(1);
//    std::cerr<<"P2P"<<std::endl;
//    std::cerr<<P2P<<std::endl;
//    std::cerr<<P1P2<<std::endl;
//    std::cerr<<P3P<<std::endl;
//    std::cerr<<P3P4<<std::endl;
//    std::cerr<<"cross:"<<std::endl;
//    std::cerr<<arma::cross(P2P,P1P2)<<std::endl;
//    std::cerr<<arma::cross(P3P,P3P4)<<std::endl;
//    std::cerr<<arma::cross(P1P,P1P4)<<std::endl;
//    std::cerr<<arma::cross(P2P,P2P3)<<std::endl;
    return      (arma::dot(arma::cross(P2P,P1P2),arma::cross(P3P,P3P4))>=0)
           &&   (arma::dot(arma::cross(P1P,P1P4),arma::cross(P2P,P2P3))<=0);
}

void get3DMBB(arma::fmat& input,arma::uword axis,arma::fmat& box)
{
    if(3!=input.n_rows)std::logic_error("3!=input.n_rows");
    float max = arma::max(input.row(axis));
    float min = arma::min(input.row(axis));
    arma::fmat box2d;
    get2DMBB(input,axis,box2d);
    arma::fmat h(1,8);
    h.head_cols(4).fill(min);
    h.tail_cols(4).fill(max);
    box = arma::join_cols(arma::join_rows( box2d , box2d ),h);
}

void buildBB(DefaultMesh& mesh)
{
    DefaultMesh::VertexHandle vhandle[8];
    vhandle[0] = mesh.add_vertex(DefaultMesh::Point(-1, -1,  1));
    vhandle[1] = mesh.add_vertex(DefaultMesh::Point( 1, -1,  1));
    vhandle[2] = mesh.add_vertex(DefaultMesh::Point( 1,  1,  1));
    vhandle[3] = mesh.add_vertex(DefaultMesh::Point(-1,  1,  1));
    vhandle[4] = mesh.add_vertex(DefaultMesh::Point(-1, -1, -1));
    vhandle[5] = mesh.add_vertex(DefaultMesh::Point( 1, -1, -1));
    vhandle[6] = mesh.add_vertex(DefaultMesh::Point( 1,  1, -1));
    vhandle[7] = mesh.add_vertex(DefaultMesh::Point(-1,  1, -1));
    // generate (quadrilateral) faces
    std::vector<DefaultMesh::VertexHandle>  face_vhandles;
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[3]);
    mesh.add_face(face_vhandles);

    face_vhandles.clear();
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[6]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[4]);
    mesh.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[4]);
    face_vhandles.push_back(vhandle[5]);
    mesh.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[1]);
    face_vhandles.push_back(vhandle[5]);
    face_vhandles.push_back(vhandle[6]);
    mesh.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[2]);
    face_vhandles.push_back(vhandle[6]);
    face_vhandles.push_back(vhandle[7]);
    mesh.add_face(face_vhandles);
    face_vhandles.clear();
    face_vhandles.push_back(vhandle[0]);
    face_vhandles.push_back(vhandle[3]);
    face_vhandles.push_back(vhandle[7]);
    face_vhandles.push_back(vhandle[4]);
    mesh.add_face(face_vhandles);
}

