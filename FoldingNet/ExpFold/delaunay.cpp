#include "delaunay.h"
#include <cstdlib>
#include <iostream>
#include <cmath>


void CreateMesh(vector<VERTEX2D> vertex, MESH_PTR pMesh)
{   
	int amount = vertex.size();
	int num_triangle = 0;
	InitMesh(pMesh, amount);

//	VERTEX2D* create_point = new VERTEX2D[amount];
	for(int i=0; i<vertex.size(); i++)
	{
		(pMesh->pVerArr+i+3)->x = vertex[i].x;
		(pMesh->pVerArr+i+3)->y = vertex[i].y;
//		create_point[i].x = poly[i].x;
//		create_point[i].y = poly[i].y;
	}

	IncrementalDelaunay(pMesh);

	TRIANGLE_PTR pTri = pMesh->pTriArr;
	while( pTri != NULL )
	{
		num_triangle++;
		pTri = pTri->pNext;
	}

	pMesh->triangle_num = num_triangle;
}

void IncrementalDelaunay(MESH_PTR pMesh)
{
	// Add a appropriate triangle boudingbox to contain V
	AddBoundingBox(pMesh);

	// Get a vertex/point vi from V and Insert(vi)
	for (int i=3; i<pMesh->vertex_num+3; i++)
	{
		Insert(pMesh, i);
	}

	// Remove the bounding box
	RemoveBoundingBox(pMesh);
}

// Allocate memory to store vertices and triangles
void InitMesh(MESH_PTR pMesh, int ver_num )
{
	// Allocate memory for vertex array
	pMesh->pVerArr = (VERTEX2D_PTR)malloc((ver_num+3)*sizeof(VERTEX2D));
	if (pMesh->pVerArr == NULL)
	{
		fprintf(stderr,"Error:Allocate memory for mesh failed\n");
		exit(1);
	}

	pMesh->vertex_num = ver_num;

}

// Deallocate memory
void UnInitMesh(MESH_PTR pMesh)
{
	// free vertices
	if(pMesh->pVerArr != NULL)
		free(pMesh->pVerArr);

	// free triangles
	TRIANGLE_PTR pTri = pMesh->pTriArr;
	TRIANGLE_PTR pTemp = NULL;
	while (pTri != NULL)
	{
		pTemp = pTri->pNext;
		free(pTri);
		pTri = pTemp;
	}
}

void AddBoundingBox(MESH_PTR pMesh)
{
	float max = 0;
	float max_x = 0;
	float max_y = 0;
	float t;

	for (int i=3; i<pMesh->vertex_num+3; i++)
	{
        t = std::abs(float(((VERTEX2D_PTR)(pMesh->pVerArr+i))->x));
		if (max_x < t)
		{
			max_x = t;
		}

        t = abs(float(((VERTEX2D_PTR)(pMesh->pVerArr+i))->y));
		if (max_y < t)
		{
			max_y = t;
		}
	}

	max = max_x > max_y ? max_x:max_y;

	VERTEX2D v1 = {0, 4*max};
	VERTEX2D v2 = {-4*max, -4*max};
	VERTEX2D v3 = {4*max, 0};

	// Assign to Vertex array
	*(pMesh->pVerArr) = v1;
	*(pMesh->pVerArr + 1) = v2;
	*(pMesh->pVerArr + 2) = v3;

	// add the Triangle boundingbox
	AddTriangleNode(pMesh, NULL, 0, 1, 2);
}

void RemoveBoundingBox(MESH_PTR pMesh)
{
	int statify[3]={0,0,0};
	int vertex_index;
	int* pi;
	int k = 1;

	TRIANGLE_PTR pTri = pMesh->pTriArr;
	TRIANGLE_PTR pNext = NULL;
	while (pTri != NULL)
	{
		pNext = pTri->pNext;

		statify[0] = 0;
		statify[1] = 0;
		statify[2] = 0;

		pi = &(pTri->i1);
		for (int j=0, k = 1; j<3; j++, k*= 2)
		{			
			vertex_index = *pi++;

			if(vertex_index == 0 || vertex_index == 1 || vertex_index == 2) // bounding box vertex
			{
				statify[j] = k;
			}
		}

		switch(statify[0] | statify[1] | statify[2] )
		{
		case 0: // no statify
			break;
		case 1:
		case 2:
		case 4: // 1 statify, remove 1 triangle, 1 vertex
			RemoveTriangleNode(pMesh, pTri);		
			break;
		case 3:
		case 5:
		case 6: // 2 statify, remove 1 triangle, 2 vertices
			RemoveTriangleNode(pMesh, pTri);			
			break;
		case 7: // 3 statify, remove 1 triangle, 3 vertices
			RemoveTriangleNode(pMesh, pTri);
			break;
		default:
			break;
		}

		// go to next item
		pTri = pNext;
	}
}

