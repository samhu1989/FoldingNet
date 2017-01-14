#include "designtomesh.h"
#include "Parameters.h"
#include <QFileInfo>
#include <QImage>
#include "delaunay.h"
#include <QString>
#include <nanoflann.hpp>
DesignToMesh::DesignToMesh()
{

}

bool DesignToMesh::configure(Config::Ptr config)
{
    if(config->has("MyPi"))
    {
        g_parameters.MyPi = config->getFloat("MyPi");
    }else return false;
    if(config->has("InputFilePath"))
    {
        g_parameters.InputFilePath = config->getString("InputFilePath");
    }else return false;
    if(config->has("MyEpsilon"))
    {
        g_parameters.MyEpsilon = config->getFloat("MyEpsilon");
    }else return false;
    if(config->has("MyInfinity"))
    {
        g_parameters.MyInfinity = config->getFloat("MyInfinity");
    }else return false;
    if(config->has("ResolutionMultipe"))
    {
        g_parameters.ResolutionMultipe = config->getInt("ResolutionMultipe");
    }else return false;
    if(config->has("ThresholdForSeparateLines"))
    {
        g_parameters.ThresholdForSeparateLines = config->getFloat("ThresholdForSeparateLines");
    }else return false;
    if(config->has("Neighbor_Check_Size"))
    {
        neighbor_check_size_ = config->getInt("Neighbor_Check_Size");
        std::cerr<<"setting neighbor_check_size_ to "<<neighbor_check_size_ <<std::endl;
    }else neighbor_check_size_ = 5;
    if(!config->has("same_vertex_threshod"))return false;
    return true;
}

void DesignToMesh::ReadLinesFromTxt()
{
    cerr << "Reading Line Segments From TXT..." << endl;
    LineSegment temp_l;
    Vertex temp_v;

    float t_value1=0;
    float t_value2=0;
    float t_value3=0;
    float t_value4=0;
    int id = 1;

    string oneLine;
    ifstream txt_in(g_parameters.InputFilePath);
    string typeofLine;

    //estimate the range of x and y
    //min and max of x and y
    int t_max_x = 0;
    int t_max_y = 0;
    int t_min_x = std::numeric_limits<int>::max();
    int t_min_y = std::numeric_limits<int>::max();

    int isdash;
    //line by line input
    while (txt_in)
    {
        txt_in >> typeofLine;
        if (typeofLine == "ISDASH")
            txt_in >> isdash;
        if (typeofLine == "LINESEGMENT" || typeofLine == "BEZIER")
        {
            txt_in >> t_value1 >> t_value2 >> t_value3 >> t_value4 ;

            //防止线段两个端点一样
            if (fabs(t_value1 - t_value3) < g_parameters.MyEpsilon && fabs(t_value2 - t_value4) < g_parameters.MyEpsilon)
                continue;

            //查找二维点坐标中XY的最大最小值
            if (t_value1 > t_value3)
            {
                if (t_max_x < t_value1)
                    t_max_x = t_value1;
                if (t_min_x > t_value3)
                    t_min_x = t_value3;
            }
            else
            {
                if (t_max_x < t_value3)
                    t_max_x = t_value3;
                if (t_min_x > t_value1)
                    t_min_x = t_value1;
            }

            if (t_value2 > t_value4)
            {
                if (t_max_y < t_value2)
                    t_max_y = t_value2;
                if (t_min_y > t_value4)
                    t_min_y = t_value4;
            }
            else
            {
                if (t_max_y < t_value4)
                    t_max_y = t_value4;
                if (t_min_y > t_value2)
                    t_min_y = t_value2;
            }

            temp_l.SetP1(t_value1, t_value2);
            temp_l.SetP2(t_value3, t_value4);
            temp_l.SetIsDash(isdash);
            _LineList.push_back(temp_l);
            id++;
        }

    }
    txt_in.close();

    vector<LineSegment>::iterator it, it1;

    //prevent the line is duplicated
    for (it = _LineList.begin(); it != _LineList.end(); it++)
    {
        for (it1 = it + 1; it1 != _LineList.end(); )
        {
            if (*it1 == *it)
            {
                if ((*it1).GetIsDash() != (*it).GetIsDash())
                    (*it).SetIsDash(0);
                it1 = _LineList.erase(it1);
            }
            else
                ++it1;
        }
    }

    _MinPX = int(t_min_x);
    _MinPY = int(t_min_y);
    _RangeofPX = (int(t_max_x - t_min_x) + 5)*g_parameters.ResolutionMultipe;
    _RangeofPY = (int(t_max_y - t_min_y) + 5)*g_parameters.ResolutionMultipe;


    CutOffLines();

}

