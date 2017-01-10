#include "LineSegment.h"
#include <math.h>


LineSegment::LineSegment()
{
}

LineSegment::LineSegment(Vertex v1, Vertex v2)
{
	_V1 = v1;
	_V2 = v2;
}


LineSegment::~LineSegment()
{
}

Vertex LineSegment::GetV1()
{
	return _V1;
}

void LineSegment::SetV1(Vertex v1)
{
	_V1 = v1;
}

void LineSegment::SetV1(float v_x, float v_y, float v_z)
{
	_V1.SetX(v_x);
	_V1.SetY(v_y);
	_V1.SetZ(v_z);
}

void LineSegment::SetPinV1(Point p)
{
	_V1.SetCorrespondingPoint(p);
}

void LineSegment::SetPinV1(float x, float y)
{
	_V1.SetCorrespondingPoint(x, y);
}

Vertex LineSegment::GetV2()
{
	return _V2;
}

void LineSegment::SetV2(Vertex v2)
{
	_V2 = v2;
}

void LineSegment::SetV2(float v_x, float v_y, float v_z)
{
	_V2.SetX(v_x);
	_V2.SetY(v_y);
	_V2.SetZ(v_z);
}

void LineSegment::SetPinV2(Point p)
{
	_V2.SetCorrespondingPoint(p);
}

void LineSegment::SetPinV2(float x, float y)
{
	_V2.SetCorrespondingPoint(x, y);
}

Point LineSegment::GetP1()
{
	return _P1;
}

void LineSegment::SetP1(Point p1)
{
	_P1 = p1;
}

void LineSegment::SetP1(float p1_x, float p1_y)
{
	_P1.SetXY(p1_x, p1_y);
}

Point LineSegment::GetP2()
{
	return _P2;
}

void LineSegment::SetP2(Point p2)
{
	_P2 = p2;
}

void LineSegment::SetP2(float p2_x, float p2_y)
{
	_P2.SetXY(p2_x, p2_y);
}


int LineSegment::GetId()
{
	return _Id;
}

int LineSegment::SetId(int id)
{
	_Id = id;
	return _Id;
}

int LineSegment::GetIsDash()
{
	return _IsDash;
}

int LineSegment::SetIsDash(int isdash)
{
	_IsDash = isdash;
	return _IsDash;
}

void LineSegment::SwapPoint()
{
	Vertex t_v;
	Point t_p;

	t_v = _V1;
	_V1 = _V2;
	_V2 = t_v;

	t_p = _P1;
	_P1 = _P2;
	_P2 = t_p;
}

int LineSegment::Floatcmp(float a, float b)
{
	if (fabs(a - b) <= g_parameters.MyEpsilon) return 0;
	if (a > b) return 1;
	else return -1;
}

float LineSegment::Dot(float x1, float y1, float x2, float y2)
{

	return x1*x2 + y1*y2;
}

float LineSegment::Cross(float x1, float y1, float x2, float y2)
{
	
	return (x1*y2 - x2*y1);
}

float LineSegment::NormalizedCross(float x1, float y1, float x2, float y2)
{
	return(x1*y2 - x2*y1) / (sqrt(x1*x1 + y1*y1)*sqrt(x2*x2 + y2*y2));
}

int LineSegment::PointOnLine(Point a, Point b, Point c)
{

	float a_x = a.GetX();
	float a_y = a.GetY();
	float b_x = b.GetX();
	float b_y = b.GetY();
	float c_x = c.GetX();
	float c_y = c.GetY();
	return Floatcmp(Dot(b_x - a_x, b_y - a_y, c_x - a_x, c_y - a_y), 0);
}

float LineSegment::ABCrossAC(Point a, Point b, Point c)
{

	float a_x = a.GetX();
	float a_y = a.GetY();
	float b_x = b.GetX();
	float b_y = b.GetY();
	float c_x = c.GetX();
	float c_y = c.GetY();
	return NormalizedCross(b_x-a_x,b_y-a_y,c_x-a_x,c_y-a_y);
}

int LineSegment::IntersectionByTwoLines(LineSegment l,Point &p)
{

	float s1, s2, s3, s4;
	int d1, d2, d3, d4;
	float a_x = _P1.GetX();
	float a_y = _P1.GetY();
	float b_x = _P2.GetX();
	float b_y = _P2.GetY();
	float c_x = l.GetP1().GetX();
	float c_y = l.GetP1().GetY();
	float d_x = l.GetP2().GetX();
	float d_y = l.GetP2().GetY();

	d1 = Floatcmp(ABCrossAC(_P1, _P2, l.GetP1()), 0);
	d2 = Floatcmp(ABCrossAC(_P1, _P2, l.GetP2()), 0);
	d3 = Floatcmp(ABCrossAC(l.GetP1(), l.GetP2(), _P1), 0);
	d4 = Floatcmp(ABCrossAC(l.GetP1(), l.GetP2(), _P2), 0);


	if ((d1^d2) == -2 && (d3^d4) == -2)
	{
		s1 = Cross(b_x - a_x, b_y - a_y, c_x - a_x, c_y - a_y);
		s2 = Cross(b_x - a_x, b_y - a_y, d_x - a_x, d_y - a_y);
		p.SetX((c_x*s2 - d_x*s1) / (s2 - s1));
		p.SetY((c_y*s2 - d_y*s1) / (s2 - s1));
		return 1;
	}


	if (d1 == 0 && PointOnLine(l.GetP1(), _P1,_P2) <= 0)
	{
		p = l.GetP1();
		return 2;
	}
	if (d2 == 0 && PointOnLine(l.GetP2(), _P1, _P2) <= 0)
	{
		p = l.GetP2();
		return 3;
	}
	if (d3 == 0 && PointOnLine(_P1, l.GetP1(), l.GetP2()) <= 0)
	{
		p = _P1;
		return 4;
	}
	if (d4 == 0 && PointOnLine(_P2, l.GetP1(), l.GetP2()) <= 0)
	{
		p = _P2;
		return 5;
	}


	return -1;
}

double LineSegment::P2DLength()
{
	double dis;
	dis = (_P1.GetX() - _P2.GetX())*(_P1.GetX() - _P2.GetX()) + (_P1.GetY() - _P2.GetY())*(_P1.GetY() - _P2.GetY());
	return sqrt(dis);
}
