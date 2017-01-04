#pragma once
#include <stack>
#include "Plane.h"
#include "opencv2/opencv.hpp"
#include "Graph.h"
#include "GL/freeglut.h"
#include <stdlib.h>

using namespace std;

class FoldingNet
{
public:
	FoldingNet();
	~FoldingNet();

	void LoadParameters();
	void ReadLinesFromTxt();
	//切割线段
	void CutOffLines();

	//灰度图转化成伪彩色图像
	cv::Mat GrayToRGB(cv::Mat img);
	cv::Mat GrayToRainbow(cv::Mat img);
	//OpenGl显示
	static void Show2DPattern();
	static FoldingNet * pThis;   //静态对象指针

	//线段像素化，找平面
	void BresenhamRasterization(LineSegment lineSeg, int number);  //Bresenham方法的线段像素化
	void FloodFill4Stack(int x, int y, int planenumber);
	void FindPolygonByFloodFill();  //floodfill方法找多边形

	//3D modeling
	void Modeling();
	void RotateArbitraryAxis(float angle, GraphEdge edge, float rtmatrix[4][4]);
	Vertex MatrixMulVertex(float **rtmatrix, Vertex p);

	static void CALLBACK PolyLine3DBegin(GLenum type);
	static void CALLBACK PolyLine3DVertex(GLdouble * vertex);
	static void CALLBACK PolyLine3DEnd();
	static GLUtesselator* tesser();
	static void OnReshape(int w, int h);
	static void ShowPlane();

	static void ShowTriangulation();

	static GLfloat light_position[4];
	static GLfloat shadowMat[4][4];

	static void SetupRC(); //设置环境
	static void CalcNormal(float v[3][3], float out[3]);
	static void MakeShadowMatrix(GLfloat points[3][3], GLfloat lightpos[4], GLfloat destMat[4][4]);
	static void Draw3DModel(bool ColorOrNot);
	static void Show3DModel();

	//static void Test();

	//openGL的缩放旋转
	/// record the state of mouse
	static GLboolean mouserdown ;
	static GLboolean mouseldown;
	static GLboolean mousemdown ;

	/// when a mouse-key is pressed, record current mouse position 
	static GLint mousex , mousey ;

	static GLfloat center[3]; /// center position
	static GLfloat eye[3]; /// eye's position

	static GLfloat yrotate ; /// angle between y-axis and look direction
	static GLfloat xrotate; /// angle between x-axis and look direction
	static GLfloat celength;/// lenght between center and eye

	static GLfloat mSpeed ; /// center move speed
	static GLfloat rSpeed ; /// rotate speed
	static GLfloat lSpeed ; /// reserved

	static void CalEyePosition();
	static void MoveBackward();
	static void MoveForward();
	static void MoveLeft();
	static void MoveRight();
	static void RotateLeft();
	static void RotateRight();
	static void RotateUp();
	static void RotateDown();
	static void KeyFunc(unsigned char key, int x, int y);
	static void MouseFunc(int button, int state, int x, int y);
	static void MouseMotion(int x, int y);
	static void LookAt();

	//opencv 的滑窗
	static void MouseClick(int event, int x, int y, int flags, void* param);
	static void myShowImageScroll(char* title, IplImage* src_img, int winWidth, int winHeight);

	static double mx , my ;
	static int dx , dy , horizBar_x , vertiBar_y ;
	static bool clickVertiBar , clickHorizBar , needScroll ;
	static CvRect rect_bar_horiz, rect_bar_verti;
	static int src_x , src_y ; // 源图像中 rect_src 的左上角位置

    //save mesh
    void save_mesh(const std::string& filepath);
private:
	vector<LineSegment> _LineList;
	vector<Plane> _PolygonList;

	//两个数组，一个填线段的序号，个是flood多边形的序号
	//数组大小为线段坐标的取值范围
	int _RangeofPX;
	int _RangeofPY;
	int _MinPX;
	int _MinPY;

	cv::Mat _LineLabel;    //利用其寻找边界
	cv::Mat _PolygonLabel; //利用其填充图像


	stack<Point> _MyStack;

	Graph _Graph;

};

