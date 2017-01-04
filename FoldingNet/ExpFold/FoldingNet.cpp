#include "FoldingNet.h"
#include <iostream>
#include <fstream>
#include "GL/freeglut.h"

/*#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> //glm::translate, glm::rotate, glm::scale, glm::perspective
*/
/// record the state of mouse
GLboolean FoldingNet::mouserdown = GL_FALSE;
GLboolean FoldingNet::mouseldown = GL_FALSE;
GLboolean FoldingNet::mousemdown = GL_FALSE;

/// when a mouse-key is pressed, record current mouse position 
GLint FoldingNet::mousex = 0;
GLint FoldingNet::mousey = 0;

GLfloat FoldingNet::center[3] = { 0.0f, 0.0f, 0.0f }; /// center position
GLfloat  FoldingNet::eye[3] = { 0.0f, 0.0f, 0.0f }; /// eye's position

GLfloat  FoldingNet::yrotate = g_parameters.MyPi / 4; /// angle between y-axis and look direction
GLfloat  FoldingNet::xrotate = g_parameters.MyPi / 4; /// angle between x-axis and look direction
GLfloat  FoldingNet::celength = 20.0f;/// lenght between center and eye

GLfloat  FoldingNet::mSpeed = 0.4f; /// center move speed
GLfloat  FoldingNet::rSpeed = 0.02f; /// rotate speed
GLfloat  FoldingNet::lSpeed = 0.4f; /// reserved

//opencv
double FoldingNet::mx = 0, FoldingNet::my = 0;
int FoldingNet::dx = 0, FoldingNet::dy = 0, FoldingNet::horizBar_x = 0, FoldingNet::vertiBar_y = 0;
bool FoldingNet::clickVertiBar = false, FoldingNet::clickHorizBar = false, FoldingNet::needScroll = false;
CvRect FoldingNet::rect_bar_horiz = { 0,0,1,1 }, FoldingNet::rect_bar_verti = {0,0,1,1};
int FoldingNet::src_x = 0, FoldingNet::src_y = 0; // 源图像中 rect_src 的左上角位置

GLfloat FoldingNet::light_position[4] = { 50,50,100,0 };
GLfloat FoldingNet::shadowMat[4][4] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };


FoldingNet::FoldingNet()
{
	pThis = this;
}


FoldingNet::~FoldingNet()
{
	_LineList.clear();
	_PolygonList.clear();
}

FoldingNet *FoldingNet::pThis = NULL;

void FoldingNet::LoadParameters()
{
	ifstream para("./data/parameters.txt");
	string type;
	float t_float;
	int t_int;
	string t_str;
	while (para)
	{
		para >> type;
		if (type == "MyPi")
		{
			para >> t_float;
			g_parameters.MyPi = t_float;
		}
		else if (type == "InputFilePath")
		{
			para >> t_str;
			g_parameters.InputFilePath = t_str;
		}
		else if (type == "MyEpsilon")
		{
			para >> t_float;
		    g_parameters.MyEpsilon = t_float;
		}
		else if (type == "MyInfinity")
		{
			para >> t_int;
			g_parameters.MyInfinity = t_int;
		}
		else if (type == "ResolutionMultipe")
		{
			para >> t_int;
			g_parameters.ResolutionMultipe = t_int;
		}
		else if (type == "ThresholdForSeparateLines")
		{
			para >> t_float;
			g_parameters.ThresholdForSeparateLines = t_float;
		}
	}

	para.close();

}

void FoldingNet::ReadLinesFromTxt()
{
	cout << "Reading Line Segments From TXT..." << endl;
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

	//需要知道二维坐标点的大概范围，以便使用Flood方法
	//查找二维点坐标中XY的最大最小值
	int t_max_x = 0;
	int t_max_y = 0;
	int t_min_x = 100000;
	int t_min_y = 100000;

	int isdash;
	//逐行读取
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

	//防止线段重复读取
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

	//测试List里是否存入正确数值
	/*for (int i = 0; i < 6; i++)
	{
		cout << _LineList[i].GetP1().GetX() << "    " << _LineList[i].GetP1().GetY() << "     "
			<< _LineList[i].GetP2().GetX() << "     " << _LineList[i].GetP2().GetY() << endl;
	}
	cout << _LineList.size() << endl;*/

	//切割线段
	CutOffLines();

	/*ofstream file1("./Lines_600.txt");
	for (int i = 0; i < _LineList.size(); i++)
	{
		file1 << _LineList[i].GetP1().GetX() << " " << _LineList[i].GetP1().GetY() << " " << _LineList[i].GetP2().GetX() << " " << _LineList[i].GetP2().GetY() << endl;
	}
	file1.close();*/
}

void FoldingNet::CutOffLines()
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

cv::Mat FoldingNet::GrayToRGB(cv::Mat img)
{
	cv::Mat img_pseudocolor(img.rows, img.cols, CV_8UC3);//构造RGB图像，参数CV_8UC3教程文档里面有讲解  
	int tmp = 0;
	for (int y = 0; y < img.rows; y++)//转为伪彩色图像的具体算法  
	{
		for (int x = 0; x < img.cols; x++)
		{
			tmp = img.at<unsigned char>(y, x);
			//没有平面的部分值为0，render时颜色设置为白色
			if (tmp == 0)
			{
				img_pseudocolor.at<cv::Vec3b>(y, x)[0] = 255; //blue  
				img_pseudocolor.at<cv::Vec3b>(y, x)[1] = 255; //green  
				img_pseudocolor.at<cv::Vec3b>(y, x)[2] = 255; //red 
			}
			else
			{
				img_pseudocolor.at<cv::Vec3b>(y, x)[0] = abs(255 - tmp); //blue  
				img_pseudocolor.at<cv::Vec3b>(y, x)[1] = abs(127 - tmp); //green  
				img_pseudocolor.at<cv::Vec3b>(y, x)[2] = abs(0 - tmp); //red 
			}
			 
		}
	}
	return img_pseudocolor;
}

cv::Mat FoldingNet::GrayToRainbow(cv::Mat img)
{
	cv::Mat img_color(img.rows, img.cols, CV_8UC3);//构造RGB图像  

	uchar tmp2 = 0;
	for (int y = 0; y < img.rows; y++)//转为彩虹图的具体算法，主要思路是把灰度图对应的0～255的数值分别转换成彩虹色：红、橙、黄、绿、青、蓝。  
	{
		for (int x = 0; x < img.cols; x++)
		{
			tmp2 = img.at<uchar>(y, x);
			if (tmp2 <= 51)
			{
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
				img_color.at<cv::Vec3b>(y, x)[0] = tmp2 * 5;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
			}
			else if (tmp2 <= 102)
			{
				tmp2 -= 51;
				img_color.at<cv::Vec3b>(y, x)[0] = 255 - tmp2 * 5;
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
			}
			else if (tmp2 <= 153)
			{
				tmp2 -= 102;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
				img_color.at<cv::Vec3b>(y, x)[0] = tmp2 * 5;
			}
			else if (tmp2 <= 204)
			{
				tmp2 -= 153;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
				img_color.at<cv::Vec3b>(y, x)[0] = 255 - uchar(128.0*tmp2 / 51.0 + 0.5);
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
			}
			else
			{
				tmp2 -= 204;
				img_color.at<cv::Vec3b>(y, x)[0] = 0;
				img_color.at<cv::Vec3b>(y, x)[0] = 127 - uchar(127.0*tmp2 / 51.0 + 0.5);
				img_color.at<cv::Vec3b>(y, x)[0] = 255;
			}
		}
	}
	return img_color;
}

void FoldingNet::Show2DPattern()
{
	if (pThis == NULL) return;

	glutKeyboardFunc(KeyFunc);   // 键盘按键
	glutMouseFunc(MouseFunc);   // 鼠标按键
	glutMotionFunc(MouseMotion); //鼠标移动

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 200);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(25, 25, 25, 0, 0, 0, 0, 0, 1);
	LookAt();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// 绘制三维坐标系
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1000.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1000.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1000.0f);
	glEnd();


	glBegin(GL_LINES);
	for (int i = 0; i < pThis->_LineList.size(); i++)
	{
		//画实虚线的问题
		glColor3f(0.0f, 0.0f, 0.0f);
		Point p1 = pThis->_LineList[i].GetP1();
		Point p2 = pThis->_LineList[i].GetP2();
		if (pThis->_LineList[i].GetIsDash() == 1)
			glColor3f(1.0f, 0.0f, 1.0f);
		//将原来的点缩小至30倍后方便查看
		glVertex3f(p1.GetX() / 30, p1.GetY() / 30, 0);
		glVertex3f(p2.GetX() / 30, p2.GetY() / 30, 0);
	}

	glEnd();
	glFlush();

}