float CounterClockWise(VERTEX2D_PTR pa, VERTEX2D_PTR pb, VERTEX2D_PTR pc)
{
	return ((pb->x - pa->x)*(pc->y - pb->y) - (pc->x - pb->x)*(pb->y - pa->y));
}

// Adjust if the point lies in the triangle abc
float InTriangle(MESH_PTR pMesh, VERTEX2D_PTR pVer, TRIANGLE_PTR pTri)
{
	int vertex_index;
	VERTEX2D_PTR pV1, pV2, pV3;

	vertex_index =pTri->i1;
	pV1 = (VERTEX2D_PTR)(pMesh->pVerArr+vertex_index);
	vertex_index =pTri->i2;
	pV2 = (VERTEX2D_PTR)(pMesh->pVerArr+vertex_index);
	vertex_index =pTri->i3;
	pV3 = (VERTEX2D_PTR)(pMesh->pVerArr+vertex_index);

	float ccw1 = CounterClockWise(pV1, pV2, pVer);
	float ccw2 = CounterClockWise(pV2, pV3, pVer);
	float ccw3 = CounterClockWise(pV3, pV1, pVer);

	float r = -1;
	if (ccw1>0 && ccw2>0 && ccw3>0)
	{
		r = 1;
	}
	else if(ccw1*ccw2*ccw3 == 0 && (ccw1*ccw2 > 0 || ccw1*ccw3 > 0 || ccw2*ccw3 > 0) )
	{
		r = 0;
	}

	return r;
}

void Insert(MESH_PTR pMesh, int ver_index)
{
	VERTEX2D_PTR pVer = (VERTEX2D_PTR)(pMesh->pVerArr+ver_index);
	TRIANGLE_PTR pTargetTri = NULL;
	TRIANGLE_PTR pEqualTri1 = NULL;
	TRIANGLE_PTR pEqualTri2 = NULL;

	int j = 0;
	TRIANGLE_PTR pTri = pMesh->pTriArr;
	while (pTri != NULL)
	{
		float r = InTriangle(pMesh, pVer, pTri);
		if(r > 0) // should be in triangle
		{
			pTargetTri = pTri;
		}
		else if (r == 0) // should be on edge
		{
			if(j == 0)
			{
				pEqualTri1 = pTri;
				j++;				
			}
			else
			{
				pEqualTri2 = pTri;
			}
			
		}

		pTri = pTri->pNext;
	}

	if (pEqualTri1 != NULL && pEqualTri2 != NULL)
	{
		InsertOnEdge(pMesh, pEqualTri1, ver_index);
		InsertOnEdge(pMesh, pEqualTri2, ver_index);
	}
	else
	{
		InsertInTriangle(pMesh, pTargetTri, ver_index);
	}
}

void InsertInTriangle(MESH_PTR pMesh, TRIANGLE_PTR pTargetTri, int ver_index)
{
	int index_a, index_b, index_c;
	TRIANGLE_PTR pTri = NULL;
	TRIANGLE_PTR pNewTri = NULL;

	pTri = pTargetTri;	
	if(pTri == NULL)
	{
		return;
	}

	// Inset p into target triangle
	index_a = pTri->i1;
	index_b = pTri->i2;
	index_c = pTri->i3;

	// Insert edge pa, pb, pc
	for(int i=0; i<3; i++)
	{
		// allocate memory
		if(i == 0)
		{
			pNewTri = AddTriangleNode(pMesh, pTri, index_a, index_b, ver_index);
		}
		else if(i == 1)
		{
			pNewTri = AddTriangleNode(pMesh, pTri, index_b, index_c, ver_index);
		}
		else
		{
			pNewTri = AddTriangleNode(pMesh, pTri, index_c, index_a, ver_index);
		}

		// go to next item
		if (pNewTri != NULL)
		{
			pTri = pNewTri;
		}
		else
		{
			pTri = pTri;
		}
	}

	// Get the three sub-triangles
	pTri = pTargetTri;	
	TRIANGLE_PTR pTestTri[3];
	for (int i=0; i< 3; i++)
	{
		pTestTri[i] = pTri->pNext;

		pTri = pTri->pNext;
	}

	// remove the Target Triangle
	RemoveTriangleNode(pMesh, pTargetTri);

	for (int i=0; i< 3; i++)
	{
		// Flip test
		FlipTest(pMesh, pTestTri[i]);
	}
}

