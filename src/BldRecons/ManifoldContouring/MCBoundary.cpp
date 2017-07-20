#include "StdAfx.h"
#include "MCBoundary.h"

CMCBoundary::CMCBoundary(void)
{
}

CMCBoundary::~CMCBoundary(void)
{
}

void CMCBoundary::GetDummyBoundary( CMCGrid * grid )
{
	int sidenumber = ( 1 << grid->m_nLevel );
	m_pGrid = grid;
	grid->m_vecBoundaries.clear();
	grid->m_vecTopoEdges.clear();
	grid->m_vecTopoEdges.resize( sidenumber * sidenumber * 4 );
	CMCGrid::MCGridNode_Vector & nodes0 = m_pGrid->m_vecNodes[ 0 ];
	CMCGrid::MCHalfEdge_Vector & edges = m_pGrid->m_vecTopoEdges;

	for ( int i = 0; i < sidenumber - 1; i++ ) {
		for ( int j = 0; j < sidenumber - 1; j++ ) {
			CMCGrid::MCGridNode * n[ 2 ][ 2 ] = {
				{ & nodes0[ i * sidenumber + j ],  & nodes0[ i * sidenumber + j + 1 ] },
				{ & nodes0[ ( i + 1 ) * sidenumber + j ],  & nodes0[ ( i + 1 ) * sidenumber + j + 1 ] }
			};

			bool valid = true;
			if ( n[ 0 ][ 0 ]->hpoints[ 0 ].is_ground
				|| n[ 0 ][ 1 ]->hpoints[ 0 ].is_ground 
				|| n[ 1 ][ 0 ]->hpoints[ 0 ].is_ground 
				|| n[ 1 ][ 1 ]->hpoints[ 0 ].is_ground ) {
				valid = false;
			}

			//////////////////////////////////////////////////////////////////////////
			//      2
			//    3   1
			//      0
			//////////////////////////////////////////////////////////////////////////
			const int k_index_ij_2_nij[ 4 ][ 2 ][ 2 ] = {
				{ { 0, 0 }, { 1, 0 } },
				{ { 1, 0 }, { 1, 1 } },
				{ { 1, 1 }, { 0, 1 } },
				{ { 0, 1 }, { 0, 0 } },
			};
			int cluster[ 2 ][ 2 ] = {
				{ n[ 0 ][ 0 ]->cluster[ m_pGrid->Cluster_Index( 1, 1, 0 ) ], n[ 0 ][ 1 ]->cluster[ m_pGrid->Cluster_Index( 1, 0, 0 ) ] },
				{ n[ 1 ][ 0 ]->cluster[ m_pGrid->Cluster_Index( 0, 1, 0 ) ], n[ 1 ][ 1 ]->cluster[ m_pGrid->Cluster_Index( 0, 0, 0 ) ] }
			};

			for ( int k = 0; k < 4; k++ ) {
				CMCGrid::MCHalfEdge & edge = edges[ EdgeIndex( i, j, k ) ];
				edge.valid = valid;
				if ( valid ) {
					edge.h[ 0 ] = & n[ k_index_ij_2_nij[ k ][ 0 ][ 0 ] ][ k_index_ij_2_nij[ k ][ 0 ][ 1 ] ]->hpoints[ 0 ];
					edge.h[ 1 ] = & n[ k_index_ij_2_nij[ k ][ 1 ][ 0 ] ][ k_index_ij_2_nij[ k ][ 1 ][ 1 ] ]->hpoints[ 0 ];
					edge.hl[ 0 ] = cluster[ k_index_ij_2_nij[ k ][ 0 ][ 0 ] ][ k_index_ij_2_nij[ k ][ 0 ][ 1 ] ];
					edge.hl[ 1 ] = cluster[ k_index_ij_2_nij[ k ][ 1 ][ 0 ] ][ k_index_ij_2_nij[ k ][ 1 ][ 1 ] ];
				}
			}
		}
	}

	for ( int i = 0; i < sidenumber - 1; i++ ) {
		for ( int j = 0; j < sidenumber - 1; j++ ) {
			for ( int k = 0; k < 4; k++ ) {
				CMCGrid::MCHalfEdge & edge = edges[ EdgeIndex( i, j, k ) ];
				if ( edge.valid ) {
					edge.bi = -1;
					edge.next = NextEdgeIndex( i, j, k );
					edge.twin = TwinEdgeIndex( i, j, k );
					if ( edge.twin == -1 || CheckTwinEdge( edge, edges[ edge.twin ] ) == false ) {
						edge.twin = -1;
					}
				}
			}
		}
	}

	// now extract boundaries
	for ( int i = 0; i < sidenumber - 1; i++ ) {
		for ( int j = 0; j < sidenumber - 1; j++ ) {
			for ( int k = 0; k < 4; k++ ) {
				CMCGrid::MCHalfEdge & edge = edges[ EdgeIndex( i, j, k ) ];
				if ( edge.valid && edge.twin == -1 && edge.bi == -1 && NotGroundEdge( edge ) ) {
					int bi = ( int )grid->m_vecBoundaries.size();
					grid->m_vecBoundaries.resize( bi + 1 );
					CMCGrid::MCBoundary & bdr = grid->m_vecBoundaries.back();

					bdr.seed_edge = EdgeIndex( i, j, k );
					AssignEdgeToBoundary( bdr.seed_edge, bi );
					int ei = NextBoundaryEdge( bdr.seed_edge );

					while ( ei != bdr.seed_edge ) {
						AssignEdgeToBoundary( ei, bi );
						ei = NextBoundaryEdge( ei );
					}

					bdr.remaining_v_num = ( int )bdr.ei.size();
				}
			}
		}
	}
}