void FoldingNet::BresenhamRasterization(LineSegment lineSeg, int id)
{
	//http://blog.csdn.net/wozhengtao/article/details/51431389
	//线段像素化
	//Bresenham算法

	int y1 = round((lineSeg.GetP1().GetY() - _MinPY)*g_parameters.ResolutionMultipe) + 1;
	int x1 = round((lineSeg.GetP1().GetX() - _MinPX)*g_parameters.ResolutionMultipe) + 1;
	int y2 = round((lineSeg.GetP2().GetY() - _MinPY)*g_parameters.ResolutionMultipe) + 1;
	int x2 = round((lineSeg.GetP2().GetX() - _MinPX)*g_parameters.ResolutionMultipe) + 1;


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
			_LineLabel.at<float>(y,x) = id;
			_PolygonLabel.at<float>(y,x) = id;
		}
		else
		{
			_LineLabel.at<float>(x, y) = id;
			_PolygonLabel.at<float>(x, y) = id;
		}
		
		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}

}

void FoldingNet::FloodFill4Stack(int x, int y, int planenumber)
{
	//Floodfill算法找平面
	//http://lodev.org/cgtutor/floodfill.html#4-Way_Method_With_Stack

	vector<LineSegment> LinesInOnePlane;
	Plane t_plane;

	if (planenumber == 0) //avoid infinite loop
		return;
	_MyStack.empty();

	int dx[4] = { 0, 1, 0, -1 }; // relative neighbor x coordinates
    int dy[4] = { -1, 0, 1, 0 }; // relative neighbor y coordinates

	_MyStack.push(Point(x, y));

	while (_MyStack.size() > 0)
	{
		x = int(_MyStack.top().GetX());
		y = int(_MyStack.top().GetY());
		_MyStack.pop();
		_PolygonLabel.at<float>(x, y) = planenumber;
		for (int i = 0; i < 4; i++) 
		{
			int nx = x + dx[i];
			int ny = y + dy[i];
			if (nx >= 0 && nx < _RangeofPX && ny >= 0 && ny < _RangeofPY && _PolygonLabel.at<float>(nx, ny) == 0)
			{
				_MyStack.push(Point(nx, ny));
			}

			//需要将检测到的边缘线段存储至PolygonList[i]中
			else if (nx >= 0 && nx < _RangeofPX && ny >= 0 && ny < _RangeofPY &&_LineLabel.at<float>(nx, ny) != 0)
			{
				/*for (int kk = 1; kk < 5; kk++)
					if (nx - dx[i] * kk >= 0 && nx - dx[i] * kk < _RangeofPX && ny - dy[i] * kk >= 0 && ny - dy[i] * kk < _RangeofPY)
						_PolygonLabel.at<float>(nx - dx[i] * kk, ny - dy[i] * kk) = planenumber - 1;*/

				//_PolygonLabel.at<float>(nx - dx[i], ny - dy[i]) = planenumber -1;

				if (LinesInOnePlane.size() == 0)
					LinesInOnePlane.push_back(_LineList[_LineLabel.at<float>(nx, ny)-1]);
				else
				{
					int t_found = 0; //遍历查找id值，若找到即为1，若没找到即为0
					for (int j = 0; j < LinesInOnePlane.size(); j++)
					{
						if (LinesInOnePlane[j].GetId() == _LineLabel.at<float>(nx, ny))
							t_found++;
					}
					
					//若没有找到该id对应的线段，则将其加入polygon
					if (t_found == 0)
					{
						LinesInOnePlane.push_back(_LineList[_LineLabel.at<float>(nx, ny) - 1]);
					}
										
				}
			}
				
		}
	}
	t_plane.SetPlane(LinesInOnePlane);
	t_plane.SetPlaneNumber(planenumber);
	_PolygonList.push_back(t_plane);    //将同一个多边形内的线段存储至一个Polygon，暂时线段无序存储
}

void FoldingNet::FindPolygonByFloodFill()
{
	
	_LineLabel = cv::Mat::zeros(_RangeofPX, _RangeofPY, CV_32FC1);
	_PolygonLabel = cv::Mat::zeros(_RangeofPX, _RangeofPY, CV_32FC1);

	cout << "Bresenham Rasterization..." << endl;
	//线段像素化
	for (int i = 0; i < _LineList.size(); i++)
		BresenhamRasterization(_LineList[i], _LineList[i].GetId());

	//查看像素化结果正确与否

	//旋转90度后的显示结果
	//cv::Mat _LineLabel_rotate;
	//cv::transpose(_LineLabel, _LineLabel_rotate);
	//cv::flip(_LineLabel_rotate, _LineLabel_rotate, 1);
	////_LineLabel_rotate.convertTo(_LineLabel_rotate, CV_8UC1);
	////_LineLabel_rotate = GrayToRGB(_LineLabel_rotate);
	////_LineLabel_rotate = _LineLabel_rotate <1;
	//imshow("Rasterization", _LineLabel_rotate);
	//cvWaitKey();

	/*imshow("Rasterization", _LineLabel);
	cvWaitKey();*/
		

	cout << "Find Polygon By FloodFill..." << endl;
	//怎么能遍历全图，填充完全图！简单点！
	//存在外部空白区域的问题需要解决

	int planenumber = 5;
	for (int i = 0; i < _RangeofPX-1; i++)
		for (int j = 0; j < _RangeofPY-1; j++)
		{
			if (_PolygonLabel.at<float>(i, j) == 0)
			{
				FloodFill4Stack(i, j, planenumber);
				planenumber = planenumber + 15;
			}
		}

	//删除最大的矩形
	//将删除的平面处的值设为0
	for (int i = 0; i < _RangeofPX - 1; i++)
		for (int j = 0; j < _RangeofPY - 1; j++)
		{
			if (_PolygonLabel.at<float>(i, j) == _PolygonList[0].GetPlaneNumber())
				_PolygonLabel.at<float>(i, j) = 0;
		}
	_PolygonList.erase(_PolygonList.begin());


	//若多边形由实线包裹而成，则删除该平面
	vector<Plane>::iterator it;
	for (it = _PolygonList.begin(); it != _PolygonList.end();)
	{
		int number_solid = 0;
		for (int j = 0; j < (*it).NumberofLines(); j++)
			if ((*it).IthLine(j).GetIsDash() == 0)
				number_solid++;

		if (number_solid == (*it).NumberofLines())
		{
			//将删除的平面处的_PolygonLabel值设为0
			for (int i = 0; i < _RangeofPX - 1; i++)
				for (int j = 0; j < _RangeofPY - 1; j++)
				{
					if (_PolygonLabel.at<float>(i, j) == (*it).GetPlaneNumber())
						_PolygonLabel.at<float>(i, j) = 0;
				}
			it = _PolygonList.erase(it);
		}
			
		else
		{
			//且对每个多边形的边缘进行重排序
			//(*it).ReorderingOfEdges();
			(*it).FindLoop();
			++it;
		}		
	}	


	/*_PolygonLabel.convertTo(_PolygonLabel, CV_8UC1);
	_PolygonLabel = GrayToRGB(_PolygonLabel);
	imshow("Test for FloodFill", _PolygonLabel);
	cvWaitKey();*/

	/*for (int i = 0; i < _LineLabel.rows; i++)
	{
		for (int j = 0; j < _LineLabel.cols; j++)
		{
			cout << _LineLabel.at<float>(i, j) << "   ";
		}
		cout << "+++++++++++++++++++++++++++++++++++++++" << endl;
	}*/

	//输出label信息
	/*ofstream file1("./labelLine.txt");
	for (int i = 0; i < _LineLabel.rows; i++)
	{
		for (int j = 0; j < _LineLabel.cols; j++)
		{
			file1 << _LineLabel.at<float>(i, j) << " ";
		}
		file1 << endl;
	}
	file1.close();

	ofstream file("./label.txt");
	for (int i = 0; i < _LineLabel.rows; i++)
	{
		for (int j = 0; j < _LineLabel.cols; j++)
		{
			file << _PolygonLabel.at<float>(i, j) <<" ";
		}
		file << endl;
	}
	file.close();*/

	//显示flooding结果
	//cv::Mat _PolygonLabel_rotate;
	//_PolygonLabel.convertTo(_PolygonLabel_rotate, CV_8UC1);

	//_PolygonLabel_rotate = GrayToRGB(_PolygonLabel_rotate);
	//cv::transpose(_PolygonLabel_rotate, _PolygonLabel_rotate);
	//cv::flip(_PolygonLabel_rotate, _PolygonLabel_rotate, 1);
	//cv::namedWindow("FloodFill Result");
 //   cvSetMouseCallback("FloodFill Result", MouseClick);
	////imshow("FloodFill Result", _PolygonLabel_rotate);

	//while (1)
	//{
	//	myShowImageScroll("FloodFill Result", &(IplImage)_PolygonLabel_rotate, 1600, 900);

	//	int KEY = cvWaitKey(10);
	//	if ((char)KEY == 27)   //Esc
	//		break;

	//}
	//cv::destroyWindow("FloodFill Result");
}

