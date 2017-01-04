#pragma once
#include "GraphEdge.h"
#include<vector>

using namespace std;

class GraphNode
{
public:
	GraphNode();
	~GraphNode();
	void SetPlaneNumber(int planeNumber);
	int GetPlaneNumber();
	void SetIsVisited(int isvisited);
	int GetIsVisited();
	
	void AddNeighbourNumber(int planenumber);
	int IthNeighbourNumber(int i);
	int GetNeighbourSize();
	void ClearNeighbour();

private:
	int _PlaneNumber;
	int _IsVisited;
	vector<int> _NeighbourNumber;
};

