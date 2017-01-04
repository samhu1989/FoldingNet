#ifndef QGLPOINTSELECTION_H
#define QGLPOINTSELECTION_H
#include <armadillo>
#include <memory>
#include <vector>
class PointSelectionBase
{
public:
    typedef std::shared_ptr<PointSelectionBase> Ptr;
    typedef enum{
        Unknown,
        RayPoint,
        RayPoints,
        BoxPoints,
    }Type;
    PointSelectionBase():type_(Unknown){}
    PointSelectionBase(Type t):type_(t){}
    virtual Type type(){return type_;}
    virtual void debugSelection(){std::cerr<<"Debug Invalid Selection"<<std::endl;}
private:
    Type type_;
};

class PointSelections:public std::vector<PointSelectionBase::Ptr>
{
public:
    template<typename Mesh>
    inline void selectAll(Mesh&,arma::uvec&,double);
    template<typename Mesh>
    inline bool selectAt(size_t,Mesh&,arma::uvec&,double);
    template<typename Mesh>
    inline bool selectAt(PointSelections::iterator,Mesh&,arma::uvec&,double);
    void debugSelections();
};

class RayPointSelection:public PointSelectionBase
{
public:
    typedef std::shared_ptr<RayPointSelection> Ptr;
    RayPointSelection(arma::fvec& from,arma::fvec toward):
        PointSelectionBase(PointSelectionBase::RayPoint),from_(from),toward_(toward)
    {
    }
    template<typename Mesh>
    inline void select(Mesh&,arma::uvec&,double);
    virtual void debugSelection();
private:
    arma::fvec from_;
    arma::fvec toward_;
};

class BoxPointsSelection:public PointSelectionBase
{
public:
    typedef std::shared_ptr<BoxPointsSelection> Ptr;
    BoxPointsSelection():
        PointSelectionBase(PointSelectionBase::BoxPoints),
        rect_w_(3,4,arma::fill::zeros)
    {
        ;
    }
    template<typename Mesh>
    inline void select(Mesh&,arma::uvec&,double);
    virtual void debugSelection();
    inline void setNear(const arma::fvec& v){near_=v;}
    inline void setRect(const arma::fmat& rect){rect_w_=rect;}
private:
    arma::fvec near_;
    arma::fmat rect_w_;
};

#include "qglpointselection.hpp"
#endif // QGLPOINTSELECTION_H