void FoldingNet::Modeling()
{
	cout << "Generate 3D model ..." << endl;
	//三角化...到底有啥用...
	//对每个面进行三角化
	for (int k = 0; k < _PolygonList.size(); k++)
	{
		_PolygonList[k].Triangulation();
	}


	//构建Graph
	GraphNode t_node;
	GraphEdge t_edge;
	for (int i = 0; i < _PolygonList.size(); i++)
	{
		t_node.SetPlaneNumber(i + 1);
		for (int j = 0; j < _PolygonList[i].NumberofLines(); j++)
		{
			if (_PolygonList[i].IthLine(j).GetIsDash() == 1)
			{
				for (int k = i+1 ; k < _PolygonList.size(); k++)			
					for (int l = 0; l < _PolygonList[k].NumberofLines(); l++)
						if (_PolygonList[i].IthLine(j) == _PolygonList[k].IthLine(l))
						{
							cout << "Edge Between : " << i + 1 << "  " << k + 1 << endl;
							t_edge.SetPlaneNumber(i + 1, k + 1);
							t_edge.SetId(_PolygonList[i].IthLine(j).GetId());
							
							t_node.AddNeighbourNumber(k + 1);
							
							//先手动设定角度  TestBox_1
							/*if (((i + 1) == 3 && (k + 1) == 5)
								|| ((i + 1) == 4 && (k + 1) == 6)
								|| ((i + 1) == 8 && (k + 1) == 11)
								|| ((i + 1) == 10 && (k + 1) == 12)
								|| ((i + 1) == 13 && (k + 1) == 15)
								|| ((i + 1) == 14 && (k + 1) == 16)
								|| ((i + 1) == 19 && (k + 1) == 23)
								|| ((i + 1) == 21 && (k + 1) == 24))
								t_edge.SetAngle(90);
							else
								t_edge.SetAngle(0);
							_Graph.AddEdge(t_edge);*/

							//box_643
							/*if ((((i + 1) == 2 && (k + 1) == 4)
								|| (i + 1) == 1 && (k + 1) == 5)
								|| ((i + 1) == 4 && (k + 1) == 5)
								|| ((i + 1) == 5 && (k + 1) == 6)
								|| ((i + 1) == 5 && (k + 1) == 10))
								t_edge.SetAngle(90);

							else if (((i + 1) == 4 && (k + 1) == 9)
								|| ((i + 1) == 6 && (k + 1) == 11)
								|| ((i + 1) == 3 && (k + 1) == 6)
								|| ((i + 1) == 6 && (k + 1) == 7)
								|| ((i + 1) == 7 && (k + 1) == 8))
								t_edge.SetAngle(-90);
							else
								t_edge.SetAngle(0);
							_Graph.AddEdge(t_edge);*/

							//box_627
							//Not all folded
							/*if (((i + 1) == 2 && (k + 1) == 7)
								|| ((i + 1) == 8 && (k + 1) == 9)
								|| ((i + 1) == 9 && (k + 1) == 14))
								t_edge.SetAngle(90);
							else if (((i + 1) == 7 && (k + 1) == 15)
								|| ((i + 1) == 8 && (k + 1) == 16))
								t_edge.SetAngle(-90);
							else
								t_edge.SetAngle(0);
							_Graph.AddEdge(t_edge);*/

							//all folded
							/*if (((i + 1) == 2 && (k + 1) == 7)
								|| ((i + 1) == 8 && (k + 1) == 9)
							    || ((i + 1) == 9 && (k + 1) == 14))
							t_edge.SetAngle(90);

							else if (((i + 1) == 1 && (k + 1) == 4)
								|| ((i + 1) == 3 && (k + 1) == 9)
								|| ((i + 1) == 4 && (k + 1) == 8)
								|| ((i + 1) == 5 && (k + 1) == 6)
								|| ((i + 1) == 6 && (k + 1) == 7)
								|| ((i + 1) == 7 && (k + 1) == 15)
								|| ((i + 1) == 7 && (k + 1) == 8)
								|| ((i + 1) == 8 && (k + 1) == 16)
								|| ((i + 1) == 16 && (k + 1) == 17))
							    t_edge.SetAngle(-90);
							else
							    t_edge.SetAngle(0);
							_Graph.AddEdge(t_edge);*/

							//box_666
							if (((i + 1) == 1 && (k + 1) == 7)						
								|| ((i + 1) == 4 && (k + 1) == 5)								
								|| ((i + 1) == 7 && (k + 1) == 14)
								|| ((i + 1) == 15 && (k + 1) == 16)
								|| ((i + 1) == 9 && (k + 1) == 16)
								|| ((i + 1) == 10 && (k + 1) == 17)								
								|| ((i + 1) == 12 && (k + 1) == 19)
								)
								t_edge.SetAngle(90);

							else if (((i + 1) == 7 && (k + 1) == 8)
								|| ((i + 1) == 2 && (k + 1) == 3)
								|| ((i + 1) == 3 && (k + 1) == 9)
								|| ((i + 1) == 6 && (k + 1) == 12)
								|| ((i + 1) == 4 && (k + 1) == 10)
								|| ((i + 1) == 8 && (k + 1) == 9)
								|| ((i + 1) == 10 && (k + 1) == 11)
								|| ((i + 1) == 11 && (k + 1) == 12)
								|| ((i + 1) == 17 && (k + 1) == 18))
								t_edge.SetAngle(-90);

							else if(((i + 1) == 12 && (k + 1) == 13))
								t_edge.SetAngle(-180);

							else
								t_edge.SetAngle(0);
							_Graph.AddEdge(t_edge);
							//box_600
							/*if (((i + 1) == 1 && (k + 1) == 6)
								|| ((i + 1) == 2 && (k + 1) == 9)
								|| ((i + 1) == 3 && (k + 1) == 7)
								|| ((i + 1) == 9 && (k + 1) == 10)
								|| ((i + 1) == 6 && (k + 1) == 12)
								|| ((i + 1) == 9 && (k + 1) == 14)
								|| ((i + 1) == 5 && (k + 1) == 6))
								t_edge.SetAngle(90);

							else  if (((i + 1) == 6 && (k + 1) == 7)
								|| ((i + 1) == 8 && (k + 1) == 9)
								|| ((i + 1) == 4 && (k + 1) == 5)
								|| ((i + 1) == 5 && (k + 1) == 11)
								|| ((i + 1) == 3 && (k + 1) == 8)
								|| ((i + 1) == 7 && (k + 1) == 13)
								|| ((i + 1) == 8 && (k + 1) == 13))
								t_edge.SetAngle(-90);
							else
								t_edge.SetAngle(0);
							_Graph.AddEdge(t_edge);*/

							//box_002
							/*if (((i + 1) == 2 && (k + 1) == 3)
								|| ((i + 1) == 6 && (k + 1) == 7)
								|| ((i + 1) == 7 && (k + 1) == 8)
								|| ((i + 1) == 10 && (k + 1) == 11)
								|| ((i + 1) == 11 && (k + 1) == 12))
								t_edge.SetAngle(90);

							else  if (((i + 1) == 3 && (k + 1) == 7)
								|| ((i + 1) == 3 && (k + 1) == 4)
								|| ((i + 1) == 7 && (k + 1) == 11))
								t_edge.SetAngle(-90);

							else if(((i + 1) == 5 && (k + 1) == 6)
								|| ((i + 1) == 8 && (k + 1) == 9)
								|| ((i + 1) == 11 && (k + 1) == 13))
								t_edge.SetAngle(30);

							else if (((i + 1) == 1 && (k + 1) == 3))
								t_edge.SetAngle(-30);

							else
								t_edge.SetAngle(0);
							_Graph.AddEdge(t_edge);*/

							//box_004
							/*if (((i + 1) == 1 && (k + 1) == 2)
								|| ((i + 1) == 4 && (k + 1) == 9)
								|| ((i + 1) == 6 && (k + 1) == 10)
								|| ((i + 1) == 14 && (k + 1) == 15)
								|| ((i + 1) == 13 && (k + 1) == 16))
							   t_edge.SetAngle(90);
							else  if (((i + 1) == 2 && (k + 1) == 3)
								|| ((i + 1) == 2 && (k + 1) == 6)
								|| ((i + 1) == 4 && (k + 1) == 5)
								|| ((i + 1) == 5 && (k + 1) == 6)
								|| ((i + 1) == 15 && (k + 1) == 18)
								|| ((i + 1) == 10 && (k + 1) == 11)
								|| ((i + 1) == 10 && (k + 1) == 12)
								|| ((i + 1) == 4 && (k + 1) == 8)
								|| ((i + 1) == 10 && (k + 1) == 15)
								|| ((i + 1) == 13 && (k + 1) == 14)
								|| ((i + 1) == 13 && (k + 1) == 17)
								|| ((i + 1) == 13 && (k + 1) == 17))
								t_edge.SetAngle(-90);
							else
							t_edge.SetAngle(0);
							_Graph.AddEdge(t_edge);*/
							break;
						}								
			}
		}
		_Graph.AddNode(t_node);
		t_node.ClearNeighbour();
	}

	//上述操作的NeighbourNumber不完整，只有比自身大的平面
	//应双向设置neighbour

	for (int i = 0; i < _Graph.GetNodeSize(); i++)
	{
		for (int j = 0; j < _Graph.IthNode(i).GetNeighbourSize(); j++)
		{
			int neighbour_id = _Graph.IthNode(i).IthNeighbourNumber(j);
			if(neighbour_id > i+1)
				_Graph.SetIthNodeNeihbour(neighbour_id - 1, i );
		}
	}

	/*for (int i = 0; i < _Graph.GetNodeSize(); i++)
	{
		cout << i + 1 << "    ";
		for (int j = 0; j < _Graph.IthNode(i).GetNeighbourSize(); j++)
			cout << _Graph.IthNode(i).IthNeighbourNumber(j) << "    ";
		cout << endl;
	}*/

	//找最大的子图
	 _Graph.FindLargestSubGraph();

	/*for (int i = 0; i < _Graph.GetNodeSize(); i++)
	{
		cout << "PlaneNumber : "<<i+1<<"   Visited?   "<<_Graph.IthNode(i).GetIsVisited() << endl;
	}*/

	//建立最小生成树
	 vector<int> t_EdgeOrder;
	t_EdgeOrder = _Graph.MiniSpanTree_Prim();

	//根据最小生成树，选取一个面不动，计算其他面的旋转矩阵
	//选取第一个面作为底面
	

	//单元测试
	//float rtmatrix[4][4];
	//for (int i = 0; i < 4; i++)
	//	for (int j = 0; j < 4; j++)
	//		rtmatrix[i][j] = 0;

	//RotateArbitraryAxis(90, _Graph.IthEdge(3), rtmatrix);
	//
	//for (int i = 0; i < _PolygonList[4].NumberofLines(); i++)
	//{
	//	Vertex v1,v2;
	//	v1 = MatrixMulVertex(rtmatrix, _PolygonList[4].IthLine(i).GetV1());
	//	cout << v1.GetX() << "   " << v1.GetY() << "   " << v1.GetZ() << "   ";

	//	v2 = MatrixMulVertex(rtmatrix, _PolygonList[4].IthLine(i).GetV2());
	//	cout << v2.GetX() << "   " << v2.GetY() << "   " << v2.GetZ() << endl;

	//	_PolygonList[4].ChangeIthLine(i, v1, v2);
	//	
	//	cout << _PolygonList[4].IthLine(i).GetV1().GetX() << "    " << _PolygonList[4].IthLine(i).GetV1().GetY() << "   " << _PolygonList[4].IthLine(i).GetV1().GetZ() << "   "
	//		<< _PolygonList[4].IthLine(i).GetV2().GetX() << "   " << _PolygonList[4].IthLine(i).GetV2().GetY() << "   " <<_PolygonList[4].IthLine(i).GetV2().GetZ() << endl;
	//	/*_PolygonList[4].IthLine(i).SetV1(MatrixMulVertex(rtmatrix, _PolygonList[4].IthLine(i).GetV1()));
	//	_PolygonList[4].IthLine(i).SetV2(MatrixMulVertex(rtmatrix, _PolygonList[4].IthLine(i).GetV2()));*/
	//}


	//为每个面建立旋转矩阵
	float ***RTMatrix = new float **[_Graph.GetNodeSize()];
	for (int i = 0; i < _Graph.GetNodeSize(); i++)
	{
		RTMatrix[i] = new float *[4];
		for (int j = 0; j < 4; j++)
			RTMatrix[i][j] = new float[4];
	}

	//初始化旋转矩阵
	for (int i = 0; i < _Graph.GetNodeSize(); i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
			{
				if ((j == 0 && k == 0)
					|| (j == 1 && k == 1)
					|| (j == 2 && k == 2)
					|| (j == 3 && k == 3))
					RTMatrix[i][j][k] = 1;
				else
					RTMatrix[i][j][k] = 0;
			}


	int start_plane = 0,next_plane;

	//选定初始面，即为第一个平面
	//一步步叠加旋转矩阵
	//叠加旋转矩阵时，应从根节点依次往下进行叠加！！！！！！！！！！！！！！！！！！！！！！！！！！
	
	float rtmatrix[4][4];
	float temp_matrix[4][4];

	//cout << "Rotate Order: " << endl;
	for (int k = 0; k < t_EdgeOrder.size(); k = k + 2)
	{
		for (int i = 0; i < _Graph.GetEdgeSize(); i++)
		{
			_Graph.IthEdge(i).GetPlaneNumber(start_plane, next_plane);
			bool con1 = (start_plane == t_EdgeOrder[k] && next_plane == t_EdgeOrder[k + 1]);
			bool con2 = (start_plane == t_EdgeOrder[k + 1] && next_plane == t_EdgeOrder[k]);

			if (con1 || con2)
			{
				if (con2)
					swap(start_plane, next_plane);

				//cout << start_plane << "  " << next_plane << endl;
				RotateArbitraryAxis(_Graph.IthEdge(i).GetAngle(), _Graph.IthEdge(i), rtmatrix);

				//这个全局矩阵左乘的顺序要注意啊！
				for (int p = 0; p < 4; p++)
					for (int q = 0; q < 4; q++)
					{
						temp_matrix[p][q] = 0;
						for (int r = 0; r < 4; r++)
							temp_matrix[p][q] += RTMatrix[start_plane - 1][p][r] * rtmatrix[r][q];
					}

				for (int p = 0; p < 4; p++)
					for (int q = 0; q < 4; q++)
					{
						RTMatrix[next_plane - 1][p][q] = temp_matrix[p][q];
					}

			}
		}
	}
	

	//显示旋转矩阵
	/*for (int i = 0; i < _Graph.GetNodeSize(); i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
				cout << RTMatrix[i][j][k] << "   ";
			cout << endl;
		}
		cout << "+++++++++++++++++++++++" << endl;
	}*/

	for (int i = 0; i < _PolygonList.size(); i++)
	{
		for (int j = 0; j < _PolygonList[i].NumberofVertices(); j++)
			_PolygonList[i].ChangeIthVertex(j,MatrixMulVertex(RTMatrix[i], _PolygonList[i].IthVertex(j)));
		for (int j = 0; j < _PolygonList[i].NumberofLines(); j++)
		{
			_PolygonList[i].ChangeIthLine(j, MatrixMulVertex(RTMatrix[i], _PolygonList[i].IthLine(j).GetV1()), MatrixMulVertex(RTMatrix[i], _PolygonList[i].IthLine(j).GetV2()));
		}
	}

	for (int i = 0; i < _Graph.GetNodeSize(); i++)
		for (int j = 0; j < 4; j++)
			delete[]RTMatrix[i][j];
	delete[]RTMatrix;
}

void FoldingNet::RotateArbitraryAxis(float angle, GraphEdge edge, float rtmatrix[4][4])
{
	float a = _LineList[edge.GetId() - 1].GetV1().GetX();
	float b = _LineList[edge.GetId() - 1].GetV1().GetY();
	float c = _LineList[edge.GetId() - 1].GetV1().GetZ();

	float u = _LineList[edge.GetId() - 1].GetV2().GetX() - a;
	float v = _LineList[edge.GetId() - 1].GetV2().GetY() - b;
	float w = _LineList[edge.GetId() - 1].GetV2().GetZ() - c;

	//归一化
	u = u / sqrt(u*u + v*v + w*w);
	v = v / sqrt(u*u + v*v + w*w);
	w = w / sqrt(u*u + v*v + w*w);

	float uu = u * u;
	float uv = u * v;
	float uw = u * w;
	float vv = v * v;
	float vw = v * w;
	float ww = w * w;
	float au = a * u;
	float av = a * v;
	float aw = a * w;
	float bu = b * u;
	float bv = b * v;
	float bw = b * w;
	float cu = c * u;
	float cv = c * v;
	float cw = c * w;

	float costheta = cosf(angle*g_parameters.MyPi / 180);
	float sintheta = sinf(angle*g_parameters.MyPi / 180);

	rtmatrix[0][0] = uu + (vv + ww) * costheta;
	rtmatrix[1][0] = uv * (1 - costheta) + w * sintheta;
	rtmatrix[2][0] = uw * (1 - costheta) - v * sintheta;
	rtmatrix[3][0] = 0;

	rtmatrix[0][1] = uv * (1 - costheta) - w * sintheta;
	rtmatrix[1][1] = vv + (uu + ww) * costheta;
	rtmatrix[2][1] = vw * (1 - costheta) + u * sintheta;
	rtmatrix[3][1] = 0;

	rtmatrix[0][2] = uw * (1 - costheta) + v * sintheta;
	rtmatrix[1][2] = vw * (1 - costheta) - u * sintheta;
	rtmatrix[2][2] = ww + (uu + vv) * costheta;
	rtmatrix[3][2] = 0;

	rtmatrix[0][3] = (a * (vv + ww) - u * (bv + cw)) * (1 - costheta) + (bw - cv) * sintheta;
	rtmatrix[1][3] = (b * (uu + ww) - v * (au + cw)) * (1 - costheta) + (cu - aw) * sintheta;
	rtmatrix[2][3] = (c * (uu + vv) - w * (au + bv)) * (1 - costheta) + (av - bu) * sintheta;
	rtmatrix[3][3] = 1;

}

Vertex FoldingNet::MatrixMulVertex(float **rtmatrix, Vertex p)
{
	Vertex temp;
	float x = rtmatrix[0][0] * p.GetX() + rtmatrix[0][1] * p.GetY() + rtmatrix[0][2] * p.GetZ() + rtmatrix[0][3];
	float y = rtmatrix[1][0] * p.GetX() + rtmatrix[1][1] * p.GetY() + rtmatrix[1][2] * p.GetZ() + rtmatrix[1][3];
	float z = rtmatrix[2][0] * p.GetX() + rtmatrix[2][1] * p.GetY() + rtmatrix[2][2] * p.GetZ() + rtmatrix[2][3];

	temp.SetX(x);
	temp.SetY(y);
	temp.SetZ(z);
	temp.SetCorrespondingPoint(p.GetCorrespondingPoint());

	return temp;
}

void FoldingNet::PolyLine3DBegin(GLenum type)
{
	glBegin(type);
}

void FoldingNet::PolyLine3DVertex(GLdouble * vertex)
{
	const GLdouble *pointer = (GLdouble *)vertex;
	//glColor3d(1.0, 0, 0);//在此设置颜色  
	glVertex3dv(pointer);
}

void FoldingNet::PolyLine3DEnd()
{
	glEnd();
}

GLUtesselator * FoldingNet::tesser()
{
	GLUtesselator * tess;
	tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN, (void (CALLBACK*)())&PolyLine3DBegin);
	gluTessCallback(tess, GLU_TESS_VERTEX, (void (CALLBACK*)())&PolyLine3DVertex);
	gluTessCallback(tess, GLU_TESS_END, (void (CALLBACK*)())&PolyLine3DEnd);
	return tess;
}

