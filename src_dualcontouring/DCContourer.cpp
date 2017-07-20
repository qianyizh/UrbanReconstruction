#include "StdAfx.h"
#include "DCContourer.h"

CDCContourer::CDCContourer(void)
{
}

CDCContourer::~CDCContourer(void)
{
}

//////////////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////////////

void CDCContourer::AssignDCGrid( CDCGrid *grid )
{
	m_pGrid = grid;
}

void CDCContourer::Contouring()
{
	m_pGrid->m_vecClusterAssignment.resize( ( ( 1 << m_pGrid->m_nLevel ) + 1 ) * ( ( 1 << m_pGrid->m_nLevel ) + 1 ) );

	PreProcessing( Root() );

	CreateVertex( Root() );

	CreateFacePerNode( Root() );
}

//////////////////////////////////////////////////////////////////////////
// pre-processing functions
//////////////////////////////////////////////////////////////////////////

void CDCContourer::PreProcessing( CDCGrid::NodeLocator & loc )
{
	CDCGrid::DCGridNode & node = m_pGrid->LocateNode( loc );
	if ( node.pass == false ) {
		for ( int i = 0; i < 2; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				PreProcessing( m_pGrid->Leaf( loc, i, j ) );
			}
		}
	} else {
		// only work for grid corners on boundary
		// at each corner
		// 01 | 11
		// ---*---
		// 00 | 10

		int i = loc.i;
		int j = loc.j;
		int l = loc.l;
		int ii, jj;

		for ( ii = 0; ii < ( 1 << l ); ii++ ) {
			jj = 0;
			m_pGrid->m_vecClusterAssignment[ m_pGrid->RootCluster_Index( ii + ( i << l ), jj + ( j << l ) ) ].l[ 1 ][ 1 ] = node.cluster[ ii * ( ( 1 << l ) + 1 ) + jj ];
			jj = ( 1 << l );
			m_pGrid->m_vecClusterAssignment[ m_pGrid->RootCluster_Index( ii + ( i << l ), jj + ( j << l ) ) ].l[ 1 ][ 0 ] = node.cluster[ ii * ( ( 1 << l ) + 1 ) + jj ];
		}
		for ( ii = 1; ii <= ( 1 << l ); ii++ ) {
			jj = 0;
			m_pGrid->m_vecClusterAssignment[ m_pGrid->RootCluster_Index( ii + ( i << l ), jj + ( j << l ) ) ].l[ 0 ][ 1 ] = node.cluster[ ii * ( ( 1 << l ) + 1 ) + jj ];
			jj = ( 1 << l );
			m_pGrid->m_vecClusterAssignment[ m_pGrid->RootCluster_Index( ii + ( i << l ), jj + ( j << l ) ) ].l[ 0 ][ 0 ] = node.cluster[ ii * ( ( 1 << l ) + 1 ) + jj ];
		}

		for ( jj = 0; jj < ( 1 << l ); jj++ ) {
			ii = 0;
			m_pGrid->m_vecClusterAssignment[ m_pGrid->RootCluster_Index( ii + ( i << l ), jj + ( j << l ) ) ].l[ 1 ][ 1 ] = node.cluster[ ii * ( ( 1 << l ) + 1 ) + jj ];
			ii = ( 1 << l );
			m_pGrid->m_vecClusterAssignment[ m_pGrid->RootCluster_Index( ii + ( i << l ), jj + ( j << l ) ) ].l[ 0 ][ 1 ] = node.cluster[ ii * ( ( 1 << l ) + 1 ) + jj ];
		}
		for ( jj = 1; jj <= ( 1 << l ); jj++ ) {
			ii = 0;
			m_pGrid->m_vecClusterAssignment[ m_pGrid->RootCluster_Index( ii + ( i << l ), jj + ( j << l ) ) ].l[ 1 ][ 0 ] = node.cluster[ ii * ( ( 1 << l ) + 1 ) + jj ];
			ii = ( 1 << l );
			m_pGrid->m_vecClusterAssignment[ m_pGrid->RootCluster_Index( ii + ( i << l ), jj + ( j << l ) ) ].l[ 0 ][ 0 ] = node.cluster[ ii * ( ( 1 << l ) + 1 ) + jj ];
		}

		for ( int i = 0; i < node.num_of_layers; i++ ) {
			node.seq.push_back( CDCGrid::DistanceSorting( i, node.points[ i ][ 2 ] ) );
		}
		m_pGrid->QSort( node.seq );
	}
}

//////////////////////////////////////////////////////////////////////////
// create vertex functions
//////////////////////////////////////////////////////////////////////////

