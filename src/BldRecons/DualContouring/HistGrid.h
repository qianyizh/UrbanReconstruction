#pragma once

#include "Geometry\Vector3D.h"
#include "Geometry\BoundingBox.h"
#include "Geometry\LinePlane.h"
#include "Miscs\Histogram.h"
#include "Miscs\HistWriter.h"
#include "Miscs\HistReader.h"
#include "DCContourer.h"

enum FixType { FT_Free, FT_HalfFixed, FT_Fixed };

struct AuxVertex {
	FixType t;
	int li[ 2 ];
	AuxVertex() { t = FT_Free; li[0] = li[1] = -1; }
};

struct AuxLine {
	CLine l;
	int pi;
	double acc_length;
	double acc_error;
	// first compare acc_length
	// if they are the same, then compare acc_error
	std::vector< int > vi;
};

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

	CMesh * m_pMesh;
	CMeshBoundary * m_pBoundary;
	CDCGrid * m_pGrid;
	double m_dbErrorTolerance;
	double m_dbMinimumLength;

public:
	void Init( char filename[], char histfilename[] );
	void Simplify( CDCContourer & contourer, double error_tolerance, double minimum_length );

private:
	void LoadFromHist( char filename[] );
	CHistogram & LocateHistogram( CVector3D & v );

	// fitting along principal directions
	//int FittingPrincipalDirection( COutline & outline, CAuxOutline & aux, std::vector< int > & vertex_loop );
	void FittingPrincipalDirection( int i );
	void FittingSimplify( CMeshBoundary::CAuxBoundary & bdr, std::vector< AuxVertex > & verts, std::vector< AuxLine > & lines );
	void FixVertices( int i );

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

	bool IsBetterMaxLine( double acc_length, double acc_error, AuxLine & max_line ) {
		// first compare acc_length, the larger the better
		// if they are the same, then compare acc_error, the smaller the better
		if ( abs( acc_length - max_line.acc_length ) < 1e-4 ) {
			if ( acc_error < max_line.acc_error )
				return true;
			else
				return false;
		} else if ( acc_length > max_line.acc_length ) {
			return true;
		} else {
			return false;
		}
	}
};