void FoldingNet::OnReshape(int w, int h)
{
	// prevents division by zero when minimising window  
	if (h == 0)
	{
		h = 1;
	}
	// set the drawable region of the window  
	glViewport(0, 0, w, h);

	// set up the projection matrix  
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// just use a perspective projection  
	//gluPerspective(45,(float)w/h,0.1,100);  
	if (w <= h)
	{
		glOrtho(-20.0, 20.0, -20.0 * (GLfloat)h / (GLfloat)w, 20.0 * (GLfloat)h / (GLfloat)w, 0.0, 100.0);
	}
	else
	{
		glOrtho(-20.0, 20.0, -20.0 * (GLfloat)h / (GLfloat)w, 20.0 * (GLfloat)h / (GLfloat)w, 0.0, 100.0);
	}
	// go back to model view matrix so we can move the objects about  
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void FoldingNet::ShowPlane()
{
	if (pThis == NULL) return;

	glutKeyboardFunc(KeyFunc);   // 键盘按键
	glutMouseFunc(MouseFunc);   // 鼠标按键
	glutMotionFunc(MouseMotion); //鼠标移动

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 200);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(20, 20, 20, 0, 0, 0, 0, 0, 1);
	LookAt();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	//glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
	//glShadeModel(GL_SMOOTH);   // Enable smooth shading
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective corrections

	 //绘制三维坐标系
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1000.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1000.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1000.0f);
	glEnd();

	
	glBegin(GL_LINES);
	//显示一个多边形的顶点顺序连成的边

	Vertex pp;
	//int j = 1;
	/*for (int j = 0; j < pThis->_PolygonList.size(); j++)
	{
		glVertex3f(pThis->_PolygonList[j].IthVertex(0).GetCorrespondingPoint().GetX() / 40, pThis->_PolygonList[j].IthVertex(0).GetCorrespondingPoint().GetY() / 40, 0);
		for (int i = 1; i < pThis->_PolygonList[j].NumberofVertices(); i++)
		{
			pp = pThis->_PolygonList[j].IthVertex(i);
			glVertex3f(pp.GetCorrespondingPoint().GetX() / 40, pp.GetCorrespondingPoint().GetY() / 40, 0);
			glVertex3f(pp.GetCorrespondingPoint().GetX() / 40, pp.GetCorrespondingPoint().GetY() / 40, 0);
		}
		glVertex3f(pThis->_PolygonList[j].IthVertex(0).GetCorrespondingPoint().GetX() / 40, pThis->_PolygonList[j].IthVertex(0).GetCorrespondingPoint().GetY() / 40, 0);
	}*/


	int j = 18;
	//for (int j = 0; j <  pThis->_PolygonList.size(); j++)
		for (int i = 0; i < pThis->_PolygonList[j].NumberofLines(); i++) //pThis->_PolygonList[j].NumberofLines()
		{
			glColor3f(0, 0, 0);
			if (pThis->_PolygonList[j].IthLine(i).GetIsDash() == 1)
				glColor3f(1.0f, 0.0f, 1.0f);
			Point p1 = pThis->_PolygonList[j].IthLine(i).GetP1();
			Point p2 = pThis->_PolygonList[j].IthLine(i).GetP2();
			glVertex3f(p1.GetX() / 100, p1.GetY() / 100, 0);
			glVertex3f(p2.GetX() / 100, p2.GetY() / 100, 0);
		}

	//显示交点
	//int j =0;
	//for (int j = 0; j < pThis->_PolygonList.size(); j++)
	/*for (int i = 0; i < pThis->_PolygonList[j].NumberofVertices(); i++)
	{
	pp = pThis->_PolygonList[j].IthVertex(i);
	cout << pp.GetCorrespondingPoint().GetX() << "     " << pp.GetCorrespondingPoint().GetY() << endl;
	glVertex3f(pp.GetCorrespondingPoint().GetX() / 40 - 0.3f, pp.GetCorrespondingPoint().GetY() / 40 - 0.3f, 0);
	glVertex3f(pp.GetCorrespondingPoint().GetX() / 40 - 0.3f, pp.GetCorrespondingPoint().GetY() / 40 + 0.3f, 0);
	glVertex3f(pp.GetCorrespondingPoint().GetX() / 40 - 0.3f, pp.GetCorrespondingPoint().GetY() / 40 + 0.3f, 0);
	glVertex3f(pp.GetCorrespondingPoint().GetX() / 40 + 0.3f, pp.GetCorrespondingPoint().GetY() / 40 + 0.3f, 0);
	glVertex3f(pp.GetCorrespondingPoint().GetX() / 40 + 0.3f, pp.GetCorrespondingPoint().GetY() / 40 + 0.3f, 0);
	glVertex3f(pp.GetCorrespondingPoint().GetX() / 40 + 0.3f, pp.GetCorrespondingPoint().GetY() / 40 - 0.3f, 0);
	glVertex3f(pp.GetCorrespondingPoint().GetX() / 40 + 0.3f, pp.GetCorrespondingPoint().GetY() / 40 - 0.3f, 0);
	glVertex3f(pp.GetCorrespondingPoint().GetX() / 40 - 0.3f, pp.GetCorrespondingPoint().GetY() / 40 - 0.3f, 0);
	}*/

	glEnd();

	glFlush();

}