void CDCContourer::CreateVertex( CDCGrid::NodeLocator & loc )
{
	CDCGrid::DCGridNode & node = m_pGrid->LocateNode( loc );
	if ( node.pass == false ) {
		for ( int i = 0; i < 2; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				CreateVertex( m_pGrid->Leaf( loc, i, j ) );
			}
		}
	} else {
		if ( node.num_of_layers == 1 && abs( node.points[ 0 ][ 2 ] - m_pGrid->m_pPointCloud->m_dbGroundZ ) < 1e-5 ) {
			node.vi = -1;
		} else {
			node.vi = ( int )m_cMesh.m_vecVertex.size();
			for ( int i = 0; i < node.num_of_layers; i++ ) {
				m_cMesh.m_vecVertex.push_back( CMesh::MeshVertex( node.points[ i ] ) );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// create face functions
//////////////////////////////////////////////////////////////////////////
void CDCContourer::CreateFacePerNode( CDCGrid::NodeLocator & loc )
{
	CDCGrid::DCGridNode * n = & m_pGrid->LocateNode( loc );

	if ( n->pass == false ) {
		// not a leaf node
		CDCGrid::NodeLocator pointloc[ 2 ][ 2 ];
		CDCGrid::NodeLocator edgeloc[ 2 ];
		for ( int i = 0; i < 2; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				pointloc[ i ][ j ] = m_pGrid->Leaf( loc, i, j, n->pass );
			}
		}

		for ( int i = 0; i < 2; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				CreateFacePerNode( pointloc[ i ][ j ] );
			}
		}

		for ( int i = 0; i < 2; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				edgeloc[ j ] = pointloc[ i ][ j ];
			}
			CreateFacePerEdge( edgeloc, ED_Along_X );
		}
		for ( int i = 0; i < 2; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				edgeloc[ j ] = pointloc[ j ][ i ];
			}
			CreateFacePerEdge( edgeloc, ED_Along_Y );
		}

		CreateFacePerPoint( pointloc );
	} else {
	}
}

