#include "StdAfx.h"
#include "MCGrid.h"

void CMCGrid::DetectTopologyFeatures()
{
	MCGridNode_Vector & nodes = m_vecNodes[ 0 ];
	int sidelength = 1;
	int sidenumber = ( 1 << m_nLevel );

	// 1. all h-points with 3 or more layers are topology features
	for ( int i = 0; i < sidenumber; i++ ) {
		for ( int j = 0; j < sidenumber; j++ ) {
			MCGridNode & node = nodes[ i * sidenumber + j ];
			MCHyperPoint & hpoint = node.hpoints[ 0 ];

			if ( hpoint.num_of_layers > 2 ) {
				hpoint.is_feature = true;
			} else {
				hpoint.is_feature = false;
				hpoint.leaf_to_parent[ 0 ] = 0;
				hpoint.leaf_to_parent[ 1 ] = 1;
			}
		}
	}

	// 2. all h-points associating with an inconsistent edge are topology features (folding points)
	// Edge along X
	for ( int i = 0; i < sidenumber; i++ ) {
		for ( int j = 0; j < sidenumber - 1; j++ ) {
			// ( i, j ) - ( i, j + 1 )
			MCGridNode & node0 = nodes[ i * sidenumber + j ];
			MCGridNode & node1 = nodes[ i * sidenumber + ( j + 1 ) ];
			int c[ 2 ][ 2 ] = {
				{ node0.cluster[ Cluster_Index( 0, 1, 0 ) ], node0.cluster[ Cluster_Index( 1, 1, 0 ) ] },
				{ node1.cluster[ Cluster_Index( 0, 0, 0 ) ], node1.cluster[ Cluster_Index( 1, 0, 0 ) ] }
			};

			if ( c[ 0 ][ 0 ] == c[ 0 ][ 1 ] && c[ 1 ][ 0 ] != c[ 1 ][ 1 ] ) {
				node0.hpoints[ 0 ].is_feature = true;
				node0.hpoints[ 0 ].is_foldingpoint = true;
				//fprintf_s( stderr, "%d %d bottom: (%d %d, %d %d), (%d %d, %d %d)\n", i, j, node0.cluster[0], node0.cluster[1], node0.cluster[2], node0.cluster[3], node1.cluster[0], node1.cluster[1], node1.cluster[2], node1.cluster[3] );
			} else if ( c[ 0 ][ 0 ] != c[ 0 ][ 1 ] && c[ 1 ][ 0 ] == c[ 1 ][ 1 ] ) {
				node1.hpoints[ 0 ].is_feature = true;
				node1.hpoints[ 0 ].is_foldingpoint = true;
				//fprintf_s( stderr, "%d %d top: (%d %d, %d %d), (%d %d, %d %d)\n", i, j + 1, node0.cluster[0], node0.cluster[1], node0.cluster[2], node0.cluster[3], node1.cluster[0], node1.cluster[1], node1.cluster[2], node1.cluster[3] );
			}
		}
	}

	// Edge along Y
	for ( int i = 0; i < sidenumber - 1; i++ ) {
		for ( int j = 0; j < sidenumber; j++ ) {
			// ( i, j ) - ( i + 1, j )
			MCGridNode & node0 = nodes[ i * sidenumber + j ];
			MCGridNode & node1 = nodes[ ( i + 1 ) * sidenumber + j ];
			int c[ 2 ][ 2 ] = {
				{ node0.cluster[ Cluster_Index( 1, 0, 0 ) ], node0.cluster[ Cluster_Index( 1, 1, 0 ) ] },
				{ node1.cluster[ Cluster_Index( 0, 0, 0 ) ], node1.cluster[ Cluster_Index( 0, 1, 0 ) ] }
			};

			if ( c[ 0 ][ 0 ] == c[ 0 ][ 1 ] && c[ 1 ][ 0 ] != c[ 1 ][ 1 ] ) {
				node0.hpoints[ 0 ].is_feature = true;
				node0.hpoints[ 0 ].is_foldingpoint = true;
				//fprintf_s( stderr, "%d %d right: (%d %d, %d %d), (%d %d, %d %d)\n", i, j, node0.cluster[0], node0.cluster[1], node0.cluster[2], node0.cluster[3], node1.cluster[0], node1.cluster[1], node1.cluster[2], node1.cluster[3] );
			} else if ( c[ 0 ][ 0 ] != c[ 0 ][ 1 ] && c[ 1 ][ 0 ] == c[ 1 ][ 1 ] ) {
				node1.hpoints[ 0 ].is_feature = true;
				node1.hpoints[ 0 ].is_foldingpoint = true;
				//fprintf_s( stderr, "%d %d left: (%d %d, %d %d), (%d %d, %d %d)\n", i + 1, j, node0.cluster[0], node0.cluster[1], node0.cluster[2], node0.cluster[3], node1.cluster[0], node1.cluster[1], node1.cluster[2], node1.cluster[3] );
			}
		}
	}
}