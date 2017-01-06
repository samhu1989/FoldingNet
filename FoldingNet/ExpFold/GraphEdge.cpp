#include "GraphEdge.h"
#include <math.h>

GraphEdge::GraphEdge()
{
	_StartPlaneNumber = 0;
	_EndPlaneNumber = 0;
	_Angle = 0;
	_IsInMinSpanTree = 0;
}


GraphEdge::~GraphEdge()
{
}

void GraphEdge::SetPlaneNumber(int startPlaneNumber, int endPlaneNumber)
{
	_StartPlaneNumber = startPlaneNumber;
	_EndPlaneNumber = endPlaneNumber;
}

void GraphEdge::GetPlaneNumber(int &startPlaneNumber, int &endPlaneNumber)
{
	startPlaneNumber = _StartPlaneNumber;
	endPlaneNumber = _EndPlaneNumber;
}

void GraphEdge::SetAngle(float angle)
{
	_Angle = angle;
	//顺便定义一下weight
	_Weight = cos(_Angle);
}

int GraphEdge::GetAngle()
{
	return _Angle;
}

void GraphEdge::SetWeight(float weight)
{
	_Weight = weight;
}

float GraphEdge::GetWeight()
{
	return _Weight;
}

void GraphEdge::SetId(int id)
{
	_Id = id;
}

int GraphEdge::GetId()
{
	return _Id;
}

void GraphEdge::SetIsInMinSpanTree(int isintree)
{
	_IsInMinSpanTree = isintree;
}

int GraphEdge::GetIsInMinSpanTree()
{
	return _IsInMinSpanTree;
}
