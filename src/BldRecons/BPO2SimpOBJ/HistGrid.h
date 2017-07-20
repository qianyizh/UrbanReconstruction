#pragma once

#include "Geometry\Vector3D.h"
#include "Geometry\BoundingBox.h"
#include "Geometry\Outline.h"
#include "Miscs\Histogram.h"
#include "Miscs\HistWriter.h"
#include "Miscs\HistReader.h"
#include "AuxOutline.h"

class CHistGrid
{
public:
	CHistGrid(void);
	~CHistGrid(void);

public:
	std::vector< CVector3D > m_vecCenter;
	std::vector< CHistogram > m_vecHistogram;
	CBoundingBox m_cBoundingBox;
	int m_nGridNumber[ 2 ];
	CHistWriter m_cWriter;
	CHistReader m_cReader;
	HistHeader m_cHeader;

public:
	void Init();
	void Simplify();

private:
	void LoadFromHist( char filename[] );
	CHistogram & LocateHistogram( CVector3D & v );

private:
	void SimplifyOutline( std::vector< COutline > & vecOutline, std::vector< COutline > & vecSimpOutline );
	void SnapLines( std::vector< CAuxOutline > & vecAuxOutline, std::vector< COutline > & vecOutline );

private:
	// fitting along principal directions
	int FittingPrincipalDirection( COutline & outline, CAuxOutline & aux, std::vector< int > & vertex_loop );

private:
	int CompareVector2D( CVector3D & v0, CVector3D & v1 ) {
		if ( v0[0] > v1[0] ) {
			return 1;
		} else if ( v0[0] == v1[0] ) {
			if ( v0[1] > v1[1] )
				return 1;
			else if ( v0[1] == v1[1] )
				return 0;
			else
				return -1;
		} else {
			return -1;
		}
	}
};
