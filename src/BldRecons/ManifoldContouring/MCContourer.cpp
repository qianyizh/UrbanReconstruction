#include "StdAfx.h"
#include "MCContourer.h"

CMCContourer::CMCContourer(void)
{
}

CMCContourer::~CMCContourer(void)
{
}

//////////////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////////////

void CMCContourer::AssignMCGrid( CMCGrid *grid )
{
	m_pGrid = grid;
}

void CMCContourer::Contouring( bool wall_rectangle )
{
	m_bWallRectangle = wall_rectangle;

	CreateVertex();

	CreateFace();
}

void CMCContourer::CreateVertex()
{
	int sidenumber = ( 1 << m_pGrid->m_nLevel );
	CMCGrid::MCGridNode_Vector & nodes0 = m_pGrid->m_vecNodes[ 0 ];
	for ( int i = 0; i < sidenumber; i++ ) {
		for ( int j = 0; j < sidenumber; j++ ) {
			CMCGrid::MCGridNode & node = nodes0[ i * sidenumber + j ];
			CMCGrid::MCHyperPoint & hpoint = node.hpoints[ 0 ];
			hpoint.vi = -1;
			if ( hpoint.is_feature ) {
				CreateVertex_HPoint( hpoint );
			} else if ( ( hpoint.parent == & hpoint || hpoint.parent->is_collapsible == false ) && hpoint.is_ground == false ) {
				CreateVertex_HPoint( hpoint );
			}
		}
	}

	for ( int l = 1; l <= m_pGrid->m_nLevel; l++ ) {
		sidenumber = ( 1 << ( m_pGrid->m_nLevel - l ) );
		CMCGrid::MCGridNode_Vector & noden = m_pGrid->m_vecNodes[ l ];
		for ( int i = 0; i < sidenumber; i++ ) {
			for ( int j = 0; j < sidenumber; j++ ) {
				CMCGrid::MCGridNode & node = noden[ i * sidenumber + j ];
				for ( int k = 0; k < ( int )node.hpoints.size(); k++ ) {
					CMCGrid::MCHyperPoint & hpoint = node.hpoints[ k ];
					hpoint.vi = -1;
					if ( hpoint.is_collapsible == true && ( hpoint.parent == & hpoint || hpoint.parent->is_collapsible == false ) && hpoint.is_ground == false ) {
						CreateVertex_HPoint( hpoint );
					}
				}
			}
		}
	}
}

void CMCContourer::CreateVertex_HPoint( CMCGrid::MCHyperPoint & hpoint )
{
	hpoint.vi = ( int )m_cMesh.m_vecVertex.size();
	for ( int i = 0; i < ( int )hpoint.num_of_layers; i++ ) {
		m_cMesh.m_vecVertex.push_back( hpoint.points[ i ] );
		m_cBoundary.m_vecGroupInfo.push_back( CMeshBoundary::CVertexGroupInfo( hpoint.num_of_layers, i ) );
	}

	if ( ! m_bWallRectangle ) {
		for ( int i = 0; i < hpoint.num_of_layers; i++ ) {
			hpoint.seq.push_back( CMCGrid::DistanceSorting( i, hpoint.points[ i ][ 2 ] ) );
		}
		m_pGrid->QSort( hpoint.seq );
	}
}

void CMCContourer::CreateFace()
{
	CreateSurfaceFace();
	CreateBoundaryFace();
}

void CMCContourer::CreateSurfaceFace()
{
	int sidelength = 1;
	int sidenumber = ( 1 << m_pGrid->m_nLevel );

	for ( int i = 0; i < sidenumber - 1; i++ ) {
		for ( int j = 0; j < sidenumber - 1; j++ ) {
			CMCGrid::MCGridNode * n[ 2 ][ 2 ] = {
				{ & LocateNode( 0, i, j ), & LocateNode( 0, i, j + 1 ) },
				{ & LocateNode( 0, i + 1, j ), & LocateNode( 0, i + 1, j + 1 ) },
			};
			CreateSurfaceFace( n );
		}
	}
}

