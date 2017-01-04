#ifndef SEGVIEW_H
#define SEGVIEW_H

#include <QWidget>
#include <armadillo>
#include <QPaintEvent>
#include "visualizationcore_global.h"
namespace Ui {
class SegView;
}

class  VISUALIZATIONCORESHARED_EXPORT SegView : public QWidget
{
    Q_OBJECT

public:
    typedef QHash<arma::uword,arma::uword> ColorLabelMap;
    explicit SegView(
            QImage& img,
            arma::uvec& lbl,
            ColorLabelMap& map,
            QWidget *parent = 0
            );

    ~SegView();
public slots:
    void view_label(bool v){view_label_ = v;}
protected slots:
    void paintEvent(QPaintEvent*);
private:
    Ui::SegView *ui;
    bool view_label_;
    QImage &img_;
    arma::uvec &label_;
    ColorLabelMap & map_;
};

#endif // SEGVIEW_H