void CDCContourer::CreateFacePerEdge( CDCGrid::NodeLocator loc[ 2 ], EdgeDirection dir )
{
	CDCGrid::DCGridNode * n[ 2 ] = { & m_pGrid->LocateNode( loc[ 0 ] ), & m_pGrid->LocateNode( loc[ 1 ] ) };

	if ( n[0]->pass == false || n[1]->pass == false ) {
		// not a minimum edge
		if ( dir == ED_Along_X ) {
			CDCGrid::NodeLocator pointloc[ 2 ][ 2 ];
			CDCGrid::NodeLocator edgeloc[ 2 ];

			for ( int i = 0; i < 2; i++ ) {
				for ( int j = 0; j < 2; j++ ) {
					pointloc[ i ][ j ] = m_pGrid->Leaf( loc[ j ], i, 1 - j, n[ j ]->pass );
				}
			}

			for ( int i = 0; i < 2; i++ ) {
				for ( int j = 0; j < 2; j++ ) {
					edgeloc[ j ] = pointloc[ i ][ j ];
				}
				CreateFacePerEdge( edgeloc, ED_Along_X );
			}

			CreateFacePerPoint( pointloc );
		} else if ( dir == ED_Along_Y ) {
			CDCGrid::NodeLocator pointloc[ 2 ][ 2 ];
			CDCGrid::NodeLocator edgeloc[ 2 ];

			for ( int i = 0; i < 2; i++ ) {
				for ( int j = 0; j < 2; j++ ) {
					pointloc[ i ][ j ] = m_pGrid->Leaf( loc[ i ], 1 - i, j, n[ i ]->pass );
				}
			}

			for ( int i = 0; i < 2; i++ ) {
				for ( int j = 0; j < 2; j++ ) {
					edgeloc[ j ] = pointloc[ j ][ i ];
				}
				CreateFacePerEdge( edgeloc, ED_Along_Y );
			}

			CreateFacePerPoint( pointloc );
		}
	} else {
		int idx[ 2 ], c[ 2 ][ 2 ], s[ 2 ][ 2 ];		// c[A..B][0..1]
		if ( dir == ED_Along_X ) {
			if ( loc[ 0 ].l <= loc[ 1 ].l ) {
				idx[ 0 ] = RootCluster_NodeCornerIndex( loc[ 0 ], 0, 1 );
				idx[ 1 ] = RootCluster_NodeCornerIndex( loc[ 0 ], 1, 1 );
			} else {
				idx[ 0 ] = RootCluster_NodeCornerIndex( loc[ 1 ], 0, 0 );
				idx[ 1 ] = RootCluster_NodeCornerIndex( loc[ 1 ], 1, 0 );
			}
			c[ 0 ][ 0 ] = m_pGrid->m_vecClusterAssignment[ idx[ 0 ] ].l[ 1 ][ 0 ];
			c[ 0 ][ 1 ] = m_pGrid->m_vecClusterAssignment[ idx[ 0 ] ].l[ 1 ][ 1 ];
			c[ 1 ][ 0 ] = m_pGrid->m_vecClusterAssignment[ idx[ 1 ] ].l[ 0 ][ 0 ];
			c[ 1 ][ 1 ] = m_pGrid->m_vecClusterAssignment[ idx[ 1 ] ].l[ 0 ][ 1 ];
		} else {
			if ( loc[ 0 ].l <= loc[ 1 ].l ) {
				idx[ 0 ] = RootCluster_NodeCornerIndex( loc[ 0 ], 1, 1 );
				idx[ 1 ] = RootCluster_NodeCornerIndex( loc[ 0 ], 1, 0 );
			} else {
				idx[ 0 ] = RootCluster_NodeCornerIndex( loc[ 1 ], 0, 1 );
				idx[ 1 ] = RootCluster_NodeCornerIndex( loc[ 1 ], 0, 0 );
			}
			c[ 0 ][ 0 ] = m_pGrid->m_vecClusterAssignment[ idx[ 0 ] ].l[ 0 ][ 0 ];
			c[ 0 ][ 1 ] = m_pGrid->m_vecClusterAssignment[ idx[ 0 ] ].l[ 1 ][ 0 ];
			c[ 1 ][ 0 ] = m_pGrid->m_vecClusterAssignment[ idx[ 1 ] ].l[ 0 ][ 1 ];
			c[ 1 ][ 1 ] = m_pGrid->m_vecClusterAssignment[ idx[ 1 ] ].l[ 1 ][ 1 ];
		}

		// first find the position of point in the sequence.
		for ( int j = 0; j < 2; j++ ) {
			s[ 0 ][ j ] = s[ 1 ][ j ] = -1;
			for ( int k = 0; k < ( int )n[ j ]->seq.size(); k++ ) {
				if ( n[ j ]->seq[ k ].index == c[ 0 ][ j ] )
					s[ 0 ][ j ] = k;
				if ( n[ j ]->seq[ k ].index == c[ 1 ][ j ] )
					s[ 1 ][ j ] = k;
			}
		}

		CMesh::MeshTriangle f;

		// c00 - c10 - c11
		if ( s[ 0 ][ 0 ] > s[ 1 ][ 0 ] ) {
			for ( int i = s[ 0 ][ 0 ]; i > s[ 1 ][ 0 ]; i-- ) {
				//i - i-1 - c11
				f.i[ 0 ] = n[ 0 ]->vi + n[ 0 ]->seq[ i ].index;
				f.i[ 1 ] = n[ 0 ]->vi + n[ 0 ]->seq[ i - 1 ].index;
				f.i[ 2 ] = n[ 1 ]->vi + n[ 1 ]->seq[ s[ 1 ][ 1 ] ].index;
				PushTriangleSafe( f );
			}
		} else if ( s[ 0 ][ 0 ] < s[ 1 ][ 0 ] ) {
			for ( int i = s[ 0 ][ 0 ]; i < s[ 1 ][ 0 ]; i++ ) {
				//i - i+1 - c11
				f.i[ 0 ] = n[ 0 ]->vi + n[ 0 ]->seq[ i ].index;
				f.i[ 1 ] = n[ 0 ]->vi + n[ 0 ]->seq[ i + 1 ].index;
				f.i[ 2 ] = n[ 1 ]->vi + n[ 1 ]->seq[ s[ 1 ][ 1 ] ].index;
				PushTriangleSafe( f );
			}
		}

		// c11 - c01 - c00
		if ( s[ 1 ][ 1 ] > s[ 0 ][ 1 ] ) {
			for ( int i = s[ 1 ][ 1 ]; i > s[ 0 ][ 1 ]; i-- ) {
				//i - i-1 - c00
				f.i[ 0 ] = n[ 1 ]->vi + n[ 1 ]->seq[ i ].index;
				f.i[ 1 ] = n[ 1 ]->vi + n[ 1 ]->seq[ i - 1 ].index;
				f.i[ 2 ] = n[ 0 ]->vi + n[ 0 ]->seq[ s[ 0 ][ 0 ] ].index;
				PushTriangleSafe( f );
			}
		} else if ( s[ 1 ][ 1 ] < s[ 0 ][ 1 ] ) {
			for ( int i = s[ 1 ][ 1 ]; i < s[ 0 ][ 1 ]; i++ ) {
				//i - i+1 - c00
				f.i[ 0 ] = n[ 1 ]->vi + n[ 1 ]->seq[ i ].index;
				f.i[ 1 ] = n[ 1 ]->vi + n[ 1 ]->seq[ i + 1 ].index;
				f.i[ 2 ] = n[ 0 ]->vi + n[ 0 ]->seq[ s[ 0 ][ 0 ] ].index;
				PushTriangleSafe( f );
			}
		}
	}
}

