#ifndef MBB_H
#define MBB_H
//minimum bounding box
#include "common.h"
#include <armadillo>
void COMMONSHARED_EXPORT get2DMBB(arma::fmat&,arma::uword,arma::fmat&);
bool COMMONSHARED_EXPORT in2DMBB(arma::fmat&,arma::fvec&);
void COMMONSHARED_EXPORT get3DMBB(arma::fmat&,arma::uword,arma::fmat&);
void COMMONSHARED_EXPORT buildBB(DefaultMesh&);
#endif // MBB_H
