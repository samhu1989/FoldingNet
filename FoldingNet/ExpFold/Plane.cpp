#include "Plane.h"


Plane::Plane()
{
	_LoopNo = 0;
}


Plane::~Plane()
{
}

int Plane::NumberofLines()
{
	return _Lines.size();
}

int Plane::NumberofVertices()
{
	return _Vertices.size();
}

void Plane::SetPlane(vector<LineSegment> l)
{
	_Lines = l;
}

LineSegment Plane::IthLine(int i)
{
	return _Lines[i];
}

Vertex Plane::IthVertex(int i)
{
	return _Vertices[i];
}

MESH Plane::GetMesh()
{
	return _Mesh;
}

void Plane::SetPlaneNumber(int number)
{
	_PlaneNumber = number;
}

int Plane::GetPlaneNumber()
{
	return _PlaneNumber;
}

int Plane::GetLoopNo()
{
	return _LoopNo;
}

void Plane::ChangeIthLine(int i,Vertex v1, Vertex v2)
{
	_Lines[i].SetV1(v1);
	_Lines[i].SetV2(v2);
	_Lines[i].SetP1(v1.GetCorrespondingPoint());
	_Lines[i].SetP2(v2.GetCorrespondingPoint());
}

void Plane::ChangeIthVertex(int i, Vertex v1)
{
	_Vertices[i].SetX(v1.GetX());
	_Vertices[i].SetY(v1.GetY());
	_Vertices[i].SetZ(v1.GetZ());
	_Vertices[i].SetCorrespondingPoint(v1.GetCorrespondingPoint());
}