void FoldingNet::ShowTriangulation()
{
	if (pThis == NULL) return;
	
	glutKeyboardFunc(KeyFunc);   // 键盘按键
	glutMouseFunc(MouseFunc);   // 鼠标按键
	glutMotionFunc(MouseMotion); //鼠标移动

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 40);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(20, 20, 20, 0, 0, 0, 0, 0, 1);
	LookAt();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// 绘制三维坐标系
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1000.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1000.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1000.0f);
	glEnd();

	glBegin(GL_LINES);

	TRIANGLE_PTR pTri;
	//int k = 4;
	for (int k = 0; k < pThis->_PolygonList.size(); k++)
	{
		pThis->_PolygonList[k].Triangulation();
		pTri = pThis->_PolygonList[k].GetMesh().pTriArr;
		while (pTri != NULL)
		{
			glVertex3f(pThis->_PolygonList[k].IthVertex(pTri->i1 - 3).GetCorrespondingPoint().GetX() / 100, pThis->_PolygonList[k].IthVertex(pTri->i1 - 3).GetCorrespondingPoint().GetY() / 100, 0);
			glVertex3f(pThis->_PolygonList[k].IthVertex(pTri->i2 - 3).GetCorrespondingPoint().GetX() / 100, pThis->_PolygonList[k].IthVertex(pTri->i2 - 3).GetCorrespondingPoint().GetY() / 100, 0);
			glVertex3f(pThis->_PolygonList[k].IthVertex(pTri->i2 - 3).GetCorrespondingPoint().GetX() / 100, pThis->_PolygonList[k].IthVertex(pTri->i2 - 3).GetCorrespondingPoint().GetY() / 100, 0);
			glVertex3f(pThis->_PolygonList[k].IthVertex(pTri->i3 - 3).GetCorrespondingPoint().GetX() / 100, pThis->_PolygonList[k].IthVertex(pTri->i3 - 3).GetCorrespondingPoint().GetY() / 100, 0);
			glVertex3f(pThis->_PolygonList[k].IthVertex(pTri->i3 - 3).GetCorrespondingPoint().GetX() / 100, pThis->_PolygonList[k].IthVertex(pTri->i3 - 3).GetCorrespondingPoint().GetY() / 100, 0);
			glVertex3f(pThis->_PolygonList[k].IthVertex(pTri->i1 - 3).GetCorrespondingPoint().GetX() / 100, pThis->_PolygonList[k].IthVertex(pTri->i1 - 3).GetCorrespondingPoint().GetY() / 100, 0);
			pTri = pTri->pNext;
		}
	}

	glEnd();
	glFlush();
}

