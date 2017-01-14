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
    bool configure(Config::Ptr);
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
    void FindNewStartFromDash(int plane_id,int x, int y);
    void remove_isolate_lines(void);//lines that only connect to one plane
    bool isNeighorToPlane(const Point& p,int plane_id);
    void get_dash_state(const DefaultMesh&, const std::vector<DefaultMesh::VertexHandle> &vhandles);
private:
    Region::PtrLst region_lst_;
    Region::Ptr lr_ptr_;      //ptr to the main region
    vector<LineSegment> _LineList;
    stack<Point> _PointStack; //for plane to grow
    stack<std::pair<int,Point>> _PlaneStack; //for region to grow
    cv::Mat _LineLabel;
    cv::Mat _PolygonLabel;
    int neighbor_check_size_;
    int _RangeofPX;
    int _RangeofPY;
    int _MinPX;
    int _MinPY;
    int plane_num_;
    DefaultMesh mesh_;
    std::vector<int8_t> dash_;
};

#endif // DESIGNTOMESH_H
