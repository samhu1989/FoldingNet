#include "Point.h"



Point::Point()
{
}

Point::Point(float x, float y)
{
	_X = x;
	_Y = y;
}

Point::~Point()
{
}

float Point::GetX() const
{
	return _X;
}

void Point::SetX(float x)
{
	_X = x;
}

float Point::GetY() const
{
	return _Y;
}

void Point::SetY(float y)
{
	_Y = y;
}

void Point::SetXY(float x, float y)
{
	_X = x;
	_Y = y;
}

void Point::SetParameters(Parameters para)
{
	g_parameters = para;
}