void FoldingNet::SetupRC()
{
	GLfloat ambientLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//glEnable(GL_DEPTH_TEST);    // Hidden surface removal
	//glEnable(GL_CULL_FACE);        // Do not calculate inside of jet
	//glFrontFace(GL_CCW);        // Counter clock-wise polygons face out
	

	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Nice light
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// any three points on the ground
	GLfloat points[3][3] = { {0,10,-0.1},{0,0,-0.1},{10,0,-0.1} };
	MakeShadowMatrix(points, light_position, shadowMat);
}

void FoldingNet::CalcNormal(float v[3][3], float out[3])
{
	///poiints p1,p2,p3 spectied in conter-wise order
	float v1[3], v2[3];
	static const int x = 0;
	static const int y = 1;
	static const int z = 2;

	///calculate two vectors from the three points
	v1[x] = v[0][x] - v[1][x];
	v1[y] = v[0][y] - v[1][y];
	v1[z] = v[0][z] - v[1][z];

	v2[x] = v[1][x] - v[2][x];
	v2[y] = v[1][y] - v[2][y];
	v2[z] = v[1][z] - v[2][z];

	///take the cross product of the two vectors to get the normal vectors, which will be stored in out[]
	out[x] = v1[y] * v2[z] - v1[z] * v2[y];
	out[y] = v1[z] * v2[x] - v1[x] * v2[z];
	out[z] = v1[x] * v2[y] - v1[y] * v2[x];

	///Normalize the vector
	float sum = out[x] + out[y] + out[z];
	out[x] = out[x] / sum;
	out[y] = out[y] / sum;
	out[z] = out[z] / sum;

}

void FoldingNet::MakeShadowMatrix(GLfloat points[3][3], GLfloat lightpos[4], GLfloat destMat[4][4])
{
	///creates a shadow projection matrix out of the plane equation
	///coefficients and the position of the light. The return value is stored in destMat[][]

	GLfloat planeCoff[4];
	GLfloat dot;
	/// find the plane equation coefficients
	///find the first coefficients the same way we find a normal 
	CalcNormal(points, planeCoff);

	///find the last coefficients by back substitutions
	planeCoff[3] = -(planeCoff[0] * points[2][0] + planeCoff[1] * points[2][1] + planeCoff[2] * points[2][2]);

	///dot product of plane and light position
	dot = planeCoff[0] * lightpos[0] + planeCoff[1] * lightpos[1] + planeCoff[2] * lightpos[2] + planeCoff[3] * lightpos[3];

	///Now do the projection

	destMat[0][0] = dot - lightpos[0] * planeCoff[0];
	destMat[1][0] = 0.0 - lightpos[0] * planeCoff[1];
	destMat[2][0] = 0.0 - lightpos[0] * planeCoff[2];
	destMat[3][0] = 0.0 - lightpos[0] * planeCoff[3];

	destMat[0][1] = 0.0 - lightpos[1] * planeCoff[0];
	destMat[1][1] = dot - lightpos[1] * planeCoff[1];
	destMat[2][1] = 0.0 - lightpos[1] * planeCoff[2];
	destMat[3][1] = 0.0 - lightpos[1] * planeCoff[3];

	destMat[0][2] = 0.0 - lightpos[2] * planeCoff[0];
	destMat[1][2] = 0.0 - lightpos[2] * planeCoff[1];
	destMat[2][2] = dot - lightpos[2] * planeCoff[2];
	destMat[3][2] = 0.0 - lightpos[2] * planeCoff[3];

	destMat[0][3] = 0.0 - lightpos[3] * planeCoff[0];
	destMat[1][3] = 0.0 - lightpos[3] * planeCoff[1];
	destMat[2][3] = 0.0 - lightpos[3] * planeCoff[2];
	destMat[3][3] = dot - lightpos[3] * planeCoff[3];

}

