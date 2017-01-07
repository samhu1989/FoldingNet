#include "common.h"
#include "common_global.h"
#include "MeshType.h"
#include "MeshColor.h"
#include "voxelgraph.hpp"
void ColorArray::hsv2rgb(float h,float s,float v,RGB32&rgba)
{
int hi = (int(h) / 60) % 6;
float f = h / 60.0 - hi;
uint8_t p = 255.0*v*(1-s);
uint8_t q = 255.0*v*(1.0-f*s);
uint8_t t = 255.0*v*(1.0-(1.0-f)*s);
uint8_t v_ = 255.0*v;
    switch(hi)
    {
    case 0:
        rgba.rgba.r =v_;rgba.rgba.g=t;rgba.rgba.b = p;return;
    case 1:
        rgba.rgba.r =q;rgba.rgba.g=v_;rgba.rgba.b = p;return;
    case 2:
        rgba.rgba.r=p;rgba.rgba.g=v_;rgba.rgba.b=t;return;
    case 3:
        rgba.rgba.r=p;rgba.rgba.g=q;rgba.rgba.b=v_;return;
    case 4:
        rgba.rgba.r=t;rgba.rgba.g=p;rgba.rgba.b=v_;return;
    case 5:
        rgba.rgba.r=v_;rgba.rgba.g=p;rgba.rgba.b=q;return;
    }
}

void getRotationFromZY(
        const arma::fvec& z_axis,
        const arma::fvec& y_direction,
        arma::fmat& transformation
        )
{
    arma::fvec tmp0 = arma::normalise(arma::cross(y_direction,z_axis));
    arma::fvec tmp1 = arma::normalise(arma::cross(z_axis,tmp0));
    arma::fvec tmp2 = arma::normalise(z_axis);
    transformation = arma::fmat(3,3);
    transformation(0,0)=tmp0[0]; transformation(0,1)=tmp0[1]; transformation(0,2)=tmp0[2];
    transformation(1,0)=tmp1[0]; transformation(1,1)=tmp1[1]; transformation(1,2)=tmp1[2];
    transformation(2,0)=tmp2[0]; transformation(2,1)=tmp2[1]; transformation(2,2)=tmp2[2];
}

void getRotationFromXY(
        const arma::fvec& x_axis,
        const arma::fvec& y_direction,
        arma::fmat& transformation
        )
{
    arma::fvec tmp2 = arma::normalise(arma::cross(x_axis,y_direction));
    arma::fvec tmp1 = arma::normalise(arma::cross(tmp2,x_axis));
    arma::fvec tmp0 = arma::normalise(x_axis);
    transformation = arma::fmat(3,3);
    transformation(0,0)=tmp0[0]; transformation(0,1)=tmp0[1]; transformation(0,2)=tmp0[2];
    transformation(1,0)=tmp1[0]; transformation(1,1)=tmp1[1]; transformation(1,2)=tmp1[2];
    transformation(2,0)=tmp2[0]; transformation(2,1)=tmp2[1]; transformation(2,2)=tmp2[2];
}

arma::mat getRotationMatrix2D(const arma::vec& center,double theta,double scale)
{
    double th = theta*M_PI/180.0;
    double c = std::cos(th);
    double s = std::sin(th);
    double x = center(0);
    double y = center(1);
    arma::mat tmp = {{scale*c,-s,x},{s,scale*c,y}};
    return tmp;
}

arma::uvec randperm(arma::uword N)
{
    arma::vec x = arma::randu<arma::vec>(N);
    return arma::sort_index(x);
}

void init_resouce()
{
    Q_INIT_RESOURCE(rs);
}
//axis px py pz dx dy dz (position and direction)
void getTransformFromAxis(const arma::fvec& axis,const float theta,arma::fmat& T)
{
    T = arma::fmat(4,4,arma::fill::eye);

    float a = axis(0);
    float b = axis(1);
    float c = axis(2);

    arma::fvec dir = axis.tail(3);
    dir = arma::normalise(dir);

    float u = dir(0);
    float v = dir(1);
    float w = dir(2);

    float uu = u * u;
    float uv = u * v;
    float uw = u * w;
    float vv = v * v;
    float vw = v * w;
    float ww = w * w;
    float au = a * u;
    float av = a * v;
    float aw = a * w;
    float bu = b * u;
    float bv = b * v;
    float bw = b * w;
    float cu = c * u;
    float cv = c * v;
    float cw = c * w;

    float costheta = cosf(theta);
    float sintheta = sinf(theta);

    T(0,0) = uu + (vv + ww) * costheta;
    T(0,1) = uv * (1 - costheta) + w * sintheta;
    T(0,2) = uw * (1 - costheta) - v * sintheta;
    T(0,3) = 0;

    T(1,0) = uv * (1 - costheta) - w * sintheta;
    T(1,1) = vv + (uu + ww) * costheta;
    T(1,2) = vw * (1 - costheta) + u * sintheta;
    T(1,3) = 0;

    T(2,0) = uw * (1 - costheta) + v * sintheta;
    T(2,1) = vw * (1 - costheta) - u * sintheta;
    T(2,2) = ww + (uu + vv) * costheta;
    T(2,3) = 0;

    T(3,0) = (a * (vv + ww) - u * (bv + cw)) * (1 - costheta) + (bw - cv) * sintheta;
    T(3,1) = (b * (uu + ww) - v * (au + cw)) * (1 - costheta) + (cu - aw) * sintheta;
    T(3,2) = (c * (uu + vv) - w * (au + bv)) * (1 - costheta) + (av - bu) * sintheta;
    T(3,3) = 1;
    T = T.t();
}




