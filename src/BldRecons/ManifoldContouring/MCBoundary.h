#pragma once

#include "MCGrid.h"

//////////////////////////////////////////////////////////////////////////
//      2
//    3   1
//      0
//////////////////////////////////////////////////////////////////////////

class CMCBoundary
{
public:
	CMCBoundary(void);
	~CMCBoundary(void);

protected:
	CMCGrid * m_pGrid;

public:
	void GetDummyBoundary( CMCGrid * grid );

protected:
	int NodeIndex( int i, int j ) {
		return ( i << m_pGrid->m_nLevel ) + j;
	}

	bool IsIndexValid( int i ) {
		return ( i >= 0 && i < ( 1 << m_pGrid->m_nLevel ) - 1 );
	}
	
	int EdgeIndex( int i, int j, int k ) {
		return NodeIndex( i, j ) * 4 + k;
	}

	int NextEdgeIndex( int i, int j, int k ) {
		return EdgeIndex( i, j, ( k + 1 ) % 4 );
	}

	int TwinEdgeIndex( int i, int j, int k ) {
		int ij_shift[ 2 ][ 4 ] = {
			{ 0, 1, 0, -1 },
			{ -1, 0, 1, 0 }
		};
		
		if ( IsIndexValid( i + ij_shift[ 0 ][ k ] ) && IsIndexValid( j + ij_shift[ 1 ][ k ] ) ) {
			return EdgeIndex( i + ij_shift[ 0 ][ k ], j + ij_shift[ 1 ][ k ], ( k + 2 ) % 4 );
		} else {
			return -1;
		}
	}

	bool CheckTwinEdge( CMCGrid::MCHalfEdge & this_edge, CMCGrid::MCHalfEdge & twin_edge ) {
		return ( twin_edge.valid
			&& this_edge.h[ 0 ] == twin_edge.h[ 1 ] && this_edge.hl[ 0 ] == twin_edge.hl[ 1 ]
			&& this_edge.h[ 1 ] == twin_edge.h[ 0 ] && this_edge.hl[ 1 ] == twin_edge.hl[ 0 ]);
	}

	int NextBoundaryEdge( int ei )
	{
		if ( m_pGrid->m_vecTopoEdges[ ei ].twin != -1 ) {
			return -1;
		} else {
			int next_ei = m_pGrid->m_vecTopoEdges[ ei ].next;
			while ( m_pGrid->m_vecTopoEdges[ next_ei ].twin != -1 )
				next_ei = m_pGrid->m_vecTopoEdges[ m_pGrid->m_vecTopoEdges[ next_ei ].twin ].next;
			return next_ei;
		}
	}

	void AssignEdgeToBoundary( int ei, int bi ) {
		m_pGrid->m_vecBoundaries[ bi ].ei.push_back( ei );
		CMCGrid::MCHalfEdge & edge = m_pGrid->m_vecTopoEdges[ ei ];
		edge.bi = bi;
		edge.h[ 0 ]->bi[ edge.hl[ 0 ] ] = bi;
		edge.h[ 1 ]->bi[ edge.hl[ 1 ] ] = bi;
	}

	bool NotGroundEdge( CMCGrid::MCHalfEdge & edge ) {
		double g = m_pGrid->m_pPointCloud->m_dbGroundZ;
		return ( abs( edge.h[ 0 ]->points[ edge.hl[ 0 ] ][ 2 ] - g ) > 1e-5 || abs( edge.h[ 1 ]->points[ edge.hl[ 1 ] ][ 2 ] - g ) > 1e-5 );
	}
};
