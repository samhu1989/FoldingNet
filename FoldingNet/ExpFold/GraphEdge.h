#pragma once
class GraphEdge
{
public:
	GraphEdge();
	~GraphEdge();
	void SetPlaneNumber(int startPlaneNumber, int endPlaneNumber);
	void GetPlaneNumber(int &startPlaneNumber, int &endPlaneNumber);
	void SetAngle(float angle);
	int GetAngle();
	void SetWeight(float weight);
	float GetWeight();

	void SetId(int id);
	int GetId();

	void SetIsInMinSpanTree(int isintree);
	int GetIsInMinSpanTree();

private:
	int _StartPlaneNumber;
	int _EndPlaneNumber;
	float _Angle;
	float _Weight;
	int _Id;
	int _IsInMinSpanTree;
};

