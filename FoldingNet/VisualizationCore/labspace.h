#ifndef LABSPACE_H
#define LABSPACE_H
#include <QFrame>
#include <QPaintEvent>
#include <armadillo>
#include "visualizationcore_global.h"
#include <QLabel>
#include <QPainter>
#include "common.h"
#include <QSlider>
#include <QMouseEvent>
namespace Ui {
class LabSpace;
}
class LabLabel:public QLabel
{
    Q_OBJECT
public:
    typedef std::shared_ptr<arma::gmm_diag> GMM_Ptr;
    typedef enum{
        L,ab
    }Mode;
    explicit LabLabel(Mode m,QWidget *parent = 0):
        QLabel(parent),
        m_(m)
    {
        switch(m_)
        {
        case L:
            setMinimumSize(50,256);
            setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
            L_ = QImage(128,128,QImage::Format_RGBA8888);
            std::cerr<<"L size:"<<L_.byteCount()<<std::endl;
            PL_ = std::make_shared<arma::Mat<uint8_t>>
                     ((uint8_t*)L_.bits(),4,L_.byteCount()/4,false,true);
            break;
        case ab:
            setMinimumSize(256,256);
            setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
            ab_ = QImage(256,256,QImage::Format_RGB888);
            std::cerr<<"ab size:"<<ab_.byteCount()<<std::endl;
            L50ab_ = std::make_shared<arma::Mat<uint8_t>>((uint8_t*)ab_.bits(),3,ab_.byteCount()/3,false,true);
            break;
        }
    }

    void init()
    {
        switch(m_)
        {
        case L:
            {
                arma::Col<uint32_t> color_bar(L_.height());
                uint32_t* ptr = (uint32_t*)color_bar.memptr();
                ColorArray::colorfromIndex(ptr,color_bar.size());
                arma::Mat<uint32_t> Lmat((uint32_t*)L_.bits(),L_.width(),L_.height(),false,true);
                Lmat.each_row() = color_bar.t();
            }
            break;
        case ab:
            Lab_ = arma::fmat(L50ab_->n_rows,L50ab_->n_cols);
            size_t cnt = 0;
            for(float b=127;b>=-128;b-=1.0)
            {
                for(float a=-128;a<=127;a+=1.0)
                {
                    Lab_(0,cnt) = 50.0;
                    Lab_(1,cnt) = a;
                    Lab_(2,cnt) = b;
                    ++cnt;
                }
            }
            ColorArray::Lab2RGB(Lab_,*L50ab_);
            break;
        }
        update();
    }

    void setGMM(GMM_Ptr ptr){gmm_ptr_ = ptr;}

    ~LabLabel()
    {
        std::cerr<<"-"<<std::endl;
    }
protected:
    void mousePressEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent *e);
protected slots:
    void changeL(int);
public:
    std::shared_ptr<arma::Mat<uint8_t>> L50ab_;
    std::shared_ptr<arma::Mat<uint8_t>> PL_;
private:
    arma::fmat Lab_;
    QImage ab_;
    QImage L_;
    std::shared_ptr<arma::gmm_diag> gmm_ptr_;
    Mode m_;
};

class VISUALIZATIONCORESHARED_EXPORT LabSpace : public QFrame
{
    Q_OBJECT
public:
    typedef std::shared_ptr<arma::gmm_diag> GMM_Ptr;
    explicit LabSpace(QWidget *parent = 0);
    ~LabSpace();
    void setGMM(GMM_Ptr ptr)
    {
        gmm_ptr_= ptr;
        ab_->setGMM(gmm_ptr_);
        L_->setGMM(gmm_ptr_);
    }
protected:

private:
    GMM_Ptr gmm_ptr_;
    LabLabel* ab_;
    LabLabel* L_;
    Ui::LabSpace *ui;
};

#endif // LABSPACE_H
