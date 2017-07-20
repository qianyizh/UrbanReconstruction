#pragma once

#include <vector>

#include "LinePlane.h"

class COutlineVertex
{
public:
	CVector3D v;
	CVector3D d;
	int prev;
	int next;
};

class COutlineTriangle
{
public:
	int vi[ 3 ];

public:
	COutlineTriangle() {
	}
	COutlineTriangle( int i, int j, int k ) {
		vi[0] = i; vi[1] = j; vi[2] = k;
	}
};

class COutline
{
public:
	COutline(void);
	~COutline(void);

public:
	CPlane m_cPlane;
	std::vector< COutlineVertex > m_vecVertex;
	std::vector< COutlineTriangle > m_vecRoofTriangle;

public:
	void Add( int v0, int v1 );
	void ComputeDirection( int nNeighbor );
	void Save( FILE * pFile );
	void Load( FILE * pFile );

public:
	void GenerateRoofTriangle();

	void WriteOBJVertex( FILE * pFile, double ground_z );
	void WriteOBJFace( FILE * pFile, int offset, bool wall_rectangle );

private:
	bool InPolygon( double x, double y, int n, const double * px, const double * py );
};
