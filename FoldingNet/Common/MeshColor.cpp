#include "MeshColor.h"
#include "common_global.h"
#include <QColor>
#include <random>
const float ColorArray::Lab_L_min = 0;
const float ColorArray::Lab_L_max = 100;
const float ColorArray::Lab_ab_min = -120;
const float ColorArray::Lab_ab_max = 120;

std::default_random_engine e(time(nullptr));

void COMMONSHARED_EXPORT ColorArray::RGBArray::reset(long size, uint8_t r, uint8_t g, uint8_t b)
{
    int N;
    if(size_!=size)
    {
        if(data_)delete[] data_;
        size_ = size;
        N = 1 + ( size_/ 2 );
        data_ = (uint8_t*)(new uint64_t[ N ]); // aligned allocation
    }
    RGB64 rgb64;
    rgb64.rgba.r0 = r;
    rgb64.rgba.g0 = g;
    rgb64.rgba.b0 = b;
    rgb64.rgba.a0 = 255;
    rgb64.rgba.r1 = r;
    rgb64.rgba.g1 = g;
    rgb64.rgba.b1 = b;
    rgb64.rgba.a1 = 255;

    uint64_t* long_ptr = (uint64_t*)data_;
    for(long i = 0 ; i < N ; i ++ )
    {
        *long_ptr = rgb64.color;
        long_ptr++;
    }
}

void ColorArray::Lab2RGB(const arma::fmat& Lab, arma::Mat<uint8_t>& rgb)
{
    if(3!=Lab.n_rows)std::logic_error("Lab.n_rows!=3");

    const float T1(0.008856);
    const float T2(0.206893);
    const float m[9]={
        3.240479,-0.969256,0.055648,
        -1.537150,1.875992,-0.204043,
        -0.498535,0.041556,1.057311};

    const arma::fmat::fixed<3,3> MAT(&m[0]);
    //Y
    arma::frowvec Y;
    arma::frowvec L = Lab.row(0);
    Y = arma::pow( ( L + 16.0 ) / 116.0 , 3 );
    arma::uvec YT = arma::find( Y > T1 );
    arma::uvec _YT = arma::find( Y <= T1 );
    Y(_YT) = L(_YT);
    Y(_YT) /= 903.3;
    //alter fY
    arma::frowvec fY = Y;
    fY(YT) = arma::pow( fY(YT) , 1.0/3.0 );
    fY(_YT)*=7.787;
    fY(_YT) += 16.0/116.0;
    //X
    arma::frowvec X;
    X = Lab.row(1) / 500.0 + fY;
    arma::uvec XT = arma::find( X > T2 );
    arma::uvec _XT = arma::find( X <= T2 );
    X(XT) = arma::pow( X(XT) ,3);
    X(_XT) -= 16.0/116.0;
    X(_XT) /= 7.787;
    X*=0.950456;
    //Z
    arma::frowvec Z;
    Z = fY;
    Z -= Lab.row(2) / 200.0;
    arma::uvec ZT = arma::find( Z > T2 );
    arma::uvec _ZT = arma::find( Z <= T2 );
    Z(ZT) = arma::pow( Z(ZT) ,3);
    Z(_ZT) -= 16.0/116.0;
    Z(_ZT) /= 7.787;
    Z*= 1.088754;

    arma::fmat fxyz(3,Lab.n_cols);
    arma::fmat frgb;

    fxyz.row(0) = X;
    fxyz.row(1) = Y;
    fxyz.row(2) = Z;

    frgb = MAT*fxyz;

    arma::uvec i0 = arma::find( frgb.row(0) < 0.0 );
    arma::uvec i1 = arma::find( frgb.row(1) < 0.0 );
    arma::uvec i2 = arma::find( frgb.row(2) < 0.0 );


    frgb.cols(i0).fill(1.0);
    frgb.cols(i1).fill(1.0);
    frgb.cols(i2).fill(1.0);

    i0 = arma::find( frgb.row(0) > 1.0 );
    i1 = arma::find( frgb.row(1) > 1.0 );
    i2 = arma::find( frgb.row(2) > 1.0 );

    frgb.cols(i0).fill(1.0);
    frgb.cols(i1).fill(1.0);
    frgb.cols(i2).fill(1.0);

    frgb*=255.0;

    rgb = arma::conv_to<arma::Mat<uint8_t>>::from(frgb);
}