void InsertOnEdge(MESH_PTR pMesh, TRIANGLE_PTR pTargetTri, int ver_index)
{
	int index_a, index_b, index_c;
	TRIANGLE_PTR pTri = NULL;
	TRIANGLE_PTR pNewTri = NULL;

	pTri = pTargetTri;	
	if(pTri == NULL)
	{
		return;
	}

	// Inset p into target triangle
	index_a = pTri->i1;
	index_b = pTri->i2;
	index_c = pTri->i3;

	// Insert edge pa, pb, pc
	for(int i=0; i<3; i++)
	{
		// allocate memory
		if(i == 0)
		{
			pNewTri = AddTriangleNode(pMesh, pTri, index_a, index_b, ver_index);
		}
		else if(i == 1)
		{
			pNewTri = AddTriangleNode(pMesh, pTri, index_b, index_c, ver_index);
		}
		else
		{
			pNewTri = AddTriangleNode(pMesh, pTri, index_c, index_a, ver_index);
		}		

		// go to next item
		if (pNewTri != NULL)
		{
			pTri = pNewTri;
		}
		else
		{
			pTri = pTri;
		}
	}

	// Get the two sub-triangles
	pTri = pTargetTri;	
	TRIANGLE_PTR pTestTri[2];
	for (int i=0; i< 2; i++)
	{
		pTestTri[i] = pTri->pNext;
		pTri = pTri->pNext;
	}

	// remove the Target Triangle
	RemoveTriangleNode(pMesh, pTargetTri);

	for (int i=0; i< 2; i++)
	{
		// Flip test
		FlipTest(pMesh, pTestTri[i]);
	}
}

bool FlipTest(MESH_PTR pMesh, TRIANGLE_PTR pTestTri)
{
	bool flipped = false;

	int index_a = pTestTri->i1;
	int index_b = pTestTri->i2;
	int index_p = pTestTri->i3;

	int statify[3]={0,0,0};
	int vertex_index;
	int* pi;
	int k = 1;

	// find the triangle which has edge consists of start and end
	TRIANGLE_PTR pTri = pMesh->pTriArr;

	int index_d = -1;
	while (pTri != NULL)
	{
		statify[0] = 0;
		statify[1] = 0;
		statify[2] = 0;

		pi = &(pTri->i1);
		for (int j=0, k = 1; j<3; j++, k*= 2)
		{
			vertex_index = *pi++;
			if(vertex_index == index_a || vertex_index == index_b)
			{
				statify[j] = k;
			}
		}

		switch(statify[0] | statify[1] | statify[2] )
		{
			case 3:
				if(CounterClockWise((VERTEX2D_PTR)(pMesh->pVerArr+index_a), (VERTEX2D_PTR)(pMesh->pVerArr+index_b), (VERTEX2D_PTR)(pMesh->pVerArr+pTri->i3)) < 0)
				{
					index_d = pTri->i3;
				}
				
				break;
			case 5:
				if(CounterClockWise((VERTEX2D_PTR)(pMesh->pVerArr+index_a), (VERTEX2D_PTR)(pMesh->pVerArr+index_b), (VERTEX2D_PTR)(pMesh->pVerArr+pTri->i2)) < 0)
				{
					index_d = pTri->i2;
				}
				
				break;
			case 6: 
				if(CounterClockWise((VERTEX2D_PTR)(pMesh->pVerArr+index_a), (VERTEX2D_PTR)(pMesh->pVerArr+index_b), (VERTEX2D_PTR)(pMesh->pVerArr+pTri->i1)) < 0)
				{
					index_d = pTri->i1;
				}

				break;

			default:
				break;
		}

		if (index_d != -1)
		{
			VERTEX2D_PTR pa = (VERTEX2D_PTR)(pMesh->pVerArr+index_a);
			VERTEX2D_PTR pb = (VERTEX2D_PTR)(pMesh->pVerArr+index_b);
			VERTEX2D_PTR pd = (VERTEX2D_PTR)(pMesh->pVerArr+index_d);
			VERTEX2D_PTR pp = (VERTEX2D_PTR)(pMesh->pVerArr+index_p);
			
			if(InCircle( pa, pb, pp, pd) < 0) // not local Delaunay
			{
				flipped = true;

				// add new triangle adp,  dbp, remove abp, abd.
				// allocate memory for adp
				TRIANGLE_PTR pT1 = AddTriangleNode(pMesh, pTestTri, pTestTri->i1, index_d, pTestTri->i3);				
				// allocate memory for dbp
				TRIANGLE_PTR pT2 = AddTriangleNode(pMesh, pT1, index_d, pTestTri->i2, index_p);				
				// remove abp
				RemoveTriangleNode(pMesh, pTestTri);
				// remove abd
				RemoveTriangleNode(pMesh, pTri);

				FlipTest(pMesh, pT1); // pNewTestTri satisfies CCW order
				FlipTest(pMesh, pT2); // pNewTestTri2  satisfies CCW order

				break;
			}			
		}

		// go to next item	
		pTri = pTri->pNext;
	}

	return flipped;
}

