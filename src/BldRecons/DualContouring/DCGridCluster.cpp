#include "StdAfx.h"
#include "DCGrid.h"

//////////////////////////////////////////////////////////////////////////
// algorithm-related functions
//////////////////////////////////////////////////////////////////////////

int CDCGrid::ClusterFromLeafNodes( NodeLocator & loc )
{
	DCGridNode & node = LocateNode( loc );
	int sidelength = ( 1 << loc.l );
	int leaf_sidelength = ( 1 << ( loc.l - 1 ) );

	std::vector< int > & cluster = node.cluster;
	cluster.resize( ( sidelength + 1 ) * ( sidelength + 1 ) );

	int from_leaf_cluster_num = 0;
	int from_leaf_cluster_idx_shift[ 2 ][ 2 ];
	for ( int ii = 0; ii < 2; ii++ ) {
		for ( int jj = 0; jj < 2; jj++ ) {
			DCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
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
			DCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
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

void CDCGrid::DistanceSegmentation( CVector3DPointer_Vector & points, std::vector< int > & cluster, double dis2 )
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

void CDCGrid::DistanceSegmentationEx( CVector3DPointer_Vector & points, std::vector< int > & cluster, double dis2, double dis_z )
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

void CDCGrid::InitializeCluster( std::vector< int > & cluster )
{
	for ( int i = 0; i < ( int )cluster.size(); i++ )
		cluster[ i ] = i;
}

int CDCGrid::CompressCluster( std::vector< int > & cluster )
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

int CDCGrid::CompressClusterEx( std::vector< int > & cluster, int resize_number )
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

void CDCGrid::FlattenCluster( std::vector< int > & cluster )
{
	for ( int i = 0; i < ( int )cluster.size(); i++ ) {
		// find the root for each cluster
		FindCluster( cluster, i );
	}
}

int CDCGrid::FindCluster( std::vector< int > & cluster, int start )
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

void CDCGrid::UnionCluster( std::vector< int > & cluster, int i, int j )
{
	int ri = FindCluster( cluster, i );
	int rj = FindCluster( cluster, j );
	cluster[ ri ] = rj;
}
