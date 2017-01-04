#pragma once
#include "GraphEdge.h"
#include "GraphNode.h"
#include "Parameters.h"

class Graph
{
public:
	Graph();
	~Graph();
	void AddNode(GraphNode newNode);
	void AddEdge(GraphEdge newEdge);

	GraphNode IthNode(int i);
	int GetNodeSize();
	void SetIthNodeNeihbour(int i, int n);

	GraphEdge IthEdge(int i);
	int GetEdgeSize();

	void DepthFirstSearch(int i,int ith,int &VisitedNumber);
	int FindLargestSubGraph();
	vector<int> MiniSpanTree_Prim();


private:
	vector<GraphNode> _NodeList;
	vector<GraphEdge> _EdgeList;
};

