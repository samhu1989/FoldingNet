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
	//�и��߶�
	void CutOffLines();

	//�Ҷ�ͼת����α��ɫͼ��
	cv::Mat GrayToRGB(cv::Mat img);
	cv::Mat GrayToRainbow(cv::Mat img);
	//OpenGl��ʾ
	static void Show2DPattern();
	static FoldingNet * pThis;   //��̬����ָ��

	//�߶����ػ�����ƽ��
	void BresenhamRasterization(LineSegment lineSeg, int number);  //Bresenham�������߶����ػ�
	void FloodFill4Stack(int x, int y, int planenumber);
    bool FindPolygonByFloodFill();  //floodfill�����Ҷ�����

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

	static void SetupRC(); //���û���
	static void CalcNormal(float v[3][3], float out[3]);
	static void MakeShadowMatrix(GLfloat points[3][3], GLfloat lightpos[4], GLfloat destMat[4][4]);
	static void Draw3DModel(bool ColorOrNot);
	static void Show3DModel();

	//static void Test();

	//openGL��������ת
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

	//opencv �Ļ���
	static void MouseClick(int event, int x, int y, int flags, void* param);
	static void myShowImageScroll(char* title, IplImage* src_img, int winWidth, int winHeight);

	static double mx , my ;
	static int dx , dy , horizBar_x , vertiBar_y ;
	static bool clickVertiBar , clickHorizBar , needScroll ;
	static CvRect rect_bar_horiz, rect_bar_verti;
	static int src_x , src_y ; // Դͼ���� rect_src �����Ͻ�λ��

    //save mesh
    void save_mesh(const std::string& filepath);
private:
	vector<LineSegment> _LineList;
	vector<Plane> _PolygonList;

	//�������飬һ�����߶ε����ţ�����flood�����ε�����
	//������СΪ�߶�������ȡֵ��Χ
	int _RangeofPX;
	int _RangeofPY;
	int _MinPX;
	int _MinPY;

	cv::Mat _LineLabel;    //������Ѱ�ұ߽�
	cv::Mat _PolygonLabel; //����������ͼ��


	stack<Point> _MyStack;

	Graph _Graph;

};

