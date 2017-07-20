#include "StdAfx.h"
#include "MCGrid.h"

//////////////////////////////////////////////////////////////////////////
// algorithm-related functions
//////////////////////////////////////////////////////////////////////////

int CMCGrid::ClusterFromLeafNodes( NodeLocator & loc )
{
	MCGridNode & node = LocateNode( loc );
	int sidelength = ( 1 << loc.l );
	int leaf_sidelength = ( 1 << ( loc.l - 1 ) );

	std::vector< int > & cluster = node.cluster;
	cluster.resize( ( sidelength + 1 ) * ( sidelength + 1 ) );

	int from_leaf_cluster_num = 0;
	int from_leaf_cluster_idx_shift[ 2 ][ 2 ];
	for ( int ii = 0; ii < 2; ii++ ) {
		for ( int jj = 0; jj < 2; jj++ ) {
			MCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
			from_leaf_cluster_idx_shift[ ii ][ jj ] = from_leaf_cluster_num;
			from_leaf_cluster_num += leaf_node.num_of_layers;
		}
	}

	std::vector< int > from_leaf_cluster( from_leaf_cluster_num );
	InitializeCluster( from_leaf_cluster );

	for ( int ii = 0; ii < 2; ii++ ) {
		for ( int leaf_i = 0; leaf_i <= leaf_sidelength; leaf_i++ ) {
			// seal [ii, 0] and [ii, 1]
			UnionCluster(
				from_leaf_cluster,
				LocateNode( Leaf( loc, ii, 0 ) ).cluster[ Cluster_Index( leaf_i, leaf_sidelength, loc.l - 1 ) ] + from_leaf_cluster_idx_shift[ ii ][ 0 ],
				LocateNode( Leaf( loc, ii, 1 ) ).cluster[ Cluster_Index( leaf_i, 0, loc.l - 1 ) ] + from_leaf_cluster_idx_shift[ ii ][ 1 ]
			);
		}
	}
	for ( int jj = 0; jj < 2; jj++ ) {
		for ( int leaf_j = 0; leaf_j <= leaf_sidelength; leaf_j++ ) {
			// seal [0, jj] and [0, jj]
			UnionCluster(
				from_leaf_cluster,
				LocateNode( Leaf( loc, 0, jj ) ).cluster[ Cluster_Index( leaf_sidelength, leaf_j, loc.l - 1 ) ] + from_leaf_cluster_idx_shift[ 0 ][ jj ],
				LocateNode( Leaf( loc, 1, jj ) ).cluster[ Cluster_Index( 0, leaf_j, loc.l - 1 ) ] + from_leaf_cluster_idx_shift[ 1 ][ jj ]
			);
		}
	}

	int num_of_clusters = CompressCluster( from_leaf_cluster );

	for ( int ii = 0; ii < 2; ii++ ) {
		for ( int jj = 0; jj < 2; jj++ ) {
			MCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
			std::vector< int > & leaf_cluster = leaf_node.cluster;
			leaf_node.leaf_cluster_to_root_cluster.resize( leaf_node.num_of_layers );

			for ( int k = 0; k < leaf_node.num_of_layers; k++ ) {
				leaf_node.leaf_cluster_to_root_cluster[ k ] = from_leaf_cluster[ k + from_leaf_cluster_idx_shift[ ii ][ jj ] ];
			}

			for ( int leaf_i = 0; leaf_i <= leaf_sidelength; leaf_i++ ) {
				for ( int leaf_j = 0; leaf_j <= leaf_sidelength; leaf_j++ ) {
					int leaf_idx = leaf_i * ( leaf_sidelength + 1 ) + leaf_j;
					int root_idx = ( leaf_i + ii * leaf_sidelength ) * ( sidelength + 1 ) + ( leaf_j + jj * leaf_sidelength );
					cluster[ root_idx ] = leaf_node.leaf_cluster_to_root_cluster[ leaf_cluster[ leaf_idx ] ];
				}
			}
		}
	}

	return num_of_clusters;
}

void CMCGrid::DistanceSegmentation( CVector3DPointer_Vector & points, std::vector< int > & cluster, double dis2 )
{
	cluster.resize( points.size() );
	InitializeCluster( cluster );

	for ( int i = 0; i < ( int )cluster.size(); i++ ) {
		for ( int j = 0; j < i; j++ ) {
			if ( ( *( points[ i ] ) - *( points[ j ] ) ).length2() < dis2 ) {
				UnionCluster( cluster, i, j );
			}
		}
	}
}