void Plane::FindLoop()
{

	if (_Lines.size() == 0)
	{
		cout << "Plane doesn't exist! " << endl;
		return;
	}

	vector<vector<LineSegment>> t_ListForFindingLoop;

	//��_line��ÿ���߶���β�ߵ���push��_Line��
	int t_size = _Lines.size();
	for (int i = 0; i < t_size; i++)
	{
		LineSegment t_line = _Lines[i];
		t_line.SwapPoint();
		_Lines.push_back(t_line);
	}

	//Reordering,delete redundant lines,find loop
	//but���ҵ�������ͬ����Ļ�·��
	vector<LineSegment> s;
	s.push_back(_Lines[0]);
	t_ListForFindingLoop.push_back(s);
	_Lines.erase(_Lines.begin());
	s.clear();
	vector<int> adjacent_order;

	while (_Lines.size() > 0)
	{
		int left = _Lines.size();
		int t_ListNo = t_ListForFindingLoop.size();
		for (int i = 0; i < t_ListNo; i++)
		{
			//�ҵ���list�����һ���߶��ڽӵ��߶�
			for (int j = 0; j < _Lines.size(); j++)
			{
				if (isTwoLinesAdjacent(t_ListForFindingLoop[i][t_ListForFindingLoop[i].size()-1], _Lines[j]) == true)
					adjacent_order.push_back(j);
			}

			//���ڽ��߶�ֻ��һ��
			if (adjacent_order.size() == 1)
			{
				t_ListForFindingLoop[i].push_back(_Lines[adjacent_order[0]]);
				_Lines.erase(_Lines.begin() + adjacent_order[0]);			
			}

		    //���ڽ��߶β�ֹһ��
			else if (adjacent_order.size() > 1)
			{
				for (int k = 1; k < adjacent_order.size(); k++)
				{
					t_ListForFindingLoop.push_back(t_ListForFindingLoop[i]);
					t_ListForFindingLoop[t_ListForFindingLoop.size()-1].push_back(_Lines[adjacent_order[k]]);
				}
				t_ListForFindingLoop[i].push_back(_Lines[adjacent_order[0]]);

				//���ֱ���ӵ������߶�Ҳ���ڣ���Ҳ��Ҫ��ӵ�List��ȥ
				/*for (int k = 0; k < adjacent_order.size()-1; k++)
					for (int k1 = k + 1; k1 < adjacent_order.size(); k1++)
					{
						if (_Lines[adjacent_order[k]].GetP1() == _Lines[adjacent_order[k1]].GetP1())
						{
							_Lines[adjacent_order[k]].SwapPoint();
							s.push_back(_Lines[adjacent_order[k]]);
							s.push_back(_Lines[adjacent_order[k1]]);
							t_ListForFindingLoop.push_back(s);
							s.clear();
						}
					}	*/		
				
				int delete_no = 0;
				for (int k = 0; k < adjacent_order.size(); k++)
				{
					delete_no = adjacent_order[k] - k;
					_Lines.erase(_Lines.begin() + delete_no);
				}
			}

			else
				continue;

			adjacent_order.clear();
		}

		if (_Lines.size() == left)
		{
			s.push_back(_Lines[0]);
			t_ListForFindingLoop.push_back(s);
			_Lines.erase(_Lines.begin());
			s.clear();
		}
	}

	//check�Ƿ��л�·
	//checkǰ��ĺ����һ���߶��Ƿ���β����
	//��Ӧȥ����βΪͬһ���߶ε����

	for (int i = 0; i < t_ListForFindingLoop.size(); i++)
	{
		int flag = 0;
		if (flag == 0)
		{
			for (int j = 0; j < t_ListForFindingLoop[i].size() - 1; j++)
			{
				if (isLoopFrom(t_ListForFindingLoop[i]) >= 0)
				{
					int t_loopfrom = isLoopFrom(t_ListForFindingLoop[i]);
					_Lines.insert(_Lines.end(),t_ListForFindingLoop[i].begin() + t_loopfrom, t_ListForFindingLoop[i].end());
					_LoopNo++;   //��·��1
					flag = 1;
					break;
				}
				t_ListForFindingLoop[i].pop_back();
			}
		}
		
	}

	//ɾ��ͬһ���߶�
	//ɾ���ظ���·�е��߶�
	vector<LineSegment>::iterator it, it1;
	for (it = _Lines.begin(); it != _Lines.end(); it++)
	{
		for (it1 = it + 1; it1 != _Lines.end(); )
		{
			if (*it1 == *it)
			{
				if ((*it1).GetIsDash() != (*it).GetIsDash())
					(*it).SetIsDash(0);
				it1 = _Lines.erase(it1);
			}
			else
				++it1;
		}
	}
	_LoopNo = _LoopNo / 2;

	//LineSegment temp = _Lines[4];
	//_Lines.erase(_Lines.begin() + 4);
	//_Lines.push_back(temp);


	/*for (int i = 0; i < _Lines.size(); i++)
		cout << _Lines[i].GetP1().GetX() << "   " << _Lines[i].GetP1().GetY() << "   " << _Lines[i].GetP2().GetX() << "   " << _Lines[i].GetP2().GetY() << endl;*/

	if (_Lines.size() > 0)
	{

		Vertex v; //�����Ӧ����ά��
		v.SetCorrespondingPoint(_Lines[0].GetP1());
		_Vertices.push_back(v);
		for (int i = 0; i < _Lines.size() - 1; i++)
		{
			v.SetCorrespondingPoint(_Lines[i].GetP2());
			_Vertices.push_back(v);
		}
	}
	else
		cout <<_PlaneNumber<< "   No Loop !!!!!!!!!!!!!" << endl;

}



void Plane::MergeLines()
{
    //��һЩͬһ��ֱ���ϵ�short lines���кϲ�
	for (int i = 0; i < _Lines.size(); i++)
	{

	}
}

bool Plane::isTwoLinesAdjacent(LineSegment &a, LineSegment &b)
{
	Point a_s = a.GetP1();
	Point a_t = a.GetP2();
	Point b_s = b.GetP1();
	Point b_t = b.GetP2();

	if (a_t == b_s && !(a_s == b_t)) //���ڣ����Ҳ���ͬһ����
		return true;
	else
		return false;
	
}