void DesignToMesh::CutOffLines()
{
    Point p;
    int flag = 1;
    //若线段长度小于2，则不进行细分
    int Min_Length = 0.5;

    while (flag == 1)
    {
        flag = 0;
        for (int i = 0; i < _LineList.size() - 1; i++)
        {
            if (_LineList[i].P2DLength() > Min_Length)
            {
                for (int j = i + 1; j < _LineList.size(); j++)
                {
                    if (_LineList[j].P2DLength() > Min_Length)
                    {
                        bool judge1 = (_LineList[i].GetP1() == _LineList[j].GetP2()) || (_LineList[i].GetP2() == _LineList[j].GetP1()) || (_LineList[i].GetP1() == _LineList[j].GetP1()) || (_LineList[i].GetP2() == _LineList[j].GetP2());   //两个顶点重叠
                        bool judge2 = ((_LineList[i].GetP1() == _LineList[j].GetP1()) && (_LineList[i].GetP2() == _LineList[j].GetP2())) || ((_LineList[i].GetP1() == _LineList[j].GetP2()) && (_LineList[i].GetP2() == _LineList[j].GetP1()));   //同一条线段
                        if (_LineList[i].IntersectionByTwoLines(_LineList[j], p) > 0 && !judge1)
                        {
                            flag = 1;
                            switch (_LineList[i].IntersectionByTwoLines(_LineList[j], p))
                            {
                            case 1:
                            {
                                _LineList.push_back(_LineList[i]);
                                _LineList[i].SetP2(p);
                                _LineList[_LineList.size() - 1].SetP1(p);
                                _LineList[_LineList.size() - 1].SetId(_LineList.size());
                                _LineList.push_back(_LineList[j]);
                                _LineList[j].SetP2(p);
                                _LineList[_LineList.size() - 1].SetP1(p);
                                _LineList[_LineList.size() - 1].SetId(_LineList.size());
                                break;
                            }
                            case 2:
                            {
                                if (!judge1 && !judge2)
                                {
                                    _LineList.push_back(_LineList[i]);
                                    _LineList[i].SetP2(p);
                                    _LineList[_LineList.size() - 1].SetP1(p);
                                    _LineList[_LineList.size() - 1].SetId(_LineList.size());
                                }
                                break;
                            }
                            case 3:
                            {
                                if (!judge1 && !judge2)
                                {
                                    _LineList.push_back(_LineList[i]);
                                    _LineList[i].SetP2(p);
                                    _LineList[_LineList.size() - 1].SetP1(p);
                                    _LineList[_LineList.size() - 1].SetId(_LineList.size());
                                }
                                break;
                            }
                            case 4:
                            {
                                if (!judge1 && !judge2)
                                {
                                    _LineList.push_back(_LineList[j]);
                                    _LineList[j].SetP2(p);
                                    _LineList[_LineList.size() - 1].SetP1(p);
                                    _LineList[_LineList.size() - 1].SetId(_LineList.size());
                                }
                                break;
                            }
                            case 5:
                            {
                                if (!judge1 && !judge2)
                                {
                                    _LineList.push_back(_LineList[j]);
                                    _LineList[j].SetP2(p);
                                    _LineList[_LineList.size() - 1].SetP1(p);
                                    _LineList[_LineList.size() - 1].SetId(_LineList.size());
                                }
                                break;
                            }
                            default:
                                break;
                            }
                        }
                    }

                }
            }

        }

    }

    //防止线段重复读取
    vector<LineSegment>::iterator it, it1;
    for (it = _LineList.begin(); it != _LineList.end(); it++)
        for (it1 = it + 1; it1 != _LineList.end() ; )
        {
            if (*it1 == *it)
            {
                //cout << (*it1).GetP1().GetX() << "    " << (*it1).GetP1().GetY() << endl;
                if ((*it1).GetIsDash() != (*it).GetIsDash())
                    (*it).SetIsDash(0);
                it1 = _LineList.erase(it1);
            }
            else
                ++it1;
        }
    //break the len if it is too long
    arma::vec len(_LineList.size());
    for (int i = 0; i < _LineList.size(); i++)
    {
        len(i) = _LineList[i].P2DLength();
    }
    float stddev = arma::stddev(len);
    float mean = arma::mean(len);
    int N = _LineList.size();
    float th = mean;
    Point tp1,tp2;
    for( int i=0;i<N;i++)
    {
        if(  th  < _LineList[i].P2DLength() )
        {
            int cut_to_n = std::floor(_LineList[i].P2DLength() / th);
            float x0 = _LineList[i].GetP1().GetX();
            float y0 = _LineList[i].GetP1().GetY();
            float dx = _LineList[i].GetP2().GetX() - _LineList[i].GetP1().GetX();
            float dy = _LineList[i].GetP2().GetY() - _LineList[i].GetP1().GetY();
            for(int k=1;k<cut_to_n;++k)
            {
                tp1.SetXY(x0+dx*k/cut_to_n,y0+dy*k/cut_to_n);
                tp2.SetXY(x0+dx*(k+1)/cut_to_n,y0+dy*(k+1)/cut_to_n);
                if(k==1)_LineList[i].SetP2(tp1);
                _LineList.push_back(_LineList[i]);
                _LineList.back().SetP1(tp1);
                _LineList.back().SetP2(tp2);
            }
        }
    }

    //将二维点的x,y赋值到三维坐标中去
    for (int i = 0; i < _LineList.size(); i++)
    {
        float x1 = _LineList[i].GetP1().GetX();
        float y1 = _LineList[i].GetP1().GetY();
        float x2 = _LineList[i].GetP2().GetX();
        float y2 = _LineList[i].GetP2().GetY();

        _LineList[i].SetV1(x1, y1, 0);
        _LineList[i].SetV2(x2, y2, 0);
        _LineList[i].SetPinV1(x1, y1);
        _LineList[i].SetPinV2(x2, y2);
    }

    //set id
    for (int i = 0; i < _LineList.size(); i++)
        _LineList[i].SetId(i + 1);

}

