#pragma once

#include <vector>

#include "PointCloud.h"
#include "BoundingBox.h"

//////////////////////////////////////////////////////////////////////////
// This is a indexing grid with PROTECTED boundary
// Boundary cells are guaranteed to be empty cells
//////////////////////////////////////////////////////////////////////////

class CIndexingGrid
{
public:
	typedef std::vector< int > Grid_Index;

public:
	CIndexingGrid(void);
	~CIndexingGrid(void);

public:
	CPointCloud * m_pPointCloud;

	int m_nWidth;
	int m_nHeight;
	double m_dbGridLength;

	std::vector< Grid_Index > m_vecIndex;

public:
	void BuildIndexingGrid( CPointCloud * pointcloud, double unit_length );

protected:
	void CreateIndex();

protected:
	int XtoI( double x ) {
		int i = ( int )( ( x - m_pPointCloud->m_cBoundingBox.m_vMin[ 0 ] ) / m_dbGridLength ) + 1;
		if ( i < 1 ) i = 1;
		if ( i >= m_nWidth - 1 ) i = m_nWidth - 2;
		return i;
	}
	int YtoJ( double y ) {
		int j = ( int )( ( y - m_pPointCloud->m_cBoundingBox.m_vMin[ 1 ] ) / m_dbGridLength ) + 1;
		if ( j < 1 ) j = 1;
		if ( j >= m_nHeight - 1 ) j = m_nHeight - 2;
		return j;
	}

	bool IIsValid( int i ) {
		return ( i >= 0 && i < m_nWidth );
	}
	bool JIsValid( int j ) {
		return ( j >= 0 && j < m_nHeight );
	}

	int Index( int i, int j ) {
		return ( i * m_nHeight + j );
	}

	int Index( double x, double y ) {
		return Index( XtoI( x ), YtoJ( y ) );
	}

	int GetI( int index ) {
		return index / m_nHeight;
	}

	int GetJ( int index ) {
		return index % m_nHeight;
	}
};
