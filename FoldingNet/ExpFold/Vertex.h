#pragma once
#include "Point.h"
#include "Parameters.h"
#include <math.h>

class Vertex
{
public:
	Vertex();
	~Vertex();

	Vertex(float x, float y, float z);

    Vertex(const Vertex& v){_id=v._id;_X=v._X;_Y=v._Y;_Z=v._Z;_P=v._P;}

    inline int GetId(){return _id;}
    inline int SetId(int id){_id=id;}
	float GetX();
	void SetX(float x);
	float GetY();
	void SetY(float y);
	float GetZ();
	void SetZ(float z);
	Point GetCorrespondingPoint();
	void SetCorrespondingPoint(Point p);
	void SetCorrespondingPoint(float p_x,float p_y);

	bool operator ==(const Vertex& rhs) const
	{
		if (fabs(_X-rhs._X)< g_parameters.ThresholdForSeparateLines && fabs(_Y-rhs._Y)< g_parameters.ThresholdForSeparateLines && fabs(_Z-rhs._Z)< g_parameters.ThresholdForSeparateLines)
			return true;
		else
			return false;
	}

	void SetParameters(Parameters para);

private:
    int _id;
	float _X;
	float _Y;
	float _Z;

	//每个Vertex对应于一个二维平面上的Point
	Point _P;
};