void ColorArray::RGB2Lab(const arma::Mat<uint8_t>& rgb, arma::fmat& Lab)
{
    if(3!=rgb.n_rows)std::logic_error("rgb.n_rows!=3");
    const float T(0.008856);
    const float m[9] = {
        0.412453,0.212671,0.019334,
        0.212671,0.715160,0.072169,
        0.019224,0.119193,0.950227
                       };
    const arma::fmat::fixed<3,3> MAT(&m[0]);
    arma::fmat XYZ = MAT*(arma::conv_to<arma::fmat>::from(rgb)/255.0);
    XYZ.row(0) /= 0.950456;
    XYZ.row(2) /= 1.088754;

    arma::uvec XT = arma::find( XYZ.row(1) > T );
    arma::uvec _XT = arma::find( XYZ.row(1) <= T );

    arma::frowvec fX = XYZ.row(0);
    fX(XT) = arma::pow(fX(XT),1.0/3.0);
    fX(_XT) *= 7.787;
    fX(_XT) += 16.0/116.0;

    arma::uvec YT = arma::find( XYZ.row(0) > T );
    arma::uvec _YT = arma::find( XYZ.row(0) <= T );

    arma::frowvec fY = XYZ.row(1);
    fY(YT) = arma::pow(fY(YT),1.0/3.0);
    fY(_YT) *= 7.787;
    fY(_YT) += 16.0/116.0;

    arma::frowvec Y = XYZ.row(1);
    Y(_YT) *= 903.3;
    Y(YT) = fY(YT);
    Y(YT) *= 116.0;
    Y(YT) -= 16.0;

    arma::uvec ZT = arma::find( XYZ.row(2) > T );
    arma::uvec _ZT = arma::find( XYZ.row(2) <= T );

    arma::frowvec fZ = XYZ.row(2);
    fZ(ZT) = arma::pow(fZ(ZT),1.0/3.0);
    fZ(_ZT) *= 7.787;
    fZ(_ZT) += 16.0/116.0;


    Lab = arma::fmat(rgb.n_rows,rgb.n_cols);
    Lab.row(0) = Y;
    Lab.row(1) = 500*( fX - fY );
    Lab.row(2) = 200*( fY - fZ );
}

void ColorArray::RGB2Lab(const arma::Col<uint8_t>& rgb, arma::fvec& Lab)
{
    if(3!=rgb.n_rows)std::logic_error("rgb.n_rows!=3");
    const float T(0.008856);
    const float m[9] = {
        0.412453,0.212671,0.019334,
        0.212671,0.715160,0.072169,
        0.019224,0.119193,0.950227
                       };
    const arma::fmat::fixed<3,3> MAT(&m[0]);
    arma::fvec XYZ = MAT*(arma::conv_to<arma::fvec>::from(rgb)/255.0);
    XYZ(0) /= 0.950456;
    XYZ(2) /= 1.088754;

    float fX;
    fX = XYZ(0);
    if( fX > T )
    {
        fX = std::pow(fX,1.0/3.0);
    }else{
        fX *= 7.787;
        fX += 16.0/116.0;
    }

    float fY;
    fY = XYZ(1);
    if( fY > T )
    {
        fY = std::pow(fY,1.0/3.0);
    }else{
        fY *= 7.787;
        fY += 16.0/116.0;
    }

    float fZ;
    fZ = XYZ(2);
    if( fZ > T )
    {
        fZ = std::pow(fZ,1.0/3.0);
    }else{
        fZ *= 7.787;
        fZ += 16.0/116.0;
    }

    float Y = XYZ(1);
    if( Y > T )
    {
        Y = std::pow(Y,1.0/3.0);
        Y *= 116.0;
        Y -= 16.0;
    }else{
        Y*=903.3;
    }

    Lab = arma::fvec(3);
    Lab(0) = Y;
    Lab(1) = 500*( fX - fY );
    Lab(2) = 200*( fY - fZ );
}

void ColorArray::Lab2BGR(const arma::fmat& Lab, arma::Mat<uint8_t>& bgr)
{
    Lab2RGB(Lab,bgr);
    bgr.swap_rows(0,2);
}

void ColorArray::BGR2Lab(const arma::Mat<uint8_t>& bgr, arma::fmat& Lab)
{
    arma::Mat<uint8_t> rgb = bgr;
    rgb.swap_rows(0,2);
    RGB2Lab(rgb,Lab);
}

void ColorArray::BGR2Lab(const arma::Col<uint8_t>& bgr, arma::fvec& Lab)
{
    arma::Col<uint8_t> rgb = bgr;
    rgb.swap_rows(0,2);
    RGB2Lab(rgb,Lab);
}

void ColorArray::colorfromlabel(uint32_t* ptr,arma::uword size,const arma::uvec& label)
{
    if( label.size() < size ){
        std::cerr<<"label.size() < size"<<std::endl;
        return;
    }
    int index;
    for(int i = 0 ; i < size ; ++i )
    {
        if(label(i)==0)index=0;
        else{
            std::srand(label(i));
            index = std::rand()%( ColorArray::DefaultColorNum_ - 1 );
            index += 1;
        }
        *ptr = ColorArray::DefaultColor[index].color;
        ++ptr;
    }
}

