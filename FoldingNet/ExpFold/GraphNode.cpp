#include "GraphNode.h"



GraphNode::GraphNode()
{
	_IsVisited = false;
}


GraphNode::~GraphNode()
{
}

void GraphNode::SetPlaneNumber(int planeNumber)
{
	_PlaneNumber = planeNumber;
}

int GraphNode::GetPlaneNumber()
{
	return _PlaneNumber;
}

void GraphNode::SetIsVisited(int isvisited)
{
	_IsVisited = isvisited;
}

int GraphNode::GetIsVisited()
{
	return _IsVisited;
}

void GraphNode::AddNeighbourNumber(int planenumber)
{
	_NeighbourNumber.push_back(planenumber);
}

int GraphNode::IthNeighbourNumber(int i)
{
	return _NeighbourNumber[i];
}

int GraphNode::GetNeighbourSize()
{
	return _NeighbourNumber.size();
}

void GraphNode::ClearNeighbour()
{
	_NeighbourNumber.clear();
}

//void GraphNode::SetEdgeList(vector<GraphEdge> edges)
//{
//	_Edges = edges;
//}
//
//vector<GraphEdge> GraphNode::GetEdges()
//{
//	return _Edges;
//}
