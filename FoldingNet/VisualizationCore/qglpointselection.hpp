#ifndef QGLPOINTSELECTION_HPP
#define QGLPOINTSELECTION_HPP
#include "qglpointselection.h"
#include "common.h"
template<typename Mesh>
inline void PointSelections::selectAll(Mesh&m,arma::uvec&indices,double radius)
{
    for(iterator iter=begin();iter!=end();  )
    {
        arma::uvec result;
        if(selectAt(iter,m,result,radius))
        {
            indices = arma::join_cols(indices,result);
            iter = erase(iter);
        }else{
            ++iter;
        }
    }
}

template<typename Mesh>
inline bool PointSelections::selectAt(size_t index,Mesh&m,arma::uvec&indices,double radius)
{
    PointSelectionBase::Ptr ptr = (*this)[index];
    if(ptr&&0!=ptr.use_count())
    {
        switch(ptr->type())
        {
        case PointSelectionBase::RayPoint:
            {
                RayPointSelection::Ptr p = std::dynamic_pointer_cast<RayPointSelection>(ptr);
                p->select<Mesh>(m,indices,radius);
            }
            break;
        case PointSelectionBase::BoxPoints:
            {
                BoxPointsSelection::Ptr p = std::dynamic_pointer_cast<BoxPointsSelection>(ptr);
                p->debugSelection();

                return false;
            }
            break;
        default:
            std::cerr<<"Invalid Selection Type"<<std::endl;
        }
    }
    return true;
}

template<typename Mesh>
inline bool PointSelections::selectAt(PointSelections::iterator iter,Mesh&m,arma::uvec&indices,double radius)
{
    PointSelectionBase::Ptr ptr = *iter;
    if(ptr&&0!=ptr.use_count())
    {
        switch(ptr->type())
        {
        case PointSelectionBase::RayPoint:
            {
                RayPointSelection::Ptr p = std::dynamic_pointer_cast<RayPointSelection>(ptr);
                p->select<Mesh>(m,indices,radius);
            }
            break;
        case PointSelectionBase::BoxPoints:
            {
                BoxPointsSelection::Ptr p = std::dynamic_pointer_cast<BoxPointsSelection>(ptr);
                p->debugSelection();
                return false;
            }
            break;
        default:
            std::cerr<<"Invalid Selection Type"<<std::endl;
        }
    }
    return true;
}


template<typename Mesh>
void RayPointSelection::select(Mesh& m,arma::uvec& indices,double radius )
{
    indices.reset();
    arma::fmat p((float*)m.points(),3,m.n_vertices(),true,true);
    arma::fvec dir = toward_ - from_;
    dir = arma::normalise(dir);
    p.each_col() -= from_;
    arma::frowvec distsa = arma::sum(arma::square(p));
    arma::frowvec distsb = arma::square( dir.t()*p );
    arma::frowvec dists = arma::sqrt( distsa - distsb );
    p*=-1.0;
    p = arma::normalise(p);
    arma::frowvec vars = dir.t()*p;
    arma::uvec within = arma::find( dists < 0.05*radius );
    if(within.is_empty())return;
    arma::uword select;
    vars(within).min(select);
    indices = arma::uvec(1);
    indices(0) = within(select);
}

#endif // QGLPOINTSELECTION_HPP

