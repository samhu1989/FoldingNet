#pragma once
#include "Point.h"
#include "Vertex.h"
#include <iostream>


using namespace std;
class LineSegment
{
public:
	LineSegment();
    LineSegment(const LineSegment&);
	LineSegment(Vertex v1, Vertex v2);
	~LineSegment();

    inline int GetLoopNum(){return _loop_num;}
    inline void SetLoopNum(int num){_loop_num=num;}

    std::vector<int> connected_planes_;
    std::vector<int> connected_to_main_;
    std::vector<int> idx_p1_;
    std::vector<int> idx_p2_;

	Vertex GetV1();
	void SetV1(Vertex v1);
	void SetV1(float v_x,float v_y,float v_z);
	void SetPinV1(Point p);
	void SetPinV1(float x, float y);

	Vertex GetV2();
	void SetV2(Vertex v2);
	void SetV2(float v_x, float v_y, float v_z);
	void SetPinV2(Point p);
	void SetPinV2(float x, float y);

	Point GetP1();
	void SetP1(Point p1);
	void SetP1(float p1_x,float p1_y);

	Point GetP2();
	void SetP2(Point p2);
	void SetP2(float p2_x, float p2_y);

	int GetId();
	int SetId(int id);

	int GetIsDash();
	int SetIsDash(int isdash);

	void SwapPoint();

	int Floatcmp(float a,float b);
	float Dot(float x1, float y1, float x2, float y2);
	float Cross(float x1, float y1, float x2, float y2);
	float NormalizedCross(float x1, float y1, float x2, float y2);
	int PointOnLine(Point a,Point b,Point c);
	float ABCrossAC(Point a, Point b, Point c);
	int IntersectionByTwoLines(LineSegment l,Point &p);


	//判断两个线段是否为同一条
	bool operator ==(const LineSegment& rhs) const
	{
		if ((_V1 == rhs._V1 && _V2 == rhs._V2 && _P1 == rhs._P1 && _P2 == rhs._P2) || (_V1 == rhs._V2 && _V2 == rhs._V1 &&  _P1 == rhs._P2 && _P2 == rhs._P1))
			return true;
		else
			return false;
	}

	double P2DLength();

private:
	Vertex _V1;
	Vertex _V2;
	Point _P1;
	Point _P2;
	int _Id;
	int _IsDash;
    int _loop_num;
};

