#ifndef DESIGNTOMESH_H
#define DESIGNTOMESH_H
#include <stack>
#include <opencv2/opencv.hpp>
#include "LineSegment.h"
#include "Parameters.h"
#include "region.h"
class DesignToMesh
{
public:
    DesignToMesh();
    ~DesignToMesh();

    void LoadParameters();
    void ReadLinesFromTxt();
    void Rasterization();
    bool FindLayoutByFloodFill();
    void generate_mesh();
    void save_mesh(const std::string&);
    void save_flooding_map(const std::string&);
protected:
    void CutOffLines();
    void BresenhamRasterization(LineSegment lineSeg, int id);  //Bresenham Raterization
    inline int getXForFlood(float x){return round((x - _MinPX)*g_parameters.ResolutionMultipe) + 1;}
    inline int getYForFlood(float y){return round((y - _MinPY)*g_parameters.ResolutionMultipe) + 1;}
    int getNearByPositiveID(int x,int y);
    int FloodFill4Region(int x,int y, int start_planenumber);//return the number of found plane
    int FloodFill4Plane(int x,int y,Plane& current_plane);//
    void FindNewStartFromDash(int x, int y);
    void remove_isolate_lines(void);//lines that only connect to one plane
private:
    Region::PtrLst region_lst_;
    Region::Ptr lr_ptr_;      //ptr to the main region
    vector<LineSegment> _LineList;
    stack<Point> _PointStack; //for plane to grow
    stack<Point> _PlaneStack; //for region to grow
    cv::Mat _LineLabel;
    cv::Mat _PolygonLabel;
    int _RangeofPX;
    int _RangeofPY;
    int _MinPX;
    int _MinPY;
    int plane_num_;
    DefaultMesh mesh_;
};

#endif // DESIGNTOMESH_H