void CDCContourer::CreateFacePerPoint( CDCGrid::NodeLocator loc[ 2 ][ 2 ] )
{
	CDCGrid::DCGridNode * n[ 2 ][ 2 ] = { { & m_pGrid->LocateNode( loc[ 0 ][ 0 ] ), & m_pGrid->LocateNode( loc[ 0 ][ 1 ] ) }, { & m_pGrid->LocateNode( loc[ 1 ][ 0 ] ), & m_pGrid->LocateNode( loc[ 1 ][ 1 ] ) } };

	if ( n[ 0 ][ 0 ]->pass == false || n[ 0 ][ 1 ]->pass == false || n[ 1 ][ 0 ]->pass == false || n[ 1 ][ 1 ]->pass == false ) {
		// not a minimum point
		CDCGrid::NodeLocator pointloc[ 2 ][ 2 ];

		for ( int i = 0; i < 2; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				pointloc[ i ][ j ] = m_pGrid->Leaf( loc[ i ][ j ], 1-i, 1-j, n[ i ][ j ]->pass );
			}
		}

		CreateFacePerPoint( pointloc );
	} else {
		int point_idx;
		int min_level = m_pGrid->m_nLevel;
		for ( int i = 0; i < 2; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				if ( loc[ i ][ j ].l < min_level ) {
					min_level = loc[ i ][ j ].l;
					point_idx = RootCluster_NodeCornerIndex( loc[ i ][ j ], 1 - i, 1 - j );
				}
			}
		}

		// we check here is [0][0]-[1][1] is connected or [0][1]-[1][0] is connected
		double c[ 2 ] = { 0.0, 0.0 };				// from beginning
		CVector3D dir[ 2 ] = {
			n[ 0 ][ 0 ]->points[ m_pGrid->m_vecClusterAssignment[ point_idx ].l[ 0 ][ 0 ] ] - n[ 1 ][ 1 ]->points[ m_pGrid->m_vecClusterAssignment[ point_idx ].l[ 1 ][ 1 ] ],
			n[ 0 ][ 1 ]->points[ m_pGrid->m_vecClusterAssignment[ point_idx ].l[ 0 ][ 1 ] ] - n[ 1 ][ 0 ]->points[ m_pGrid->m_vecClusterAssignment[ point_idx ].l[ 1 ][ 0 ] ]
		};
		dir[ 0 ].normalize();
		dir[ 1 ].normalize();
		int contribute[ 2 ][ 2 ] = {
			{ 0, 1 }, { 1, 0 }
		};

		for ( int i = 0; i < 2; i++ ) {
			for ( int j = 0; j < 2; j++ ) {
				if ( n[ i ][ j ]->mean_based_on_z == false ) {					// no boundary involved
					if ( n[ i ][ j ]->sigma2[ 2 ] < 0.01 ) {						// plane or ridge
						c[ contribute[ i ][ j ] ] += n[ i ][ j ]->sigma2[ 1 ] * abs( dir[ contribute[ i ][ j ] ] * n[ i ][ j ]->v[ 2 ] );
					}
				}
			}
		}

		int v[ 4 ] = {
			n[ 0 ][ 0 ]->vi + m_pGrid->m_vecClusterAssignment[ point_idx ].l[ 0 ][ 0 ],
			n[ 1 ][ 0 ]->vi + m_pGrid->m_vecClusterAssignment[ point_idx ].l[ 1 ][ 0 ],
			n[ 1 ][ 1 ]->vi + m_pGrid->m_vecClusterAssignment[ point_idx ].l[ 1 ][ 1 ],
			n[ 0 ][ 1 ]->vi + m_pGrid->m_vecClusterAssignment[ point_idx ].l[ 0 ][ 1 ]
		};
		CMesh::MeshTriangle f;
		if ( c[0] > c[1] ) {
			PushFace( f, v, 0, 1, 2 );
			PushFace( f, v, 0, 2, 3 );
		} else {
			PushFace( f, v, 0, 1, 3 );
			PushFace( f, v, 3, 1, 2 );
		}
	}
}
