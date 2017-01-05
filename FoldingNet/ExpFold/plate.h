#ifndef PLATE_H
#define PLATE_H
#include <memory>
#include <armadillo>
struct Plate
{
    typedef std::shared_ptr<Plate> Ptr;
    std::vector<arma::uword> indices_;
    std::vector<arma::fvec> axis_;//x,y,z,nx,ny,nz
};
#endif // PLATE_H
