#pragma once
#include "LineSegment.h"
#include <vector>
#include "delaunay.h"


class Plane
{
public:
	Plane();
	~Plane();
	int NumberofLines();
	int NumberofVertices();

	void SetPlane(vector<LineSegment> l);

	LineSegment IthLine(int i);
	Vertex IthVertex(int i);
	MESH GetMesh();

	void SetPlaneNumber(int number);
	int GetPlaneNumber();

	int GetLoopNo();

	void ChangeIthLine(int i,Vertex v1, Vertex v2);
	void ChangeIthVertex(int i, Vertex v1);

	void FindLoop();

	void MergeLines();
	bool isTwoLinesAdjacent(LineSegment &a, LineSegment &b);
	int isLoopFrom(vector<LineSegment> a);

	//三角化
	void Triangulation();

	int Floatcmp(float a, float b);
	float Cross(float x1, float y1, float x2, float y2);
	int PointOnLine(Point a, Point b, Point c);

private:
	vector<LineSegment> _Lines;
	vector<Vertex> _Vertices;  //平面上的顶点
	MESH _Mesh;
	int _PlaneNumber;
	int _LoopNo;
};