void DesignToMesh::Rasterization()
{
    _LineLabel = cv::Mat::zeros(_RangeofPX, _RangeofPY, CV_32FC1);
    _PolygonLabel = cv::Mat::zeros(_RangeofPX, _RangeofPY, CV_32FC1);

    std::cerr << "Bresenham Rasterization..." << endl;
    for (int i = 0; i < _LineList.size(); i++)
        BresenhamRasterization(_LineList[i], _LineList[i].GetId());
}

void DesignToMesh::BresenhamRasterization(LineSegment lineSeg, int id)  //Bresenham Raterization
{
    int y1 = getYForFlood(lineSeg.GetP1().GetY());
    int x1 = getXForFlood(lineSeg.GetP1().GetX());
    int y2 = getYForFlood(lineSeg.GetP2().GetY());
    int x2 = getXForFlood(lineSeg.GetP2().GetX());

    const bool steep = (abs(y2 - y1) > abs(x2 - x1));
    if (steep) //保证X增长的更快，即斜率不超过1
    {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    if (x1 > x2)   //保证起点的X坐标小于终点
    {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    const float dx = x2 - x1;
    const float dy = abs(y2 - y1);
    float error = dx / 2.0f;
    const int ystep = (y1 < y2) ? 1 : -1;
    int y = (int)y1;
    const int maxX = (int)x2;

    for (int x = (int)x1; x <= maxX; x++)
    {
        if (steep)
        {
            //not allow the dash overwrite solid
            if(lineSeg.GetIsDash())
            {
                if(_PolygonLabel.at<float>(y,x)!=-2)
                {
                    _PolygonLabel.at<float>(y,x) = -1;
                    _LineLabel.at<float>(y, x) = id;
                }
            }
            else {
                _PolygonLabel.at<float>(y,x) = -2;
                _LineLabel.at<float>(y, x) = id;
            }
        }
        else
        {
            //not allow the dash overwrite solid
            if(lineSeg.GetIsDash())
            {
                if(_PolygonLabel.at<float>(x,y)!=-2)
                {
                    _PolygonLabel.at<float>(x,y) = -1;
                    _LineLabel.at<float>(x, y) = id;
                }
            }
            else {
                _PolygonLabel.at<float>(x,y) = -2;
                _LineLabel.at<float>(x, y) = id;
            }
        }

        error -= dy;
        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

bool DesignToMesh::FindLayoutByFloodFill()
{
    std::cerr<<"FindLayoutByFloodFill"<<std::endl;
    int planenumber = 1;
    for (int i = 0; i < _RangeofPX-1; i++)
        for (int j = 0; j < _RangeofPY-1; j++)
        {
            if (_PolygonLabel.at<float>(i, j) == 0)
            {
                planenumber += FloodFill4Region(i, j, planenumber);
            }
        }
    std::cerr<<"found plane:"<<planenumber-1<<std::endl;
    plane_num_ = planenumber-1;
    //find the region with most planes
    lr_ptr_ = region_lst_.front();
    for(Region::PtrLst::iterator iter=region_lst_.begin();iter!=region_lst_.end();++iter)
    {
        if((*iter)->size()>lr_ptr_->size())
        {
            lr_ptr_ = *iter;
        }
    }
    //for the region with most planes
    //remove duplicate line records
    Region& cr = *lr_ptr_;
    for(Plane::PtrLst::iterator piter=cr.plane_lst_.begin();piter!=cr.plane_lst_.end();++piter)
    {
        Plane& p = **piter;
        std::sort(p.line_id_lst_.begin(),p.line_id_lst_.end());
        p.line_id_lst_.erase(std::unique(p.line_id_lst_.begin(),p.line_id_lst_.end()),p.line_id_lst_.end());
    }
    //for all lines and planes
    //and count lines connect to how much planes
    for(Region::PtrLst::iterator riter=region_lst_.begin();riter!=region_lst_.end();++riter)
    {
        Region& r = **riter;
        for(Plane::PtrLst::iterator piter=r.plane_lst_.begin();piter!=r.plane_lst_.end();++piter)
        {
            Plane& p = **piter;
            for(std::vector<int>::iterator liter=p.line_id_lst_.begin();liter!=p.line_id_lst_.end();++liter)
            {
                _LineList[*liter].connected_planes_.push_back(p.id_);
                if(*riter==lr_ptr_)//if this is the main region
                {
                        _LineList[*liter].connected_to_main_.push_back(p.id_);
                }
            }
        }
    }
    return true;
}

int DesignToMesh::FloodFill4Region(int x, int y, int start_planenumber)
{
    while(!_PlaneStack.empty())_PlaneStack.pop();
    region_lst_.emplace_back(new Region);
    Region& current_region = *(region_lst_.back());
    current_region.start_plane_ = start_planenumber;
    current_region.end_plane_ = start_planenumber;
    _PlaneStack.push(std::make_pair(start_planenumber,Point(x, y)));
    std::vector<std::pair<int,int>> connected_plane;
    while(!_PlaneStack.empty())
    {
        x = int(_PlaneStack.top().second.GetX());
        y = int(_PlaneStack.top().second.GetY());
        int plane_id = _PlaneStack.top().first;
        _PlaneStack.pop();
        if(0.0==_PolygonLabel.at<float>(x, y))
        {
            current_region.plane_lst_.emplace_back(new Plane);
            Plane& plane = *current_region.plane_lst_.back();
            plane.id_ = current_region.end_plane_;
            if(plane.id_!=plane_id)connected_plane.emplace_back(plane_id,plane.id_);//means that these two are connected
            current_region.end_plane_ ++;
            FloodFill4Plane(x,y,plane);
        }
        else{
            FindNewStartFromDash(plane_id,x,y);
        }
    }
    std::cerr<<"found "<<connected_plane.size()<<" connection for current region"<<std::endl;
    int N = current_region.plane_lst_.size();
    current_region.plane_connetion_ = arma::sp_imat(N,N+1);
    std::vector<std::pair<int,int>>::iterator iter;
    for(iter=connected_plane.begin();iter!=connected_plane.end();++iter)
    {
        current_region.plane_connetion_(iter->first - current_region.start_plane_,iter->second - current_region.start_plane_) = 1;
        current_region.plane_connetion_(iter->second - current_region.start_plane_,iter->first - current_region.start_plane_) = 1;
    }
    return N;
}

void DesignToMesh::FindNewStartFromDash(int plane_id, int x, int y)
{
    if(0.0==_PolygonLabel.at<float>(x, y))return;
    if(_PolygonLabel.at<float>(x, y)!=-1)return;
    int dx[4] = { 0, 1, 0, -1 }; // relative neighbor x coordinates
    int dy[4] = { -1, 0, 1, 0 }; // relative neighbor y coordinates
    for (int i = 0; i < 4; i++)
    {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < _RangeofPX && ny >= 0 && ny < _RangeofPY && _PolygonLabel.at<float>(nx, ny) == 0)
        {
            _PlaneStack.push(std::make_pair(plane_id,Point(nx, ny)));
        }
    }
}

int DesignToMesh::FloodFill4Plane(int x, int y, Plane& current_plane)
{
    std::cerr<<"Foolding for plane:"<<current_plane.id_<<std::endl;
    if (current_plane.id_ == 0) //avoid infinite loop
        return 0;
    while(!_PointStack.empty())_PointStack.pop();
    int dx[4] = { 0, 1, 0, -1 }; // relative neighbor x coordinates
    int dy[4] = { -1, 0, 1, 0 }; // relative neighbor y coordinates
    _PointStack.push(Point(x, y));
    while (!_PointStack.empty())
    {
        x = int(_PointStack.top().GetX());
        y = int(_PointStack.top().GetY());
        _PointStack.pop();
        if(0==_PolygonLabel.at<float>(x, y))_PolygonLabel.at<float>(x, y) = float(current_plane.id_);
        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (nx >= 0 && nx < _RangeofPX && ny >= 0 && ny < _RangeofPY && _PolygonLabel.at<float>(nx, ny) == 0)
            {
                _PointStack.push(Point(nx, ny));
            }
            else if (nx >= 0 && nx < _RangeofPX && ny >= 0 && ny < _RangeofPY &&_LineLabel.at<float>(nx, ny) != 0)
            {
                LineSegment& current_line = _LineList[_LineLabel.at<float>(nx, ny)-1];
                //add current line to plane
                current_plane.line_id_lst_.push_back(current_line.GetId()-1);
                //add new start for region
                if(current_line.GetIsDash())
                {
                    _PlaneStack.emplace(current_plane.id_,Point(nx, ny));
                }
            }
        }
    }
}

struct color_equal_to : public binary_function<uint32_t,uint32_t,bool>
{
        bool operator()(const uint32_t& __x, const uint32_t& __y) const{
            float dr = ( qRed(__x) - qRed(__y) );
            float dg = ( qGreen(__x) - qGreen(__y) );
            float db = ( qBlue(__x) - qBlue(__y) );
            return std::sqrt( dr*dr + dg*dg + db*db ) < 30;
        }
};

int DesignToMesh::getNearByPositiveID(int x,int y)
{
    if(x > _RangeofPX) return -3;
    if(x<0)return -3;
    if(y > _RangeofPY) return -3;
    if(y < 0)return -3;
    int id = _PolygonLabel.at<float>(x,y);
    if(id>0)return id;
    id = getNearByPositiveID(x-1,y);
    if(id>0)return id;
    id = getNearByPositiveID(x,y-1);
    if(id>0)return id;
    id = getNearByPositiveID(x+1,y);
    if(id>0)return id;
    id = getNearByPositiveID(x,y+1);
    if(id>0)return id;
}

bool DesignToMesh::isNeighorToPlane(const Point& p,int plane_id)
{
    for(int dx=-neighbor_check_size_;dx<=neighbor_check_size_;++dx)
    for(int dy=-neighbor_check_size_;dy<=neighbor_check_size_;++dy)
    {
        int x = getXForFlood(p.GetX()) + dx;
        int y = getYForFlood(p.GetY()) + dy;
        if(x<0||x>_RangeofPX||y<0||y>_RangeofPY)continue;
        if( plane_id == _PolygonLabel.at<float>(x,y) ) return true;
    }
    return false;
}

void DesignToMesh::generate_mesh()
{
    std::cerr<<"generating mesh"<<std::endl;
    std::cerr<<"requesting fields"<<std::endl;
    mesh_.request_vertex_colors();
    mesh_.request_vertex_normals();
    mesh_.request_face_colors();
    mesh_.request_face_normals();

    __gnu_cxx::hash_map<uint32_t,uint32_t,std::hash<uint32_t>,color_equal_to> color_to_key_map;
    __gnu_cxx::hash_map<uint32_t,uint32_t> key_to_color_map;

    uint32_t backgroundcolor = qRgb(179,179,179);
    uint32_t axiscolor = qRgb(255,0,0);
    key_to_color_map[std::numeric_limits<uint32_t>::max()] = backgroundcolor;
    color_to_key_map[backgroundcolor] = std::numeric_limits<uint32_t>::max();
    key_to_color_map[std::numeric_limits<uint32_t>::max()-1] = axiscolor;
    color_to_key_map[axiscolor] = std::numeric_limits<uint32_t>::max()-1;
    //generate vertices according to number of planes the line connected to
    std::cerr<<"generating vertices"<<std::endl;
    std::vector<DefaultMesh::VertexHandle> vhandle;
    for(int i=0;i<_LineList.size();++i)
    {
        LineSegment& line = _LineList[i];
        if(line.connected_planes_.size() < 2)continue;
        for(int j=0 ; j < line.connected_to_main_.size(); ++j)
        {
            vhandle.push_back(mesh_.add_vertex(
                                  DefaultMesh::Point(
                                      line.GetP1().GetX(),
                                      line.GetP1().GetY(),
                                      0.0
                                      )
                                  )
                              );
            line.idx_p1_.push_back(vhandle.size()-1);
            vhandle.push_back(mesh_.add_vertex(
                                  DefaultMesh::Point(
                                      line.GetP2().GetX(),
                                      line.GetP2().GetY(),
                                      0.0
                                      )
                                  )
                              );
            line.idx_p2_.push_back(vhandle.size()-1);
        }
    }
//    propogate_dash();
    std::cerr<<vhandle.size()<<" vertices generated"<<std::endl;
    //triangluate each plane
    std::cerr<<"generating triangulation"<<std::endl;
    Region& cr = *lr_ptr_;
    for(Plane::PtrLst::iterator piter=cr.plane_lst_.begin();piter!=cr.plane_lst_.end();++piter)
    {
        Plane& p = **piter;
        if(1==p.id_){
            int i = p.id_ - cr.start_plane_;
            cr.plane_connetion_(i,cr.plane_connetion_.n_cols-1) = qRgb(0,0,0);
            continue;
        }
        std::cout<<"plane id:"<<p.id_<<std::endl;
        uint32_t c;
        //generate unique color for this plane
        if( key_to_color_map.end() == key_to_color_map.find(p.id_))
        {
            c = ColorArray::rand_color();
            while( color_to_key_map.end() != color_to_key_map.find(c) )//if color is duplicated rand another one
            {
                c = ColorArray::rand_color();
            }
            key_to_color_map[p.id_] = c;
            color_to_key_map[c] = p.id_;
        }else{
            c = key_to_color_map[p.id_];
        }
        //restore the unique color to plane connection
        cr.plane_connetion_(p.id_ - cr.start_plane_,cr.plane_connetion_.n_cols-1) = c;
        MESH mesh;
        std::vector<VERTEX2D> v;
        std::vector<int> v2idx;//the idx of vertex
        //collecting vertices
        VERTEX2D tmp;
        std::cout<<"collecting vertices"<<std::endl;
        for(std::vector<int>::reverse_iterator iter=p.line_id_lst_.rbegin();iter!=p.line_id_lst_.rend();++iter)
        {
            LineSegment& line = _LineList[*iter];
            if(line.connected_planes_.size()<2)continue;
            if(!line.idx_p1_.empty()&&isNeighorToPlane(line.GetP1(),p.id_))
            {
                tmp.x = line.GetP1().GetX();
                tmp.y = line.GetP1().GetY();
                v.push_back(tmp);
                v2idx.push_back(line.idx_p1_.back());
                line.idx_p1_.pop_back();//pop after use
            }
            if(!line.idx_p2_.empty()&&isNeighorToPlane(line.GetP2(),p.id_))
            {
                tmp.x = line.GetP2().GetX();
                tmp.y = line.GetP2().GetY();
                v.push_back(tmp);
                v2idx.push_back(line.idx_p2_.back());
                line.idx_p2_.pop_back();//pop after use
            }
        }
        std::cout<<"triangulization with "<<v.size()<<" vertices"<<std::endl;
        CreateMesh(v,&mesh);
        //restore the 2d mesh into 3d mesh
        std::cout<<"converting to 3d mesh"<<std::endl;
        TRIANGLE_PTR f_ptr = mesh.pTriArr;
        for(int iface = 0 ; iface < mesh.triangle_num ; ++iface )
        {
            float cx=0,cy=0;
            cx += v[f_ptr->i1 - 3].x;
            cy += v[f_ptr->i1 - 3].y;
            cx += v[f_ptr->i2 - 3].x;
            cy += v[f_ptr->i2 - 3].y;
            cx += v[f_ptr->i3 - 3].x;
            cy += v[f_ptr->i3 - 3].y;
            cx /= 3.0;
            cy /= 3.0;
            int cix = getXForFlood(cx);
            int ciy = getYForFlood(cy);
            //check if the center is inside this plane
            //if not do not add it
            if( p.id_!=getNearByPositiveID(cix,ciy) ){
//                std::cout<<"delete triangle"<<std::endl;
//                std::cout<<"p.id:"<<p.id_<<std::endl;
//                std::cout<<"nearby id:"<<getNearByPositiveID(cix,ciy)<<std::endl;
//                std::cout<<_PolygonLabel.at<float>(cix,ciy)<<":"<<cix<<","<<ciy<<std::endl;
                f_ptr = f_ptr->pNext;
                if(f_ptr==NULL)break;
                continue;
            }
            std::vector<DefaultMesh::VertexHandle> vhandle_face;
            vhandle_face.push_back(vhandle[v2idx[f_ptr->i1 - 3]]);
            vhandle_face.push_back(vhandle[v2idx[f_ptr->i2 - 3]]);
            vhandle_face.push_back(vhandle[v2idx[f_ptr->i3 - 3]]);
            DefaultMesh::FaceHandle f = mesh_.add_face(vhandle_face);
            //set the color for this face to this plane
            mesh_.set_color(vhandle_face[0],DefaultMesh::Color(qRed(c),qGreen(c),qBlue(c)));
            mesh_.set_color(vhandle_face[1],DefaultMesh::Color(qRed(c),qGreen(c),qBlue(c)));
            mesh_.set_color(vhandle_face[2],DefaultMesh::Color(qRed(c),qGreen(c),qBlue(c)));
            mesh_.set_color(f,DefaultMesh::Color(qRed(c),qGreen(c),qBlue(c)));
            f_ptr = f_ptr->pNext;
            if(f_ptr==NULL)break;
        }
        std::cout<<"end plane:"<<p.id_<<std::endl;
//        if(piter==cr.plane_lst_.begin()+1)break;
    }
    std::cerr<<mesh_.n_faces()<<" faces generated"<<std::endl;
    //
}

void DesignToMesh::get_dash_state(const DefaultMesh& mesh, const std::vector<DefaultMesh::VertexHandle> &vhandles)
{
    std::cerr<<"get dash state for each vertex"<<std::endl;
    DefaultMesh::ConstVertexIter vIt(mesh_.vertices_begin());
    DefaultMesh::ConstVertexIter vEnd(mesh_.vertices_end());

    typedef DefaultMesh::Point Point;
    using OpenMesh::Vec3f;

    Vec3f bbMin, bbMax;

    bbMin = bbMax = OpenMesh::vector_cast<Vec3f>(mesh_.point(*vIt));

    for (size_t count=0; vIt!=vEnd; ++vIt, ++count)
    {
        bbMin.minimize( OpenMesh::vector_cast<Vec3f>(mesh_.point(*vIt)));
        bbMax.maximize( OpenMesh::vector_cast<Vec3f>(mesh_.point(*vIt)));
    }
    float radius = (bbMin-bbMax).norm()*0.5;
    float same_vertex_th = radius*g_config->getDouble("same_vertex_threshod");
    dash_.resize(mesh.n_vertices());
    arma::fmat v((float*)mesh.points(),3,mesh.n_vertices(),false,true);
    ArmaKDTreeInterface<arma::fmat> arma_points(v);
    nanoflann::KDTreeSingleIndexAdaptor<
            nanoflann::L2_Simple_Adaptor<float,ArmaKDTreeInterface<arma::fmat>>,
            ArmaKDTreeInterface<arma::fmat>,
            3,arma::uword>
            kdtree(3,arma_points,nanoflann::KDTreeSingleIndexAdaptorParams(3));
    kdtree.buildIndex();
    for(std::vector<LineSegment>::iterator iter=_LineList.begin();iter!=_LineList.end();++iter)
    {
        arma::fvec p0(3,arma::fill::zeros),p1(3,arma::fill::zeros);
        p0(0) = iter->GetP1().GetX();
        p0(1) = iter->GetP1().GetY();
        p1(0) = iter->GetP2().GetX();
        p1(1) = iter->GetP2().GetY();
        std::vector<std::pair<arma::uword,float>> index_dist;
        kdtree.radiusSearch(p0.memptr(),same_vertex_th,index_dist,nanoflann::SearchParams());
        std::vector<std::pair<arma::uword,float>>::iterator piter;
        for(piter=index_dist.begin();piter!=index_dist.end();++piter)
        {
            const DefaultMesh::VertexHandle h0 = vhandles[piter->first];
            for(DefaultMesh::ConstVertexVertexIter vviter = mesh.cvv_begin(h0);vviter.is_valid();++vviter)
            {
                const DefaultMesh::Point& p = mesh.point(*vviter);
                arma::fvec pk((float*)p.data(),3,true,true);
                if(same_vertex_th > arma::norm(p1 - pk))
                {
                    if(iter->GetIsDash()){
                        dash_[h0.idx()] = 1;
                        dash_[vviter->idx()] = 1;
                    }
                    else{
                        if(dash_[vviter->idx()]<1)dash_[vviter->idx()] = -1;
                        if(dash_[h0.idx()]<1)dash_[h0.idx()] = -1;
                    }
                }
            }
        }
    }
}

void DesignToMesh::save_mesh(const std::string& filepath)
{
    std::cerr<<"saving mesh"<<std::endl;
    std::string meshpath,conpath,dashpath;
    QFileInfo info(QString::fromStdString(g_parameters.InputFilePath));
    std::cerr<<"filepath:"<<filepath<<std::endl;
    meshpath = filepath + "/" + info.baseName().toStdString() + ".ply";
    conpath = filepath + "/" + info.baseName().toStdString() + ".sp_imat.arma";
    dashpath = filepath +"/"+ info.baseName().toStdString() + ".col_int.arma";
    QFileInfo oinfo(QString::fromStdString(meshpath));
    QFileInfo ocinfo(QString::fromStdString(conpath));
    QFileInfo odinfo(QString::fromStdString(dashpath));
    std::cerr<<"saving mesh to:"<<oinfo.filePath().toStdString()<<std::endl;
    std::cerr<<"saving connection to:"<<ocinfo.filePath().toStdString()<<std::endl;
    std::cerr<<"saving dash state to:"<<odinfo.filePath().toStdString()<<std::endl;
    DefaultMesh mesh;
    mesh.request_vertex_colors();
    mesh.request_vertex_normals();
    mesh.request_face_colors();
    mesh.request_face_normals();
    __gnu_cxx::hash_map<uint32_t,uint32_t> old_vidx_to_new_vidx;
    std::vector<DefaultMesh::VertexHandle> vhandles;
    for(DefaultMesh::FaceIter iter = mesh_.faces_begin();iter!=mesh_.faces_end();++iter)
    {
        std::vector<DefaultMesh::VertexHandle> fvhandle;
        for(DefaultMesh::FaceVertexIter fviter = mesh_.fv_begin(*iter);fviter!=mesh_.fv_end(*iter);++fviter)
        {
            if(old_vidx_to_new_vidx.end()==old_vidx_to_new_vidx.find(fviter->idx()))
            {
                old_vidx_to_new_vidx[fviter->idx()] = vhandles.size();
                vhandles.push_back(mesh.add_vertex(mesh_.point(*fviter)));
                fvhandle.push_back(vhandles.back());
                mesh.set_color(vhandles.back(),mesh_.color(*fviter));
            }else{
                fvhandle.push_back(vhandles[old_vidx_to_new_vidx[fviter->idx()]]);
            }
        }
        mesh.set_color(mesh.add_face(fvhandle),mesh_.color(*iter));
    }
    mesh.update_normals();
    OpenMesh::IO::Options opt;
    opt+=OpenMesh::IO::Options::Binary;
    opt+=OpenMesh::IO::Options::VertexColor;
    opt+=OpenMesh::IO::Options::VertexNormal;
    opt+=OpenMesh::IO::Options::FaceColor;
    opt+=OpenMesh::IO::Options::FaceNormal;
    if(!OpenMesh::IO::write_mesh(mesh,oinfo.filePath().toStdString(),opt,13)){
        std::cerr<<"can't save to:"<<oinfo.filePath().toStdString()<<std::endl;
        return;
    }
    std::cout<<"plane connection:"<<std::endl;
    for(arma::sp_imat::iterator iter=lr_ptr_->plane_connetion_.begin();iter!=lr_ptr_->plane_connetion_.end();++iter)
    {
        std::cout<<iter.row()+lr_ptr_->start_plane_<<"->"<<iter.col()+lr_ptr_->start_plane_<<std::endl;
    }
    if(!lr_ptr_->plane_connetion_.save(ocinfo.filePath().toStdString(),arma::arma_binary))
    {
        std::cerr<<"can't save to:"<<ocinfo.filePath().toStdString()<<std::endl;
        return;
    }
    std::cout<<"dash state:"<<std::endl;
    get_dash_state(mesh,vhandles);
    arma::Col<int> dash = arma::conv_to<arma::Col<int>>::from(dash_);
    if(!dash.save(odinfo.filePath().toStdString(),arma::raw_ascii))
    {
        std::cerr<<"can't save to:"<<odinfo.filePath().toStdString()<<std::endl;
        return;
    }
}

void DesignToMesh::save_flooding_map(const std::string& filepath)
{
    QFileInfo info(QString::fromStdString(g_parameters.InputFilePath));
//    std::string mappath = filepath + "/" + info.baseName().toStdString() + ".color.png";
    std::string region_path = filepath + "/" + info.baseName().toStdString() + ".region.png";
//    QFileInfo mapinfo(QString::fromStdString(mappath));
    QFileInfo regioninfo(QString::fromStdString(region_path));
//    std::cerr<<"saving color map to:"<<mapinfo.filePath().toStdString()<<std::endl;
    std::cerr<<"saving largest region map to:"<<regioninfo.filePath().toStdString()<<std::endl;
//    QImage map_img(100*lr_ptr_->size(),100,QImage::Format_RGB888);
    float step = 300.0/float(plane_num_);
//    map_img.fill(Qt::gray);
//    for(int r = 0;r<map_img.height();++r)
//        for(int c=0;c<map_img.width();++c)
//        {
//            int id =  ( c / 100 ) + lr_ptr_->start_plane_;
//            QColor color;
//            color.setHsvF(id*step/360.0,0.6,0.75);
//            map_img.setPixel(c,r,color.rgb());
//        }
//    map_img.save(mapinfo.filePath());
    QImage region_img(_PolygonLabel.cols,_PolygonLabel.rows,QImage::Format_RGB888);
    region_img.fill(Qt::gray);
    for(int r = 0;r<_PolygonLabel.rows;++r)
        for(int c=0;c<_PolygonLabel.cols;++c)
        {
            QRgb rgb;
            if(_PolygonLabel.at<float>(r, c)==-1)
            {
                rgb = qRgb(0,0,0);
                region_img.setPixel(c,r,rgb);
            }else if(_PolygonLabel.at<float>(r, c)==-2)
            {
                rgb = qRgb(255,255,255);
                region_img.setPixel(c,r,rgb);
            }else if(_PolygonLabel.at<float>(r, c)>1&&_PolygonLabel.at<float>(r, c)>=lr_ptr_->start_plane_&&_PolygonLabel.at<float>(r, c)<lr_ptr_->end_plane_){
                QColor color;
                color.setHsvF(_PolygonLabel.at<float>(r, c)*step/360.0,0.6,0.75);
                rgb = color.rgb();
                region_img.setPixel(c,r,rgb);
            }
        }
    region_img.save(regioninfo.filePath());
}

DesignToMesh::~DesignToMesh()
{
    ;
}