void CMCGrid::DistanceSegmentationEx( CVector3DPointer_Vector & points, std::vector< int > & cluster, double dis2, double dis_z )
{
	cluster.resize( points.size() );
	InitializeCluster( cluster );

	for ( int i = 0; i < ( int )cluster.size(); i++ ) {
		for ( int j = 0; j < i; j++ ) {
			if ( ( HermiteDataXY_IsGround( * points[ i ] ) && HermiteDataXY_IsGround( * points[ j ] ) )
				|| ( fabs( points[ i ]->pVec[ 2 ] - points[ j ]->pVec[ 2 ] ) < dis_z && ( *( points[ i ] ) - *( points[ j ] ) ).length2() < dis2 ) )
			{
				UnionCluster( cluster, i, j );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// basic functions
//////////////////////////////////////////////////////////////////////////

void CMCGrid::InitializeCluster( std::vector< int > & cluster )
{
	for ( int i = 0; i < ( int )cluster.size(); i++ )
		cluster[ i ] = i;
}

int CMCGrid::CompressCluster( std::vector< int > & cluster )
{
	FlattenCluster( cluster );

	int k = 0;
	std::vector< int > idx( cluster.size(), -1 );
	for ( int i = 0; i < ( int )cluster.size(); i++ ) {
		if ( cluster[ i ] == i ) {
			idx[ i ] = k;
			k++;
		}
	}
	for ( int i = 0; i < ( int )cluster.size(); i++ ) {
		cluster[ i ] = idx[ cluster[ i ] ];
	}
	return k;
}

int CMCGrid::CompressClusterEx( std::vector< int > & cluster, int resize_number )
{
	FlattenCluster( cluster );

	int k = 0;
	std::vector< int > idx( cluster.size(), -1 );
	for ( int i = 0; i < ( int )resize_number; i++ ) {
		if ( idx[ cluster[ i ] ] == -1 ) {
			idx[ cluster[ i ] ] = k;
			k++;
		}
	}
	for ( int i = 0; i < ( int )resize_number; i++ ) {
		cluster[ i ] = idx[ cluster[ i ] ];
	}
	cluster.resize( resize_number );

	return k;
}

void CMCGrid::FlattenCluster( std::vector< int > & cluster )
{
	for ( int i = 0; i < ( int )cluster.size(); i++ ) {
		// find the root for each cluster
		FindCluster( cluster, i );
	}
}

int CMCGrid::FindCluster( std::vector< int > & cluster, int start )
{
	int pt = start;
	while ( cluster[ pt ] != pt )
		pt = cluster[ pt ];
	int root = pt;
	pt = start;
	while ( cluster[ pt ] != pt ) {
		int temp_pt = cluster[ pt ];
		cluster[ pt ] = root;
		pt = temp_pt;
	}
	return root;
}

void CMCGrid::UnionCluster( std::vector< int > & cluster, int i, int j )
{
	int ri = FindCluster( cluster, i );
	int rj = FindCluster( cluster, j );
	cluster[ ri ] = rj;
}

CMCGrid::MCHyperPoint * CMCGrid::HP_FindCluster( CMCGrid::MCHyperPoint * start )
{
	CMCGrid::MCHyperPoint * pt = start;
	while ( pt->parent != pt ) {
		pt = pt->parent;
	}
	return pt;
}

CMCGrid::MCHyperPoint * CMCGrid::HP_FindCluster( CMCGrid::MCHyperPoint * start, int leaf_to_root[ 2 ] )
{
	CMCGrid::MCHyperPoint * pt = start;
	while ( pt->parent != pt ) {
		leaf_to_root[ 0 ] = pt->leaf_to_parent[ leaf_to_root[ 0 ] ];
		leaf_to_root[ 1 ] = pt->leaf_to_parent[ leaf_to_root[ 1 ] ];
		pt = pt->parent;
	}
	return pt;
}

CMCGrid::MCHyperPoint * CMCGrid::HP_FindCluster_Flatten( MCHyperPoint * start )
{
	if ( start->parent == start ) {
		return start;
	} else  {
		CMCGrid::MCHyperPoint * root = HP_FindCluster_Flatten( start->parent );
		start->leaf_to_parent[ 0 ] = start->parent->leaf_to_parent[ start->leaf_to_parent[ 0 ] ];
		start->leaf_to_parent[ 1 ] = start->parent->leaf_to_parent[ start->leaf_to_parent[ 1 ] ];
		start->parent = root;
		return root;
	}
}

bool CMCGrid::HP_UnionCluster_Surface( MCHyperPoint * i, MCHyperPoint * j )
{
	CMCGrid::MCHyperPoint * ri = HP_FindCluster( i );
	CMCGrid::MCHyperPoint * rj = HP_FindCluster( j );
	
	if ( ri->Is_1HPoint() && rj->Is_1HPoint() ) {
		ri->parent = rj;
		return true;
	} else {
		return false;
	}
}

bool CMCGrid::HP_UnionCluster_Boundary( MCHyperPoint * i, MCHyperPoint * j, bool flip )
{
	int lri[ 2 ] = { 0, 1 }, lrj[ 2 ] = { 0, 1 };
	CMCGrid::MCHyperPoint * ri = HP_FindCluster( i, lri );
	CMCGrid::MCHyperPoint * rj = HP_FindCluster( j, lrj );

	if ( ri->Is_2HPoint() && rj->Is_2HPoint() ) {
		ri->parent = rj;
		bool same_assignment = ( lri[ 0 ] == lrj[ 0 ] );
		if ( ( flip && !same_assignment) || ( !flip && same_assignment ) ) {
			ri->leaf_to_parent[ 0 ] = 0;
			ri->leaf_to_parent[ 1 ] = 1;
		} else {
			ri->leaf_to_parent[ 0 ] = 1;
			ri->leaf_to_parent[ 1 ] = 0;
		}
		return true;
	} else {
		return false;
	}
}

void CMCGrid::HP_SweepSurface( MCGridNode * n[ 2 ][ 2 ] )
{
	// n[ 2 ][ 2 ] are all level0 nodes
	HP_SweepSurface( n[ 0 ][ 0 ], n[ 0 ][ 1 ] );
	HP_SweepSurface( n[ 0 ][ 0 ], n[ 1 ][ 0 ] );
	HP_SweepSurface( n[ 0 ][ 0 ], n[ 1 ][ 1 ] );
	HP_SweepSurface( n[ 0 ][ 1 ], n[ 1 ][ 0 ] );
	HP_SweepSurface( n[ 0 ][ 1 ], n[ 1 ][ 1 ] );
	HP_SweepSurface( n[ 1 ][ 0 ], n[ 1 ][ 1 ] );
}

void CMCGrid::HP_SweepSurface( MCGridNode * n1, MCGridNode * n2 )
{
	MCHyperPoint & h1 = n1->hpoints[ 0 ];
	MCHyperPoint & h2 = n2->hpoints[ 0 ];
	if ( h1.Is_1HPoint() && h2.Is_1HPoint() ) {
		HP_UnionCluster_Surface( &h1, &h2 );
	}
}

void CMCGrid::HP_SweepSurfaceJoin( MCGridNode * n[ 2 ][ 2 ], JoinElement_Set & jset )
{
	// n[ 2 ][ 2 ] are all level0 nodes
	HP_SweepSurfaceJoin( n[ 0 ][ 0 ], n[ 0 ][ 1 ], n[ 0 ][ 1 ]->cluster[ Cluster_Index( 1, 0, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 0 ][ 0 ], n[ 1 ][ 0 ], n[ 1 ][ 0 ]->cluster[ Cluster_Index( 0, 1, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 0 ][ 0 ], n[ 1 ][ 1 ], n[ 1 ][ 1 ]->cluster[ Cluster_Index( 0, 0, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 0 ][ 1 ], n[ 0 ][ 0 ], n[ 0 ][ 0 ]->cluster[ Cluster_Index( 1, 1, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 0 ][ 1 ], n[ 1 ][ 0 ], n[ 1 ][ 0 ]->cluster[ Cluster_Index( 0, 1, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 0 ][ 1 ], n[ 1 ][ 1 ], n[ 1 ][ 1 ]->cluster[ Cluster_Index( 0, 0, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 1 ][ 0 ], n[ 0 ][ 0 ], n[ 0 ][ 0 ]->cluster[ Cluster_Index( 1, 1, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 1 ][ 0 ], n[ 0 ][ 1 ], n[ 0 ][ 1 ]->cluster[ Cluster_Index( 1, 0, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 1 ][ 0 ], n[ 1 ][ 1 ], n[ 1 ][ 1 ]->cluster[ Cluster_Index( 0, 0, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 1 ][ 1 ], n[ 0 ][ 0 ], n[ 0 ][ 0 ]->cluster[ Cluster_Index( 1, 1, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 1 ][ 1 ], n[ 0 ][ 1 ], n[ 0 ][ 1 ]->cluster[ Cluster_Index( 1, 0, 0 ) ], jset );
	HP_SweepSurfaceJoin( n[ 1 ][ 1 ], n[ 1 ][ 0 ], n[ 1 ][ 0 ]->cluster[ Cluster_Index( 0, 1, 0 ) ], jset );
}

void CMCGrid::HP_SweepSurfaceJoin( MCGridNode * n1, MCGridNode * n2, int join_layer, JoinElement_Set & jset )
{
	MCHyperPoint & h1 = n1->hpoints[ 0 ];
	MCHyperPoint & h2 = n2->hpoints[ 0 ];
	if ( h1.Is_1HPoint() && h1.is_collapsible && !h1.is_ground && h2.num_of_layers >= 1 && h2.is_collapsible ) {
		int leaf_to_root[ 2 ] = { join_layer, join_layer };
		CMCGrid::MCHyperPoint * r1 = HP_FindCluster( & h1 );
		CMCGrid::MCHyperPoint * r2 = HP_FindCluster( & h2, leaf_to_root );

		if ( r1->Is_1HPoint() && r1->is_collapsible && r2->num_of_layers >= 2 && r2->is_collapsible ) {
			jset.insert( JoinElement( r1, r2, leaf_to_root ) );
		}
	}
}

void CMCGrid::HP_SweepBoundary( MCGridNode * n[ 2 ], EdgeDirection dir )
{
	MCHyperPoint & h1 = n[ 0 ]->hpoints[ 0 ];
	MCHyperPoint & h2 = n[ 1 ]->hpoints[ 0 ];

	if ( h1.Is_2HPoint() && h2.Is_2HPoint() ) {
		bool flip;
		bool enable_boundary;

		if ( dir == ED_Along_X ) {
			int c[ 2 ][ 2 ] = {
				{ n[ 0 ]->cluster[ Cluster_Index( 0, 1, 0 ) ], n[ 0 ]->cluster[ Cluster_Index( 1, 1, 0 ) ] },
				{ n[ 1 ]->cluster[ Cluster_Index( 0, 0, 0 ) ], n[ 1 ]->cluster[ Cluster_Index( 1, 0, 0 ) ] }
			};
			enable_boundary = ( c[ 0 ][ 0 ] != c[ 0 ][ 1 ] );
			flip = ( c[ 0 ][ 0 ] == c[ 1 ][ 1 ] && c[ 0 ][ 1 ] == c[ 1 ][ 0 ] );
		} else {
			int c[ 2 ][ 2 ] = {
				{ n[ 0 ]->cluster[ Cluster_Index( 1, 0, 0 ) ], n[ 0 ]->cluster[ Cluster_Index( 1, 1, 0 ) ] },
				{ n[ 1 ]->cluster[ Cluster_Index( 0, 0, 0 ) ], n[ 1 ]->cluster[ Cluster_Index( 0, 1, 0 ) ] }
			};
			enable_boundary = ( c[ 0 ][ 0 ] != c[ 0 ][ 1 ] );
			flip = ( c[ 0 ][ 0 ] == c[ 1 ][ 1 ] && c[ 0 ][ 1 ] == c[ 1 ][ 0 ] );
		}

		if ( enable_boundary ) {
			HP_UnionCluster_Boundary( &h1, &h2, flip );
		}
	}
}

void CMCGrid::HP_SweepBoundaryJoin( MCGridNode * n[ 2 ], EdgeDirection dir, JoinElement_Set & jset )
{
	int join_layer[ 2 ];
	if ( dir == ED_Along_X ) {
		int c[ 2 ][ 2 ] = {
			{ n[ 0 ]->cluster[ Cluster_Index( 0, 1, 0 ) ], n[ 0 ]->cluster[ Cluster_Index( 1, 1, 0 ) ] },
			{ n[ 1 ]->cluster[ Cluster_Index( 0, 0, 0 ) ], n[ 1 ]->cluster[ Cluster_Index( 1, 0, 0 ) ] }
		};

		if ( c[ 0 ][ 0 ] == 0 && c[ 0 ][ 1 ] == 1 ) {
			join_layer[ 0 ] = c[ 1 ][ 0 ];
			join_layer[ 1 ] = c[ 1 ][ 1 ];
			HP_SweepBoundaryJoin( n[ 0 ], n[ 1 ], join_layer, jset );
		} else if ( c[ 0 ][ 0 ] == 1 && c[ 0 ][ 1 ] == 0 ) {
			join_layer[ 0 ] = c[ 1 ][ 1 ];
			join_layer[ 1 ] = c[ 1 ][ 0 ];
			HP_SweepBoundaryJoin( n[ 0 ], n[ 1 ], join_layer, jset );
		}

		if ( c[ 1 ][ 0 ] == 0 && c[ 1 ][ 1 ] == 1 ) {
			join_layer[ 0 ] = c[ 0 ][ 0 ];
			join_layer[ 1 ] = c[ 0 ][ 1 ];
			HP_SweepBoundaryJoin( n[ 1 ], n[ 0 ], join_layer, jset );
		} else if ( c[ 1 ][ 0 ] == 1 && c[ 1 ][ 1 ] == 0 ) {
			join_layer[ 0 ] = c[ 0 ][ 1 ];
			join_layer[ 1 ] = c[ 0 ][ 0 ];
			HP_SweepBoundaryJoin( n[ 1 ], n[ 0 ], join_layer, jset );
		}
	} else {
		int c[ 2 ][ 2 ] = {
			{ n[ 0 ]->cluster[ Cluster_Index( 1, 0, 0 ) ], n[ 0 ]->cluster[ Cluster_Index( 1, 1, 0 ) ] },
			{ n[ 1 ]->cluster[ Cluster_Index( 0, 0, 0 ) ], n[ 1 ]->cluster[ Cluster_Index( 0, 1, 0 ) ] }
		};

		if ( c[ 0 ][ 0 ] == 0 && c[ 0 ][ 1 ] == 1 ) {
			join_layer[ 0 ] = c[ 1 ][ 0 ];
			join_layer[ 1 ] = c[ 1 ][ 1 ];
			HP_SweepBoundaryJoin( n[ 0 ], n[ 1 ], join_layer, jset );
		} else if ( c[ 0 ][ 0 ] == 1 && c[ 0 ][ 1 ] == 0 ) {
			join_layer[ 0 ] = c[ 1 ][ 1 ];
			join_layer[ 1 ] = c[ 1 ][ 0 ];
			HP_SweepBoundaryJoin( n[ 0 ], n[ 1 ], join_layer, jset );
		}

		if ( c[ 1 ][ 0 ] == 0 && c[ 1 ][ 1 ] == 1 ) {
			join_layer[ 0 ] = c[ 0 ][ 0 ];
			join_layer[ 1 ] = c[ 0 ][ 1 ];
			HP_SweepBoundaryJoin( n[ 1 ], n[ 0 ], join_layer, jset );
		} else if ( c[ 1 ][ 0 ] == 1 && c[ 1 ][ 1 ] == 0 ) {
			join_layer[ 0 ] = c[ 0 ][ 1 ];
			join_layer[ 1 ] = c[ 0 ][ 0 ];
			HP_SweepBoundaryJoin( n[ 1 ], n[ 0 ], join_layer, jset );
		}
	}
}

void CMCGrid::HP_SweepBoundaryJoin( MCGridNode * n1, MCGridNode * n2, int join_layer[ 2 ], JoinElement_Set & jset )
{
	MCHyperPoint & h1 = n1->hpoints[ 0 ];
	MCHyperPoint & h2 = n2->hpoints[ 0 ];
	if ( h1.Is_2HPoint() && h1.is_collapsible && h2.num_of_layers >= 2 && h2.is_collapsible ) {
		int temp_leaf_to_root[ 2 ] = { 0, 1 };
		CMCGrid::MCHyperPoint * r1 = HP_FindCluster( & h1, temp_leaf_to_root );
		if ( temp_leaf_to_root[ 0 ] == 1 ) {			// swap
			int temp = join_layer[ 0 ];
			join_layer[ 0 ] = join_layer[ 1 ];
			join_layer[ 1 ] = temp;
		}
		CMCGrid::MCHyperPoint * r2 = HP_FindCluster( & h2, join_layer );

		if ( r1->Is_2HPoint() && r1->is_collapsible && r2->num_of_layers >= 3 && r2->is_collapsible ) {
			jset.insert( JoinElement( r1, r2, join_layer ) );
		}
	}

}
