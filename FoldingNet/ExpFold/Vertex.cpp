#include "Vertex.h"



Vertex::Vertex()
{
	_X = 0;
	_Y = 0;
	_Z = 0;
}


Vertex::~Vertex()
{
}

Vertex::Vertex(float x, float y, float z)
{
	_X = x;
	_Y = y;
	_Z = z;
}

float Vertex::GetX()
{
	return _X;
}

void Vertex::SetX(float x)
{
	_X = x;
}

float Vertex::GetY()
{
	return _Y;
}

void Vertex::SetY(float y)
{
	_Y = y;
}

float Vertex::GetZ()
{
	return _Z;
}

void Vertex::SetZ(float z)
{
	_Z = z;
}

Point Vertex::GetCorrespondingPoint()
{
	return _P;
}

void Vertex::SetCorrespondingPoint(Point p)
{
	_P = p;
}

void Vertex::SetCorrespondingPoint(float p_x, float p_y)
{
	_P.SetXY(p_x, p_y);
}

void Vertex::SetParameters(Parameters para)
{
	g_parameters = para;
}
