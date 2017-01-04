#include "Graph.h"
#include <iostream>

Graph::Graph()
{
}


Graph::~Graph()
{
}

void Graph::AddNode(GraphNode newNode)
{
	_NodeList.push_back(newNode);
}

void Graph::AddEdge(GraphEdge newEdge)
{
	_EdgeList.push_back(newEdge);
}

GraphNode Graph::IthNode(int i)
{
	return _NodeList[i];
}

int Graph::GetNodeSize()
{
	return _NodeList.size();
}

void Graph::SetIthNodeNeihbour(int i, int n)
{
	_NodeList[i].AddNeighbourNumber(n + 1);
}

GraphEdge Graph::IthEdge(int i)
{
	return _EdgeList[i];
}

int Graph::GetEdgeSize()
{
	return _EdgeList.size();
}

void Graph::DepthFirstSearch(int i, int ith, int &VisitedNumber)
{
	int t_neighbour = 0;
	_NodeList[i].SetIsVisited(ith);
	//cout << "DFS:  " << i+1 <<"   "<<VisitedNumber<<endl;
	VisitedNumber++;
	for (int j = 0; j < _NodeList[i].GetNeighbourSize(); j++)
	{
		t_neighbour = _NodeList[i].IthNeighbourNumber(j);		
		if ((_NodeList[t_neighbour - 1].GetIsVisited()) == 0)
			DepthFirstSearch(t_neighbour - 1,ith, VisitedNumber);
	}
}

int Graph::FindLargestSubGraph()
{
	vector<int> t_visitednumber;
	int t_visited;
	int time = 1;
	for (int i = 0; i < _NodeList.size(); i++)
	{
		t_visited = 0;
		if (_NodeList[i].GetIsVisited() == 0)
		{
			DepthFirstSearch(i, time, t_visited);
			time++;
			t_visitednumber.push_back(t_visited);
		}	
	}
	/*for (int i = 0; i < t_visitednumber.size(); i++)
		cout << "Subgraph size : "<< t_visitednumber[i] << endl;*/

	//找到最大子图对应的序号，将其他节点赋值为0
	int most_visited = 0;
	for (int i = 0; i < t_visitednumber.size(); i++)
	{
		if (most_visited < t_visitednumber[i])
			most_visited = t_visitednumber[i];
	}

	for (int i = 0; i < t_visitednumber.size(); i++)
	{
		if (most_visited == t_visitednumber[i])
			time = i + 1;
	}

	for (int i = 0; i < _NodeList.size(); i++)
	{
		if (_NodeList[i].GetIsVisited() != time)
			_NodeList[i].SetIsVisited(0);
	}

	return most_visited;
}

vector<int> Graph::MiniSpanTree_Prim()
{
	//返回依次生成的边的起始节点和终结点，方便判断旋转矩阵生成的顺序

	int *adjvex = new int[_NodeList.size()];
	float *lowcost = new float[_NodeList.size()];
	float **weight = new float *[_NodeList.size()];
	for (int i = 0; i < _NodeList.size(); i++)
		weight[i] = new float[_NodeList.size()];

	for(int i=0;i<_NodeList.size();i++)
		for (int j = 0; j < _NodeList.size(); j++)
		{
			if (i == j)
				weight[i][j] = 0;
			else
				weight[i][j] = g_parameters.MyInfinity;
		}

	for (int i = 0; i < _EdgeList.size(); i++)
	{
		int startplane, endplane;
		_EdgeList[i].GetPlaneNumber(startplane, endplane);
		if (_NodeList[startplane - 1].GetIsVisited() != 0 && _NodeList[endplane - 1].GetIsVisited() != 0)
		{
			weight[startplane - 1][endplane - 1] = _EdgeList[i].GetWeight();
			weight[endplane - 1][startplane - 1] = _EdgeList[i].GetWeight();
		}
	}

	//输出weight列表
	/*for (int i = 0; i < _NodeList.size(); i++)
	{
		for (int j = 0; j < _NodeList.size(); j++)
			cout << weight[i][j] << "  ";
		cout << endl;
	}*/

	lowcost[0] = 0;
	adjvex[0] = 0;


	for (int i = 1; i < _NodeList.size(); i++)
	{
		lowcost[i] = weight[0][i];
		adjvex[i] = 0;
	}

	float min_cost;
	vector<int> EdgeOrder;
	for (int i = 0; i < _NodeList.size(); i++)
	{
		min_cost = g_parameters.MyInfinity;
		
		int j = 0;
		int k = 0;

		if (_NodeList[i].GetIsVisited() != 0)
		{


			while (j < _NodeList.size())
			{
				if (lowcost[j] != 0 && lowcost[j] < min_cost)
				{
					min_cost = lowcost[j];
					k = j;
				}	
				j++;
			}

			//设置相应的边使得在最小生成树内
			for (int m = 0; m < _EdgeList.size(); m++)
			{
				int startplane, endplane;
				_EdgeList[m].GetPlaneNumber(startplane, endplane);
				//cout <<startplane<<"   "<<endplane<<"    "<< adjvex[k] + 1 << "   " << k + 1 << endl;
				if ((startplane == adjvex[k] + 1 && endplane == k + 1) || (startplane == k + 1 && endplane == adjvex[k] + 1))
				{
					//cout << adjvex[k] + 1 << "   " << k + 1 << endl;
					_EdgeList[m].SetIsInMinSpanTree(1);
					EdgeOrder.push_back(adjvex[k] + 1);
					EdgeOrder.push_back(k + 1);
					break;
				}
					
			}

			lowcost[k] = 0;
			for (j = 0; j < _NodeList.size(); j++)
			{
				if (_NodeList[i].GetIsVisited() != 0)
				{
					if (lowcost[j] != 0 && weight[k][j] < lowcost[j])
					{
						lowcost[j] = weight[k][j];
						adjvex[j] = k;
					}
				}
			}
		}
	}

	//显示最小生成树的边
	/*cout << "Edge in Minimum Spaning Tree：" << endl;
	for (int i = 0; i <_EdgeList.size(); i++)
	{
		int startplane, endplane;
		if (_EdgeList[i].GetIsInMinSpanTree() == 1)
		{
			_EdgeList[i].GetPlaneNumber(startplane, endplane);
			cout << startplane << "    " << endplane << endl;
		}
	}*/

	//若两个面有n条边相连，只保留其中一条边
	vector<GraphEdge>::iterator it, it1;
	for( it = _EdgeList.begin() ; it != _EdgeList.end(); it++)
		for (it1 = it + 1; it1 != _EdgeList.end();it1++)
		{
			if ((*it).GetIsInMinSpanTree() == 1 && (*it1).GetIsInMinSpanTree() == 1)
			{
				int s1, s2, t1, t2;
				(*it).GetPlaneNumber(s1, t1);
				(*it1).GetPlaneNumber(s2, t2);
				if (s1 == s2 && t1 == t2)
					(*it1).SetIsInMinSpanTree(0);
			}
			
		}


	delete adjvex;
	delete lowcost;
	for (int i = 0; i < _NodeList.size(); i++)
		delete []weight[i];
	delete[]weight;

	return EdgeOrder;
}