void FoldingNet::Draw3DModel(bool ColorOrNot)
{
	if (pThis == NULL) return;

	//显示3D效果

	//由于gluTessBeginContour循环内不能有局部变量，因此需要在外赋值

	GLdouble ***v_data = new GLdouble**[pThis->_PolygonList.size()];
	for (int i = 0; i < pThis->_PolygonList.size(); i++)
	{
		v_data[i] = new GLdouble *[pThis->_PolygonList[i].NumberofLines()];
		for (int j = 0; j < pThis->_PolygonList[i].NumberofLines(); j++)
			v_data[i][j] = new GLdouble[3];
	}

	for (int i = 0; i < pThis->_PolygonList.size(); i++)
		for (int j = 0; j < pThis->_PolygonList[i].NumberofLines(); j++)
			for (int k = 0; k < 3; k++)
				v_data[i][j][k] = 0;



	for (int j = 0; j < pThis->_PolygonList.size(); j++) // pThis->_PolygonList.size()
	{
		for (int i = 0; i < pThis->_PolygonList[j].NumberofLines(); i++) //pThis->_PolygonList[j].NumberofLines()
		{
			v_data[j][i][0] = GLdouble(pThis->_PolygonList[j].IthLine(i).GetV1().GetX() / 40);
			v_data[j][i][1] = GLdouble(pThis->_PolygonList[j].IthLine(i).GetV1().GetY() / 40);
			v_data[j][i][2] = GLdouble(pThis->_PolygonList[j].IthLine(i).GetV1().GetZ() / 40);
		}

	}

	glClear(GL_STENCIL_BUFFER_BIT);
	GLUtesselator* tess = tesser();
	if (!tess) return;

	//int j=7;
	for (int j = 0; j < pThis->_PolygonList.size(); j++) // pThis->_PolygonList.size()
		if (pThis->_Graph.IthNode(j).GetIsVisited() == 1)   //只显示了最大子图的部分！++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			if (ColorOrNot == true)
			{
				//颜色需要与floodfill处的保持一致
				float color_b = float(abs(255 - pThis->_PolygonList[j].GetPlaneNumber())) / 255;
				float color_g = float(abs(127 - pThis->_PolygonList[j].GetPlaneNumber())) / 255;
				float color_r = float(abs(0 - pThis->_PolygonList[j].GetPlaneNumber())) / 255;
				glColor3f(color_r, color_g, color_b);
				//glColor3f(0, 0, 1);
			}
			else
				glColor3f(0.5, 0.5, 0.5);
				
		
			if (pThis->_PolygonList[j].GetLoopNo() == 1)
			{
				gluTessBeginPolygon(tess, NULL);
				gluTessBeginContour(tess);

				for (int i = 0; i < pThis->_PolygonList[j].NumberofLines(); i++) //pThis->_PolygonList[j].NumberofLines()
				{
					gluTessVertex(tess, v_data[j][i], v_data[j][i]);

				}
				gluTessEndContour(tess);
				gluTessEndPolygon(tess);
			}
			else if (pThis->_PolygonList[j].GetLoopNo() > 1)
			{
				//多个loop的赋值
				vector<int> t_endloop;
				for (int i = 0; i < pThis->_PolygonList[j].NumberofLines(); i++)
				{
					if (pThis->_PolygonList[j].IthLine(i).GetP2() == pThis->_PolygonList[j].IthLine(0).GetP1())
					{
						t_endloop.push_back(i);
						break;
					}
				}
				while (t_endloop.size() < pThis->_PolygonList[j].GetLoopNo())
				{
					int t_start = t_endloop[t_endloop.size() - 1] + 1;
					for (int i = t_start; i < pThis->_PolygonList[j].NumberofLines(); i++)
					{
						if (pThis->_PolygonList[j].IthLine(i).GetP2() == pThis->_PolygonList[j].IthLine(t_start).GetP1())
						{
							t_endloop.push_back(i);
							break;
						}
					}
				}

				int t_gluNo = 0;
				gluTessBeginPolygon(tess, NULL);

				gluTessBeginContour(tess);

				for (int i = 0; i <= t_endloop[t_gluNo]; i++) //pThis->_PolygonList[j].NumberofLines()
					gluTessVertex(tess, v_data[j][i], v_data[j][i]);
				gluTessEndContour(tess);
				t_gluNo++;

				while (t_gluNo < pThis->_PolygonList[j].GetLoopNo())
				{
					gluTessBeginContour(tess);
					for (int i = t_endloop[t_gluNo - 1] + 1; i <= t_endloop[t_gluNo]; i++) //pThis->_PolygonList[j].NumberofLines()
						gluTessVertex(tess, v_data[j][i], v_data[j][i]);
					gluTessEndContour(tess);
					t_gluNo++;
				}
				gluTessEndPolygon(tess);
			}

		}

	for (int i = 0; i < pThis->_PolygonList.size(); i++)
	{
		for (int j = 0; j < pThis->_PolygonList[i].NumberofLines(); j++)
			delete[]v_data[i][j];
		delete[]v_data[i];
	}
	delete[]v_data;
}



void FoldingNet::Show3DModel()
{
	if (pThis == NULL) return;

	glutKeyboardFunc(KeyFunc);   // 键盘按键
	glutMouseFunc(MouseFunc);   // 鼠标按键
	glutMotionFunc(MouseMotion); //鼠标移动

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 200);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	LookAt();
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
	//						   //glDepthFunc(GL_LEQUAL);    // Set the type of depth-test
	//						   //glShadeModel(GL_SMOOTH);   // Enable smooth shading
	//
	//						   //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Nice perspective correction
	
    //shading

	
	

	//绘制地板
	/*glColor3f(1.0f, 0.95f, 0.9f); 
	glNormal3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_QUADS);
	glVertex3f(-30.0f, 30.0f, -0.2f);
	glVertex3f(30.0f, 30.0f, -0.2f);
	glVertex3f(30.0f, -30.0f, -0.2f);
	glVertex3f(-30.0f, -30.0f, -0.2f);
	glEnd();*/

	/////save the matrix state and do the rotations
	//glPushMatrix();

	Draw3DModel(true);

	/////restore original matrix state
	//glPopMatrix();

	/////Get ready to draw the shadow and the ground
	/////first disable lighting and save the projection state
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_LIGHTING);
	//glPushMatrix();

	/////mutiply by shadow projection matrix
	//glMultMatrixf((GLfloat *)shadowMat);

	////generate the shadow map
	////glEnable(GL_LIGHTING);
	//////明亮的白光
	////GLfloat ambientLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//////开启光照
	////glEnable(GL_LIGHTING);
	//////设置全局环境光为白光
	////glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

	////GLfloat light_position[] = { 50.0,50.0,50.0,0.0 };
	//GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	//GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	//GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

	//glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);


	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	//glDepthFunc(GL_LESS);
	//glEnable(GL_DEPTH_TEST);

	////开启颜色追踪
	//glEnable(GL_COLOR_MATERIAL);
	////设置颜色追踪的材料属性以及多边形的面
	//glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	

	//绘制三维坐标系
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(1000.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1000.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1000.0f);
	glEnd();

	//加边框线
	/*glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(10);
	for(int i = 0;i < pThis->_PolygonList.size(); i++)
		for (int j = 0; j < pThis->_PolygonList[i].NumberofLines(); j++)
		{
			LineSegment t_line = pThis->_PolygonList[i].IthLine(j);
			Vertex v1 = t_line.GetV1();
			Vertex v2 = t_line.GetV2();

			glVertex3f(v1.GetX(), v1.GetY(), v1.GetZ());
			glVertex3f(v2.GetX(), v2.GetY(), v2.GetZ());
		}

	glEnd();*/

	//Draw3DModel(false);

	///restore the projection to normal
	/*glPopMatrix();

	///draw light source
	glPushMatrix();
	glTranslatef(light_position[0], light_position[1], light_position[2]);
	glColor3ub(255, 255, 0);
	glutSolidSphere(5.0, 10, 10);
	glPopMatrix();

	///restore lighting state varibles
	glEnable(GL_DEPTH_TEST);*/
	

	glutSwapBuffers();

}

//void FoldingNet::Test()
//{
//	if (pThis == NULL) return;
//
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	gluPerspective(45, 1, 1, 40);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	gluLookAt(20, 20, 20, 0, 0, 0, 0, 0, 1);
//	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
//	glClear(GL_COLOR_BUFFER_BIT);
//
//	// 绘制三维坐标系
//	glBegin(GL_LINES);
//	glColor3f(1.0f, 0.0f, 0.0f);
//	glVertex3f(0.0f, 0.0f, 0.0f);
//	glVertex3f(1000.0f, 0.0f, 0.0f);
//
//	glColor3f(0.0f, 1.0f, 0.0f);
//	glVertex3f(0.0f, 0.0f, 0.0f);
//	glVertex3f(0.0f, 1000.0f, 0.0f);
//
//	glColor3f(0.0f, 0.0f, 1.0f);
//	glVertex3f(0.0f, 0.0f, 0.0f);
//	glVertex3f(0.0f, 0.0f, 1000.0f);
//	glEnd();
//
//
//	LineSegment l1,l2,l3;
//	l1.SetV1(201, 0, 0);
//	l1.SetV2(229, 0, 0);
//
//	l2.SetV1(53, 0, 0);
//	l2.SetV2(53, 238, 0);
//
//	l3.SetV1(201, 0, 0);
//	l3.SetV2(201, 238, 0);
//
//	pThis->_LineList.push_back(l1);
//	pThis->_LineList.push_back(l2);
//	pThis->_LineList.push_back(l3);
//
//	GraphEdge edge;
//	edge.SetId(2);
//
//	float rtmatrix[4][4];
//	pThis->RotateArbitraryAxis(90, edge, rtmatrix);
//
//	l1.SetV1(pThis->MatrixMulVertex(rtmatrix, l1.GetV1()));
//	l1.SetV2(pThis->MatrixMulVertex(rtmatrix, l1.GetV2()));
//
//	/*edge.SetId(3);
//	pThis->RotateArbitraryAxis(90, edge, rtmatrix);
//	l1.SetV1(pThis->MatrixMulVertex(rtmatrix, l1.GetV1()));
//	l1.SetV2(pThis->MatrixMulVertex(rtmatrix, l1.GetV2()));*/
//
//	glBegin(GL_LINES);
//
//	glVertex3f(l1.GetV1().GetX() /40, l1.GetV1().GetY() / 40, l1.GetV1().GetZ() / 40);
//	glVertex3f(l1.GetV2().GetX() /40, l1.GetV2().GetY() / 40, l1.GetV2().GetZ() / 40);
//
//	glEnd();
//	glFlush();
//}