// In circle test, use vector cross product
float InCircle(VERTEX2D_PTR pa, VERTEX2D_PTR pb, VERTEX2D_PTR pp, VERTEX2D_PTR  pd)
{
	float det;
	float alift, blift, plift, bdxpdy, pdxbdy, pdxady, adxpdy, adxbdy, bdxady;

	float adx = pa->x - pd->x;
	float ady = pa->y - pd->y;

	float bdx = pb->x - pd->x;
	float bdy = pb->y - pd->y;

	float pdx = pp->x - pd->x;
	float pdy = pp->y - pd->y;

	bdxpdy = bdx * pdy;
	pdxbdy = pdx * bdy;
	alift = adx * adx + ady * ady;

	pdxady = pdx * ady;
	adxpdy = adx * pdy;
	blift = bdx * bdx + bdy * bdy;

	adxbdy = adx * bdy;
	bdxady = bdx * ady;
	plift = pdx * pdx + pdy * pdy;

	det = alift * (bdxpdy - pdxbdy)
		+ blift * (pdxady - adxpdy)
		+ plift * (adxbdy - bdxady);

	return -det;
}

// Remove a node from the triangle list and deallocate the memory
void RemoveTriangleNode(MESH_PTR pMesh, TRIANGLE_PTR pTri)
{
	if (pTri == NULL)
	{
		return;
	}

	// remove from the triangle list
	if (pTri->pPrev != NULL)
	{
		pTri->pPrev->pNext = pTri->pNext;
	}
	else // remove the head, need to reset the root node
	{
		pMesh->pTriArr = pTri->pNext;
	}

	if (pTri->pNext != NULL)
	{
		pTri->pNext->pPrev = pTri->pPrev;
	}

	// deallocate memory
	free(pTri);	
}

// Create a new node and add it into triangle list
TRIANGLE_PTR AddTriangleNode(MESH_PTR pMesh, TRIANGLE_PTR pPrevTri, int i1, int i2, int i3)
{
	// test if 3 vertices are co-linear
	if(CounterClockWise((VERTEX2D_PTR)(pMesh->pVerArr+i1), (VERTEX2D_PTR)(pMesh->pVerArr+i2), (VERTEX2D_PTR)(pMesh->pVerArr+i3)) == 0)
	{
		return NULL;
	}

	// allocate memory
	TRIANGLE_PTR pNewTestTri = (TRIANGLE_PTR)malloc(sizeof(TRIANGLE));

	pNewTestTri->i1 = i1;
	pNewTestTri->i2 = i2;
	pNewTestTri->i3 = i3;

	// insert after prev triangle
	if (pPrevTri == NULL) // add root
	{
		pMesh->pTriArr = pNewTestTri;
		pNewTestTri->pNext = NULL;
		pNewTestTri->pPrev = NULL;
	}
	else
	{
		pNewTestTri->pNext = pPrevTri->pNext;
		pNewTestTri->pPrev = pPrevTri;

		if(pPrevTri->pNext != NULL)
		{
			pPrevTri->pNext->pPrev = pNewTestTri;
		}

		pPrevTri->pNext = pNewTestTri;
	}
	
	return pNewTestTri;
}