void CMCContourer::CreateSurfaceFace( CMCGrid::MCGridNode * n[ 2 ][ 2 ] )
{
	int vi[ 2 ][ 2 ];
	vi[ 0 ][ 0 ] = GetVI_HPoint( & n[ 0 ][ 0 ]->hpoints[ 0 ], n[ 0 ][ 0 ]->cluster[ m_pGrid->Cluster_Index( 1, 1, 0 ) ] );
	vi[ 0 ][ 1 ] = GetVI_HPoint( & n[ 0 ][ 1 ]->hpoints[ 0 ], n[ 0 ][ 1 ]->cluster[ m_pGrid->Cluster_Index( 1, 0, 0 ) ] );
	vi[ 1 ][ 0 ] = GetVI_HPoint( & n[ 1 ][ 0 ]->hpoints[ 0 ], n[ 1 ][ 0 ]->cluster[ m_pGrid->Cluster_Index( 0, 1, 0 ) ] );
	vi[ 1 ][ 1 ] = GetVI_HPoint( & n[ 1 ][ 1 ]->hpoints[ 0 ], n[ 1 ][ 1 ]->cluster[ m_pGrid->Cluster_Index( 0, 0, 0 ) ] );

	if ( vi[ 0 ][ 0 ] == -1 || vi[ 0 ][ 1 ] == -1 || vi[ 1 ][ 0 ] == -1 || vi[ 1 ][ 1 ] == -1 || vi[ 0 ][ 0 ] == vi[ 1 ][ 1 ] || vi[ 0 ][ 1 ] == vi[ 1 ][ 0 ] )
		return;

	double c[ 2 ] = { 0.0, 0.0 };				// from beginning
	CVector3D dir[ 2 ] = {
		m_cMesh.m_vecVertex[ vi[ 0 ][ 0 ] ].v - m_cMesh.m_vecVertex[ vi[ 1 ][ 1 ] ].v,
		m_cMesh.m_vecVertex[ vi[ 0 ][ 1 ] ].v - m_cMesh.m_vecVertex[ vi[ 1 ][ 0 ] ].v
	};
	dir[ 0 ].normalize();
	dir[ 1 ].normalize();

	Vote( & n[ 0 ][ 0 ]->hpoints[ 0 ], dir, c );
	Vote( & n[ 0 ][ 1 ]->hpoints[ 0 ], dir, c );
	Vote( & n[ 1 ][ 0 ]->hpoints[ 0 ], dir, c );
	Vote( & n[ 1 ][ 1 ]->hpoints[ 0 ], dir, c );

	if ( c[ 0 ] > c[ 1 ] ) {
		PushFaceWithoutGround( vi[ 0 ][ 0 ], vi[ 1 ][ 0 ], vi[ 1 ][ 1 ] );
		PushFaceWithoutGround( vi[ 1 ][ 1 ], vi[ 0 ][ 1 ], vi[ 0 ][ 0 ] );
	} else {
		PushFaceWithoutGround( vi[ 0 ][ 0 ], vi[ 1 ][ 0 ], vi[ 0 ][ 1 ] );
		PushFaceWithoutGround( vi[ 0 ][ 1 ], vi[ 1 ][ 0 ], vi[ 1 ][ 1 ] );
	}
}

void CMCContourer::CreateBoundaryFace()
{
	int sidelength = 1;
	int sidenumber = ( 1 << m_pGrid->m_nLevel );

	for ( int i = 0; i < sidenumber; i++ ) {
		for ( int j = 0; j < sidenumber - 1; j++ ) {
			CMCGrid::MCGridNode * n[ 2 ] = { & LocateNode( 0, i, j ), & LocateNode( 0, i, j + 1 ) };
			CreateBoundaryFace( n, ED_Along_X );
		}
	}

	for ( int i = 0; i < sidenumber - 1; i++ ) {
		for ( int j = 0; j < sidenumber; j++ ) {
			CMCGrid::MCGridNode * n[ 2 ] = { & LocateNode( 0, i, j ), & LocateNode( 0, i + 1, j ) };
			CreateBoundaryFace( n, ED_Along_Y );
		}
	}
}

