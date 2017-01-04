#include "qglpointselection.h"
#include <QGL>
void PointSelections::debugSelections()
{
    for(iterator iter=begin();iter!=end();++iter)
    {
        PointSelectionBase::Ptr ptr = *iter;
        if(ptr&&0!=ptr.use_count())
        {
            ptr->debugSelection();
        }
    }
}

void BoxPointsSelection::debugSelection()
{
    std::cerr<<"BoxPointsSelection::debugSelection():Begin"<<std::endl;
    glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.9f, 0.9f);
    glVertex3fv(rect_w_.colptr(0));
    glVertex3fv(rect_w_.colptr(1));
    glVertex3fv(rect_w_.colptr(2));
    glVertex3fv(rect_w_.colptr(3));
    glEnd();
    std::cerr<<"BoxPointsSelection::debugSelection():End"<<std::endl;
}

void RayPointSelection::debugSelection()
{
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(0.1f, 0.9f, 0.9f);
    glVertex3fv(from_.memptr());
    glVertex3fv(toward_.memptr());
    glEnd();
}