void FoldingNet::CalEyePosition()
{
	if (yrotate > g_parameters.MyPi / 2.2) 
		yrotate = g_parameters.MyPi / 2.2;   /// 限制看得方向

	if (yrotate < 0.01)  
		yrotate = 0.01;

	if (xrotate > 2 * g_parameters.MyPi)  
		xrotate = 0.01;

	if (xrotate < 0)  
		xrotate = 2 * g_parameters.MyPi;

	/*if (celength > 50)  
		celength = 50;     ///  缩放距离限制

	if (celength < 5)   
		celength = 5;*/

	/// 下面利用球坐标系计算 eye 的位置，
	eye[0] = center[0] + celength * sin(yrotate) * cos(xrotate);
	eye[1] = center[1] + celength * sin(yrotate) * sin(xrotate);
	eye[2] = center[2] + celength * cos(yrotate);
}

/// center 点沿视线方向水平向后移动
void FoldingNet::MoveBackward()
{
	center[0] -= mSpeed * cos(xrotate);
	center[1] -= mSpeed * sin(xrotate);
	CalEyePosition();
}

void FoldingNet::MoveForward()
{
	center[0] += mSpeed * cos(xrotate);
	center[1] += mSpeed * sin(xrotate);
	CalEyePosition();
}

void FoldingNet::MoveLeft()
{
	center[0] += mSpeed * cos(yrotate);
	center[1] += mSpeed * sin(yrotate);
	CalEyePosition();
}

void FoldingNet::MoveRight()
{
	center[0] -= mSpeed * cos(yrotate);
	center[1] -= mSpeed * sin(yrotate);
	CalEyePosition();
}

void FoldingNet::RotateLeft()
{
	xrotate -= rSpeed;
	CalEyePosition();
}

void FoldingNet::RotateRight()
{
	xrotate += rSpeed;
	CalEyePosition();
}

void FoldingNet::RotateUp()
{
	yrotate += rSpeed;
	CalEyePosition();
}

void FoldingNet::RotateDown()
{
	yrotate -= rSpeed;
	CalEyePosition();
}

void FoldingNet::KeyFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'a': MoveLeft(); break;
	case 'd': MoveRight(); break;
	case 'w': MoveForward(); break;
	case 's': MoveBackward(); break;
	case 'z': RotateLeft(); break;
	case 'c': RotateRight(); break;
	case 'q': RotateUp(); break;
	case 'e': RotateDown(); break;
	}
	glutPostRedisplay();
}

void FoldingNet::MouseFunc(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_RIGHT_BUTTON) mouserdown = GL_TRUE;
		if (button == GLUT_LEFT_BUTTON) mouseldown = GL_TRUE;
		if (button == GLUT_MIDDLE_BUTTON)mousemdown = GL_TRUE;
	}
	else
	{
		if (button == GLUT_RIGHT_BUTTON) mouserdown = GL_FALSE;
		if (button == GLUT_LEFT_BUTTON) mouseldown = GL_FALSE;
		if (button == GLUT_MIDDLE_BUTTON)mousemdown = GL_FALSE;
	}
	mousex = x, mousey = y;
}

void FoldingNet::MouseMotion(int x, int y)
{
	if (mouserdown == GL_TRUE)
	{       /// 所除以的数字是调整旋转速度的，随便设置，达到自己想要速度即可
		xrotate += (x - mousex) / 80.0f;
		yrotate -= (y - mousey) / 120.0f;
	}

	if (mouseldown == GL_TRUE)
	{
		celength += (y - mousey) / 25.0f;
	}
	mousex = x, mousey = y;
	CalEyePosition();
	glutPostRedisplay();
}

void FoldingNet::LookAt()
{
	CalEyePosition();
	gluLookAt(eye[0], eye[1], eye[2],center[0], center[1], center[2], 0, 0, 1);
}

//opencv滑窗的鼠标响应
void FoldingNet::MouseClick(int event, int x, int y, int flags, void* param)
{
	if (needScroll)
	{
		switch (event)
		{
		case CV_EVENT_LBUTTONDOWN:
			mx = x, my = y;
			dx = 0, dy = 0;
			// 按下左键时光标定位在水平滚动条区域内  
			if (x >= rect_bar_horiz.x && x <= rect_bar_horiz.x + rect_bar_horiz.width
				&& y >= rect_bar_horiz.y && y <= rect_bar_horiz.y + rect_bar_horiz.height)
			{
				clickHorizBar = true;
			}
			// 按下左键时光标定位在垂直滚动条区域内  
			if (x >= rect_bar_verti.x && x <= rect_bar_verti.x + rect_bar_verti.width
				&& y >= rect_bar_verti.y && y <= rect_bar_verti.y + rect_bar_verti.height)
			{
				clickVertiBar = true;
			}
			break;
		case CV_EVENT_MOUSEMOVE:
			if (clickHorizBar)
			{
				dx = fabs(x - mx) > 1 ? (int)(x - mx) : 0;
				dy = 0;
			}
			if (clickVertiBar)
			{
				dx = 0;
				dy = fabs(y - my) > 1 ? (int)(y - my) : 0;
			}
			mx = x, my = y;
			break;
		case CV_EVENT_LBUTTONUP:
			mx = x, my = y;
			dx = 0, dy = 0;
			clickHorizBar = false;
			clickVertiBar = false;
			break;
		default:
			dx = 0, dy = 0;
			break;
		}
	}
}

//opencv滑窗
void FoldingNet::myShowImageScroll(char * title, IplImage * src_img, int winWidth, int winHeight)
{
	IplImage* dst_img;
	CvRect rect_dst, // 窗口中有效的图像显示区域
		rect_src; // 窗口图像对应于源图像中的区域
	int imgWidth = src_img->width,
		imgHeight = src_img->height,
		barWidth = 25; // 滚动条的宽度（像素）
	double scale_w = (double)imgWidth / (double)winWidth, // 源图像与窗口的宽度比值
		scale_h = (double)imgHeight / (double)winHeight; // 源图像与窗口的高度比值

	if (scale_w < 1)
		winWidth = imgWidth + barWidth;
	if (scale_h < 1)
		winHeight = imgHeight + barWidth;

	int showWidth = winWidth, showHeight = winHeight; // rect_dst 的宽和高

	int horizBar_width = 0, horizBar_height = 0,
		vertiBar_width = 0, vertiBar_height = 0;

	needScroll = scale_w > 1.0 || scale_h > 1.0 ? 1 : 0;
	// 若图像大于设定的窗口大小，则显示滚动条
	if (needScroll)
	{
		dst_img = cvCreateImage(cvSize(winWidth, winHeight), src_img->depth, src_img->nChannels);
		cvZero(dst_img);
		// 源图像宽度大于窗口宽度，则显示水平滚动条
		if (scale_w > 1.0)
		{
			showHeight = winHeight - barWidth;
			horizBar_width = (int)((double)winWidth / scale_w);
			horizBar_height = winHeight - showHeight;
			horizBar_x = min(
				max(0, horizBar_x + dx),
				winWidth - horizBar_width);
			rect_bar_horiz = cvRect(
				horizBar_x,
				showHeight + 1,
				horizBar_width,
				horizBar_height);
			// 显示水平滚动条
			cvRectangleR(dst_img, rect_bar_horiz, cvScalarAll(255), -1);
		}
		// 源图像高度大于窗口高度，则显示垂直滚动条
		if (scale_h > 1.0)
		{
			showWidth = winWidth - barWidth;
			vertiBar_width = winWidth - showWidth;
			vertiBar_height = (int)((double)winHeight / scale_h);
			vertiBar_y = min(
				max(0, vertiBar_y + dy),
				winHeight - vertiBar_height);
			rect_bar_verti = cvRect(
				showWidth + 1,
				vertiBar_y,
				vertiBar_width,
				vertiBar_height);
			// 显示垂直滚动条
			cvRectangleR(dst_img, rect_bar_verti, cvScalarAll(255), -1);
		}

		showWidth = min(showWidth, imgWidth);
		showHeight = min(showHeight, imgHeight);
		// 设置窗口显示区的 ROI
		rect_dst = cvRect(0, 0, showWidth, showHeight);
		cvSetImageROI(dst_img, rect_dst);
		// 设置源图像的 ROI
		src_x = (int)((double)horizBar_x*scale_w);
		src_y = (int)((double)vertiBar_y*scale_h);
		src_x = min(src_x, imgWidth - showWidth);
		src_y = min(src_y, imgHeight - showHeight);
		rect_src = cvRect(src_x, src_y, showWidth, showHeight);
		cvSetImageROI(src_img, rect_src);
		// 将源图像内容复制到窗口显示区
		cvCopy(src_img, dst_img);

		cvResetImageROI(dst_img);
		cvResetImageROI(src_img);
		// 显示图像和滚动条
		cvShowImage(title, dst_img);

		cvReleaseImage(&dst_img);
	}
	// 源图像小于设定窗口，则直接显示图像，无滚动条
	else
	{
		cvShowImage(title, src_img);
	}
}

void FoldingNet::save_mesh(const std::string& filepath)
{
    ;
}
