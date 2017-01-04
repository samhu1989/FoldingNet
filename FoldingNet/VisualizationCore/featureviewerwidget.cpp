#include "featureviewerwidget.h"
#include "ui_featureviewerwidget.h"
#include <QGLWidget>
#include <QDebug>
FeatureViewerWidget::FeatureViewerWidget(QWidget *parent) :
    QWidget(parent),
    scale_(1.0),
    ui(new Ui::FeatureViewerWidget)
{
    ui->setupUi(this);
    ui->graphicsView->setViewport(new QGLWidget());
    ui->graphicsView->setScene(&scene_);
}

void FeatureViewerWidget::refresh()
{
//    std::cerr<<"refreshing"<<std::endl;
    scene_.clear();
    feature_points_.clear();
//    std::cerr<<"feature number"<<features_.n_cols<<std::endl;
    arma::Col<uint8_t> black(3,arma::fill::zeros);
    for(size_t idx = 0; idx <  features_.n_cols ;++idx)
    {
        QPen pen;
        pen.setWidth(1);
        pen.setColor(QColor(0,0,0));
        QBrush brush;
        brush.setColor(
                    QColor(
                            feature_colors_(0,idx),
                            feature_colors_(1,idx),
                            feature_colors_(2,idx)
                        )
                    );
        brush.setStyle(Qt::SolidPattern);
        QGraphicsEllipseItem* item = scene_.addEllipse(
                    custom_scale_*scale_*features_(0,idx),
                    custom_scale_*scale_*features_(1,idx),
                    8.0,
                    8.0,
                    pen,
                    brush);
        item->setBrush(brush);
        item->setCursor(QCursor(Qt::PointingHandCursor));
        item->setToolTip(feature_strings_[idx]);
        if(!arma::any(feature_colors_.col(idx))){
            item->setZValue(item->zValue()+50.0);
//            std::cerr<<idx<<"Z was set to"<<item->zValue()<<std::endl;
        }
        feature_points_.push_back(item);
    }
}

void FeatureViewerWidget::wheelEvent(QWheelEvent* event)
{
    // Typical Calculations (Ref Qt Doc)
    const int degrees = event->delta() / 8;
    int steps = degrees / 15;

    // Declare below as class member vars and set default values as below
    // qreal h11 = 1.0
    // qreal h12 = 0
    // qreal h21 = 1.0
    // qreal h22 = 0

    double scaleFactor = 0.5; //How fast we zoom
    const qreal minFactor = 1.0;
    const qreal maxFactor = 10.0;
    if(steps>0)custom_scale_ += scaleFactor;
    if(steps<0)custom_scale_ -= scaleFactor;
    custom_scale_= custom_scale_ < maxFactor ? custom_scale_ : maxFactor;
    custom_scale_= custom_scale_ > minFactor ? custom_scale_ : minFactor;
    refresh();
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setTransform(QTransform(1, 0, 0, 1, 0, 0));
}

FeatureViewerWidget::~FeatureViewerWidget()
{
    delete ui;
}
