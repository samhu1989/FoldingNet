#pragma once
#include "LineSegment.h"

class DashLine :
	public LineSegment
{
public:
	DashLine();
	~DashLine();

private:
	int _Angle;
};

