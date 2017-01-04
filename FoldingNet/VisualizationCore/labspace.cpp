#include "labspace.h"
#include "ui_labspace.h"
#include <QPainter>
#include "common.h"
#include <QSizePolicy>
LabSpace::LabSpace(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::LabSpace)
{
    ui->setupUi(this);
    ab_ = new LabLabel(LabLabel::ab);
    ab_->init();
    L_ = new LabLabel(LabLabel::L);
    L_->init();
    connect(ui->verticalSlider,SIGNAL(valueChanged(int)),ab_,SLOT(changeL(int)));
    ui->horizontalLayout->addWidget(L_);
    ui->horizontalLayout->addWidget(ab_);
}

LabSpace::~LabSpace()
{
    ui->horizontalLayout->removeWidget(ab_);
    ui->horizontalLayout->removeWidget(L_);
    ab_->deleteLater();
    L_->deleteLater();
    delete ui;
}

void LabLabel::changeL(int value)
{
    QSlider* s = qobject_cast<QSlider*>(sender());
    if(s)
    {
        Lab_.row(0).fill(100.0*(value - s->minimum())/(s->maximum() - s->minimum()));
        ColorArray::Lab2RGB(Lab_,*L50ab_);
        update();
    }
}

void LabLabel::paintEvent(QPaintEvent *e)
{
    QPainter p;
    p.begin(this);
    switch(m_)
    {
    case L:
        p.drawImage(0,0,L_.scaledToHeight(this->height()));
        break;
    case ab:
        p.drawImage(0,0,ab_.scaled(this->size()));
        break;
    }
    p.end();
    QLabel::paintEvent(e);
}

void LabLabel::mousePressEvent(QMouseEvent* e)
{
    ;
}