void CMCContourer::CreateBoundaryFace( CMCGrid::MCGridNode * n[ 2 ], EdgeDirection dir )
{
	int c[ 2 ][ 2 ], s[ 2 ][ 2 ];		// c[A..B][0..1]
	CMCGrid::MCHyperPoint * h[ 2 ];
	if ( dir == ED_Along_X ) {
		int layer0[ 2 ] = { n[ 0 ]->cluster[ m_pGrid->Cluster_Index( 0, 1, 0 ) ], n[ 0 ]->cluster[ m_pGrid->Cluster_Index( 1, 1, 0 ) ] };
		h[ 0 ] = GetCluster_HPoint( & n[ 0 ]->hpoints[ 0 ], layer0 );
		c[ 0 ][ 0 ] = layer0[ 0 ];
		c[ 1 ][ 0 ] = layer0[ 1 ];
		int layer1[ 2 ] = { n[ 1 ]->cluster[ m_pGrid->Cluster_Index( 0, 0, 0 ) ], n[ 1 ]->cluster[ m_pGrid->Cluster_Index( 1, 0, 0 ) ] };
		h[ 1 ] = GetCluster_HPoint( & n[ 1 ]->hpoints[ 0 ], layer1 );
		c[ 0 ][ 1 ] = layer1[ 0 ];
		c[ 1 ][ 1 ] = layer1[ 1 ];
	} else {
		int layer0[ 2 ] = { n[ 0 ]->cluster[ m_pGrid->Cluster_Index( 1, 1, 0 ) ], n[ 0 ]->cluster[ m_pGrid->Cluster_Index( 1, 0, 0 ) ] };
		h[ 0 ] = GetCluster_HPoint( & n[ 0 ]->hpoints[ 0 ], layer0 );
		c[ 0 ][ 0 ] = layer0[ 0 ];
		c[ 1 ][ 0 ] = layer0[ 1 ];
		int layer1[ 2 ] = { n[ 1 ]->cluster[ m_pGrid->Cluster_Index( 0, 1, 0 ) ], n[ 1 ]->cluster[ m_pGrid->Cluster_Index( 0, 0, 0 ) ] };
		h[ 1 ] = GetCluster_HPoint( & n[ 1 ]->hpoints[ 0 ], layer1 );
		c[ 0 ][ 1 ] = layer1[ 0 ];
		c[ 1 ][ 1 ] = layer1[ 1 ];
	}

	if ( h[ 0 ] == h[ 1 ] || h[ 0 ]->vi == -1 || h[ 1 ]->vi == -1 ) {
		return;
	}

	if ( m_bWallRectangle == false ) {
		// first find the position of point in the sequence.
		for ( int j = 0; j < 2; j++ ) {
			s[ 0 ][ j ] = s[ 1 ][ j ] = -1;
			for ( int k = 0; k < ( int )h[ j ]->seq.size(); k++ ) {
				if ( h[ j ]->seq[ k ].index == c[ 0 ][ j ] )
					s[ 0 ][ j ] = k;
				if ( h[ j ]->seq[ k ].index == c[ 1 ][ j ] )
					s[ 1 ][ j ] = k;
			}
		}

		CMesh::MeshTriangle f;

		// c00 - c10 - c11
		if ( s[ 0 ][ 0 ] > s[ 1 ][ 0 ] ) {
			for ( int i = s[ 0 ][ 0 ]; i > s[ 1 ][ 0 ]; i-- ) {
				//i - i-1 - c11
				f.i[ 0 ] = h[ 0 ]->vi + h[ 0 ]->seq[ i ].index;
				f.i[ 1 ] = h[ 0 ]->vi + h[ 0 ]->seq[ i - 1 ].index;
				f.i[ 2 ] = h[ 1 ]->vi + h[ 1 ]->seq[ s[ 1 ][ 1 ] ].index;
				PushTriangleSafe( f );
			}
		} else if ( s[ 0 ][ 0 ] < s[ 1 ][ 0 ] ) {
			for ( int i = s[ 0 ][ 0 ]; i < s[ 1 ][ 0 ]; i++ ) {
				//i - i+1 - c11
				f.i[ 0 ] = h[ 0 ]->vi + h[ 0 ]->seq[ i ].index;
				f.i[ 1 ] = h[ 0 ]->vi + h[ 0 ]->seq[ i + 1 ].index;
				f.i[ 2 ] = h[ 1 ]->vi + h[ 1 ]->seq[ s[ 1 ][ 1 ] ].index;
				PushTriangleSafe( f );
			}
		}

		// c11 - c01 - c00
		if ( s[ 1 ][ 1 ] > s[ 0 ][ 1 ] ) {
			for ( int i = s[ 1 ][ 1 ]; i > s[ 0 ][ 1 ]; i-- ) {
				//i - i-1 - c00
				f.i[ 0 ] = h[ 1 ]->vi + h[ 1 ]->seq[ i ].index;
				f.i[ 1 ] = h[ 1 ]->vi + h[ 1 ]->seq[ i - 1 ].index;
				f.i[ 2 ] = h[ 0 ]->vi + h[ 0 ]->seq[ s[ 0 ][ 0 ] ].index;
				PushTriangleSafe( f );
			}
		} else if ( s[ 1 ][ 1 ] < s[ 0 ][ 1 ] ) {
			for ( int i = s[ 1 ][ 1 ]; i < s[ 0 ][ 1 ]; i++ ) {
				//i - i+1 - c00
				f.i[ 0 ] = h[ 1 ]->vi + h[ 1 ]->seq[ i ].index;
				f.i[ 1 ] = h[ 1 ]->vi + h[ 1 ]->seq[ i + 1 ].index;
				f.i[ 2 ] = h[ 0 ]->vi + h[ 0 ]->seq[ s[ 0 ][ 0 ] ].index;
				PushTriangleSafe( f );
			}
		}
	} else {
		if ( c[ 0 ][ 0 ] != c[ 1 ][ 0 ] && c[ 0 ][ 1 ] != c[ 1 ][ 1 ] ) {
			CMesh::MeshQuad q;
			q.i[ 0 ] = h[ 0 ]->vi + c[ 0 ][ 0 ];
			q.i[ 1 ] = h[ 0 ]->vi + c[ 1 ][ 0 ];
			q.i[ 2 ] = h[ 1 ]->vi + c[ 1 ][ 1 ];
			q.i[ 3 ] = h[ 1 ]->vi + c[ 0 ][ 1 ];
			PushQuadSafe( q );
		} else if ( c[ 0 ][ 0 ] != c[ 1 ][ 0 ] ) {
			//CMesh::MeshTriangle f;
			//f.i[ 0 ] = h[ 0 ]->vi + c[ 0 ][ 0 ];
			//f.i[ 1 ] = h[ 0 ]->vi + c[ 1 ][ 0 ];
			//f.i[ 2 ] = h[ 1 ]->vi + c[ 1 ][ 1 ];
			//PushTriangleSafe( f );
			CMesh::MeshQuad q;
			q.i[ 0 ] = h[ 1 ]->vi + c[ 1 ][ 1 ];
			q.i[ 1 ] = h[ 1 ]->vi + c[ 0 ][ 1 ];
			q.i[ 2 ] = h[ 0 ]->vi + c[ 0 ][ 0 ];
			q.i[ 3 ] = h[ 0 ]->vi + c[ 1 ][ 0 ];
			PushQuadSafe( q );
		} else if ( c[ 0 ][ 1 ] != c[ 1 ][ 1 ] ) {
			//CMesh::MeshTriangle f;
			//f.i[ 0 ] = h[ 0 ]->vi + c[ 0 ][ 0 ];
			//f.i[ 1 ] = h[ 1 ]->vi + c[ 1 ][ 1 ];
			//f.i[ 2 ] = h[ 1 ]->vi + c[ 0 ][ 1 ];
			//PushTriangleSafe( f );
			CMesh::MeshQuad q;
			q.i[ 0 ] = h[ 0 ]->vi + c[ 0 ][ 0 ];
			q.i[ 1 ] = h[ 0 ]->vi + c[ 1 ][ 0 ];
			q.i[ 2 ] = h[ 1 ]->vi + c[ 1 ][ 1 ];
			q.i[ 3 ] = h[ 1 ]->vi + c[ 0 ][ 1 ];
			PushQuadSafe( q );
		}
	}
}