int Plane::isLoopFrom(vector<LineSegment> a)
{
	//������Loop�����س�ʼ�ڵ�����
	//�����ǣ��򷵻�-1.
	for (int i = 0; i < a.size() - 1; i++)
	{
		if (a[i].GetP1() == a[a.size()-1].GetP2())
			return i;
	}
	return -1;
}



void Plane::Triangulation()
{
	//����lines���߶����ж�
	VERTEX2D temp_p;
	vector<VERTEX2D> EdgePointList;

	for (int i = 0; i < _Vertices.size(); i++)
	{
		temp_p.x = _Vertices[i].GetCorrespondingPoint().GetX();
		temp_p.y = _Vertices[i].GetCorrespondingPoint().GetY();
		EdgePointList.push_back(temp_p);
	}

	CreateMesh(EdgePointList, &_Mesh);

	//�ж������������Ƿ����ڰ�������⣬���ǣ���ɾ��
	//http://blog.csdn.net/hjh2005/article/details/9246967

	TRIANGLE_PTR pTri = _Mesh.pTriArr;
	bool oddNodes;
	float centroid_x, centroid_y;
	while (pTri != NULL)
	{
		oddNodes = false;
		centroid_x = (_Vertices[pTri->i1 - 3].GetCorrespondingPoint().GetX() + _Vertices[pTri->i2 - 3].GetCorrespondingPoint().GetX() + _Vertices[pTri->i3 - 3].GetCorrespondingPoint().GetX()) / 3;
		centroid_y = (_Vertices[pTri->i1 - 3].GetCorrespondingPoint().GetY() + _Vertices[pTri->i2 - 3].GetCorrespondingPoint().GetY() + _Vertices[pTri->i3 - 3].GetCorrespondingPoint().GetY()) / 3;
		for (int i = 0; i < _Lines.size(); i++)
		{
			float xi = _Lines[i].GetP1().GetX();
			float xj = _Lines[i].GetP2().GetX();

			float yi = _Lines[i].GetP1().GetY();
			float yj = _Lines[i].GetP2().GetY();

			if ((yi < centroid_y && yj >= centroid_y || yj < centroid_y && yi >= centroid_y) && (xi <= centroid_x || xj <= centroid_x))
			{
				if (xi + (centroid_y - yi) / (yj - yi)*(xj - xi) < centroid_x)
				{
					oddNodes = !oddNodes;
				}
			}
		}

		if (!oddNodes)
		{
			if (pTri == _Mesh.pTriArr)
			{
				_Mesh.pTriArr = pTri->pNext;
				if(_Mesh.pTriArr != nullptr)
				    _Mesh.pTriArr->pPrev = NULL;
				//delete pTri;
			}
			else
			{
				pTri->pPrev->pNext = pTri->pNext;
				if (pTri->pNext != NULL)
					pTri->pNext->pPrev = pTri->pPrev;
				//delete pTri;
			}
		}
		pTri = pTri->pNext;
	}

}

int Plane::Floatcmp(float a, float b)
{
	if (fabs(a - b) <= g_parameters.MyEpsilon) return 0;
	if (a > b) return 1;
	else return -1;
}

float Plane::Cross(float x1, float y1, float x2, float y2)
{
	//����жϵ��Ƿ����߶���
	return (x1*y2 - x2*y1)/(sqrt(x1*x1 + y1* y1)*sqrt(x2*x2 + y2*y2));
}

int Plane::PointOnLine(Point a, Point b, Point c)
{
	//��a���ǲ������߶�bc�ϣ�>0���ڣ�=0��˵��غϣ�<0�ڡ�
	float a_x = a.GetX();
	float a_y = a.GetY();
	float b_x = b.GetX();
	float b_y = b.GetY();
	float c_x = c.GetX();
	float c_y = c.GetY();
	return Floatcmp(Cross(b_x - a_x, b_y - a_y, c_x - a_x, c_y - a_y), 0);
}
