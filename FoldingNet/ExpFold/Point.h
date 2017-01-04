#pragma once
#include <stdlib.h>
#include<math.h>
#include"Parameters.h"

class Point
{
public:
	Point();
	Point(float x, float y);
	~Point();

	float GetX();
	void SetX(float x);
	float GetY();
	void SetY(float y);
	void SetXY(float x, float y);

	bool operator ==(const Point& rhs) const
	{
		if ((fabs(_X - rhs._X) < g_parameters.ThresholdForSeparateLines) && (fabs(_Y - rhs._Y) < g_parameters.ThresholdForSeparateLines) )  //绝对值要加啊亲。。。。。
			return true;
		else
			return false;
	}

	void SetParameters(Parameters para);

private:
	float _X;
	float _Y;
};

