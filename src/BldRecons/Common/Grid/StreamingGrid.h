#pragma once

#include "Geometry\BoundingBox.h"

class CStreamingGrid
{
public:
	CStreamingGrid(void);
	~CStreamingGrid(void);

public:
	CBoundingBox m_cBoundingBox;
	int m_nCellDepth;
	double m_dbGridLength;
	int m_nSideNumber;					// 2^k
	int m_nUnitNumber[ 2 ];				// unit_length = unit_number * grid_length
	double m_dbUnit[ 2 ];
	int m_nPointNumber;

protected:
	int m_nProgressNumber;
	int m_nReadNumber;
	void InitPrintProgress();
	void PrintProgress();
	void IncReadNumber();

protected:
	int Index( const CVector3D & v );
	void Regularize( int & i );

	inline int Index( int x, int y ) {
		return ( x * m_nSideNumber + y );
	}
	inline int I2X( int i ) {
		return i / m_nSideNumber;
	}
	inline int I2Y( int i ) {
		return i % m_nSideNumber;
	}

	inline int MinBound( int xy ) {
		return __max( 0, xy - 1 );
	}
	inline int MaxBound( int xy ) {
		return __min( m_nSideNumber - 1, xy + 1 );
	}
	inline int InBound( int x, int y ) {
		return ( x >= 0 && x < m_nSideNumber && y >= 0 && y < m_nSideNumber );
	}
};