void ColorArray::colorfromIndex(uint32_t* ptr,arma::uword size)
{
    #pragma omp parallel for
    for(int i = 0 ; i < size ; ++i )
    {
        QColor color;
        color.setHslF(0.65*qreal(i)/qreal(size),0.6,0.73);
        ptr[i] = qRgb(color.blue(),color.green(),color.red());
    }
}

void ColorArray::colorfromValue(uint32_t* ptr,arma::uword size,const arma::vec& value)
{
    double max = value.max();
    double min = value.min();
    #pragma omp parallel for
    for(int i = 0 ; i < size ; ++i )
    {
        QColor color;
        color.setHslF(0.65*qreal(value(i)-min)/qreal(max-min),0.6,0.73);
        ptr[i] = qRgb(color.blue(),color.green(),color.red());
    }
}

void ColorArray::colorfromValue(RGB888*   ptr,arma::uword size,const arma::vec& value)
{
    double max = value.max();
    double min = value.min();
//    std::cerr<<"c2"<<std::endl;
    #pragma omp parallel for
    for(int i = 0 ; i < size ; ++i )
    {
        QColor color;
        color.setHslF(0.65*qreal(value(i)-min)/qreal(max-min),0.6,0.73);
        ptr[i].r = (uint8_t)color.red();
        ptr[i].g = (uint8_t)color.green();
        ptr[i].b = (uint8_t)color.blue();
    }
}

void ColorArray::colorfromValue(RGB888*   ptr,arma::uword size,const arma::fvec& value)
{
    float max = value.max();
    std::cerr<<"c3"<<std::endl;
    #pragma omp parallel for
    for(int i = 0 ; i < size ; ++i )
    {
        QColor color;
        color.setHslF(0.65*qreal(value(i))/qreal(max),0.6,0.73);
        ptr[i].r = (uint8_t)color.red();
        ptr[i].g = (uint8_t)color.green();
        ptr[i].b = (uint8_t)color.blue();
    }
}

uint32_t COMMONSHARED_EXPORT ColorArray::rand_color(void)
{
    std::uniform_real_distribution<qreal> uh(0.0,1.0);
    std::uniform_real_distribution<qreal> us(0.1,0.9);
    std::uniform_real_distribution<qreal> uv(0.5,1.0);
    QColor c;
    c.setHsvF( uh(e), us(e), uv(e) );
    return c.rgba();
}

ColorArray::RGB32 COMMONSHARED_EXPORT ColorArray::DefaultColor[DefaultColorNum_] = {
    {0XFF000000},
    {0XFFBF80FF},
    {0XFFFF8A80},
    {0XFF808AFF},
    {0XFFFF9F80},
    {0XFFFFAA80},
    {0XFFFFB580},
    {0XFFFFBF80},
    {0XFFFFCA80},
    {0XFFFFD480},
    {0XFFFFDF80},
    {0XFFFFEA80},
    {0XFFFFF480},
    {0XFFFFFF80},
    {0XFFF4FF80},
    {0XFFEAFF80},
    {0XFFDFFF80},
    {0XFFD5FF80},
    {0XFFCAFF80},
    {0XFFBFFF80},
    {0XFFB5FF80},
    {0XFF809FFF},
    {0XFF9FFF80},
    {0XFF95FF80},
    {0XFF8AFF80},
    {0XFF80FF80},
    {0XFF80FF95},
    {0XFF80FF9F},
    {0XFF80FFAA},
    {0XFF80FFB5},
    {0XFF80FFBF},
    {0XFF80FFCA},
    {0XFF80FFD4},
    {0XFF80FF95},
    {0XFF80FFDF},
    {0XFF80FF95},
    {0XFF80FFEA},
    {0XFF80FFF4},
    {0XFF80FF95},
    {0XFF80F4FF},
    {0XFF80EAFF},
    {0XFF80DFFF},
    {0XFF80D4FF},
    {0XFF80CAFF},
    {0XFF80BFFF},
    {0XFF80B5FF},
    {0XFF80EAFF},
    {0XFF80AAFF},
    {0XFF80EAFF},
    {0XFF8095FF},
    {0XFF80FFFF},
    {0XFF80FF8A},
    {0XFF8080FF},
    {0XFF9580FF},
    {0XFF9F80FF},
    {0XFFA280FF},
    {0XFFAC80FF},
    {0XFFFF9580},
    {0XFFAAFF80}
};
