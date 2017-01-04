#include "segview.h"
#include "ui_segview.h"
#include <QColor>
#include <QDebug>
#include "common.h"
SegView::SegView(QImage& img,
        arma::uvec& lbl, ColorLabelMap &map,
        QWidget *parent
        ):
    img_(img),
    label_(lbl),
    map_(map),
    view_label_(false),
    QWidget(parent),
    ui(new Ui::SegView)
{
    ui->setupUi(this);
    setMinimumSize(320,240);
    setWindowTitle(img_.text(tr("Path")));
    ui->label->setAlignment(Qt::AlignHCenter|Qt::AlignCenter);
}

void SegView::paintEvent(QPaintEvent* e)
{
    if(view_label_&&label_.size()==(img_.height()*img_.width()))
    {
        QImage img(img_.width(),img_.height(),QImage::Format_RGB888);
        for(int y=0;y<img.height();++y)
            for(int x=0;x<img.width();++x)
            {
                int h,s,l,ch_l;
                QColor cv(img_.pixel(x,y));
                if( map_.find( label_(x+y*img_.width()) )==map_.end())
                {
                    std::srand(label_(x+y*img_.width())+1);
                    int index = std::rand()%( ColorArray::DefaultColorNum_ - 1 );
                    index += 1;
                    map_.insert(label_(x+y*img_.width()),arma::uword(ColorArray::DefaultColor[index].color));
                }
                arma::uword ci = map_.value( label_(x+y*img_.width()) );
                QColor ch = QColor::fromRgb(ci);
                cv.getHsv(&h,&s,&l);
                ch.getHsv(&h,&s,&ch_l);
                QColor combine = QColor::fromHsv(h,s,(2*ch_l+l)/3);
                img.setPixel(x,y,ch.rgb());
            }
        ui->label->setPixmap(QPixmap::fromImage(img));
    }else{
        ui->label->setPixmap(QPixmap::fromImage(img_));
    }
    QWidget::paintEvent(e);
}

SegView::~SegView()
{
    delete ui;
}
