#include "StdAfx.h"
#include "DCGrid.h"
#include "nr\nr.h"
#include "nr\nrutil.h"

void CDCGrid::BuildQuadTreeAtLevel0()
{
	DCGridNode_Vector & nodes = m_vecNodes[ 0 ];
	int sidelength = 1;
	int sidenumber = ( 1 << m_nLevel );
	nodes.clear();
	nodes.resize( sidenumber * sidenumber );

	const int MAX_MATRIX_SIZE = 8;
	int sing;
	float **a, *b, *c, *d;
	a = matrix( 1, MAX_MATRIX_SIZE, 1, MAX_MATRIX_SIZE );
	b = vector( 1, MAX_MATRIX_SIZE );
	c = vector( 1, MAX_MATRIX_SIZE );
	d = vector( 1, MAX_MATRIX_SIZE );

	for ( int i = 0; i < sidenumber; i++ ) {
		for ( int j = 0; j < sidenumber; j++ ) {
			// node[i,j], range from (i * sidelength, j * sidelength) to ((i + 1) * sidelength - 1, (j + 1) * sidelength - 1)
			// node[i,j], range from (i << l, j << l) to ((i + 1) << l - 1, (j + 1) << l - 1)
			DCGridNode & node = nodes[ i * sidenumber + j ];
			node.level = 0;
			node.pass = true;
			node.is_ground = true;

			// step 1, split layers
			CVector3DPointer_Vector points;
			for ( int ii = 0; ii <= 1; ii++ ) {
				for ( int jj = 0; jj <= 1; jj++ ) {
					points.push_back( &( m_vecHermiteDataXY[ HDIndex( ii + i, jj + j ) ].v ) );
					if ( HermiteDataXY_IsGround( m_vecHermiteDataXY[ HDIndex( ii + i, jj + j ) ].v ) == false )
						node.is_ground = false;
				}
			}
			if ( IIsValid( i + 1 ) && JIsValid( j + 1 ) ) {
				Grid_Index & grid_index = m_vecIndex[ Index( i + 1, j + 1 ) ];
				for ( int k = 0; k < ( int )grid_index.size(); k++ ) {
					points.push_back( &( m_pPointCloud->m_vecPoint[ grid_index[ k ] ] ) );
				}
			}

			DistanceSegmentationEx( points, node.cluster, m_dbSegmentationDistance * m_dbSegmentationDistance, m_dbSegmentationZ );
			node.num_of_layers = CompressClusterEx( node.cluster, 4 );
			node.points.resize( node.num_of_layers );

			// step 2, build indexing structure
			HermiteDataIndex_Vector xy;			// xy Hermite data index information
			HermiteDataIndex_Vector z;			// z Hermite data index information

			for ( int ii = 0; ii <= 1; ii++ ) {
				for ( int jj = 0; jj <= 1; jj++ ) {
					xy.push_back( HermiteDataIndex( HDIndex( ii + i, jj + j ), node.cluster[ ii * 2 + jj ] ) );
				}
			}

			for ( int ii = 0; ii <= 1; ii++ ) {
				for ( int jj = 0; jj < 1; jj++ ) {
					int idx = HDIndex( ii + i, jj + j ) * 2;
					if ( ! HermiteDataZ_IsInvalid( m_vecHermiteDataZ[ idx ].v ) ) {
						z.push_back( HermiteDataIndex( idx, -1 ) );
					}
				}
			}
			for ( int ii = 0; ii < 1; ii++ ) {
				for ( int jj = 0; jj <= 1; jj++ ) {
					int idx = HDIndex( ii + i, jj + j ) * 2 + 1;
					if ( ! HermiteDataZ_IsInvalid( m_vecHermiteDataZ[ idx ].v ) ) {
						z.push_back( HermiteDataIndex( idx, -1 ) );
					}
				}
			}

			// step 3, get means
			node.means.resize( node.num_of_layers );
			CVector3D tempxy( 0.0, 0.0, 0.0 );
			if ( z.size() != 0 ) {
				for ( int l = 0; l < ( int )z.size(); l++ ) {
					tempxy += m_vecHermiteDataZ[ z[ l ].i ].v;
				}
				tempxy /= ( double )z.size();
				node.mean_based_on_z = true;
			} else {
				for ( int l = 0; l < ( int )xy.size(); l++ ) {
					tempxy += m_vecHermiteDataXY[ xy[ l ].i ].v;
				}
				tempxy /= ( double )xy.size();
				node.mean_based_on_z = false;
			}
			std::vector< int > num_of_cluster( node.num_of_layers, 0 );
			for ( int l = 0; l < node.num_of_layers; l++ ) {
				node.means[ l ] = tempxy;
				node.means[ l ][ 2 ] = 0.0;
				num_of_cluster[ l ] = 0;
			}
			for ( int l = 0; l < ( int )xy.size(); l++ ) {
				int c = xy[ l ].l;
				node.means[ c ][ 2 ] += m_vecHermiteDataXY[ xy[ l ].i ].v[ 2 ];
				num_of_cluster[ c ]++;
			}
			for ( int l = 0; l < node.num_of_layers; l++ ) {
				node.means[ l ][ 2 ] /= ( double )num_of_cluster[ l ];
			}
			for ( int l = 0; l < node.num_of_layers; l++ ) {
				node.means[ l ] = AbsoluteToRelative( node.means[ l ] );
			}

			// step 4, compute R
			int matrix_width = node.num_of_layers + 3;
			int matrix_height = ( int )xy.size() + ( int )z.size();
			int matrix_size = matrix_height > matrix_width ? matrix_height : matrix_width;

			for ( int ii = 0; ii < matrix_size; ii++ )
				for ( int jj = 0; jj < matrix_size; jj++ )
					a[ ii + 1 ][ jj + 1 ] = 0.0f;

			for ( int ii = 0; ii < ( int )xy.size(); ii++ ) {
				HermiteData & hd = m_vecHermiteDataXY[ xy[ ii ].i ];
				a[ ii + 1 ][ 1 ] = ( float )hd.n[ 0 ];
				a[ ii + 1 ][ 2 ] = ( float )hd.n[ 1 ];
				a[ ii + 1 ][ 3 + xy[ ii ].l ] = ( float )hd.n[ 2 ];
				a[ ii + 1 ][ matrix_width ] = ( float )( AbsoluteToRelative( hd.v ) * hd.n );
			}
			for ( int ii = 0; ii < ( int )z.size(); ii++ ) {
				HermiteData & hd = m_vecHermiteDataZ[ z[ ii ].i ];
				a[ ii + ( int )xy.size() + 1 ][ 1 ] = ( float )( hd.n[ 0 ] * m_dbBoundaryWeight );
				a[ ii + ( int )xy.size() + 1 ][ 2 ] = ( float )( hd.n[ 1 ] * m_dbBoundaryWeight );
				a[ ii + ( int )xy.size() + 1 ][ matrix_width ] = ( float )( AbsoluteToRelative( hd.v ) * hd.n * m_dbBoundaryWeight );
			}

			qrdcmp_ext( a, matrix_size, c, d, &sing );

			int k = 0;
			for ( int ii = 0; ii < matrix_width; ii++ ) {
				node.r.push_back( d[ ii + 1 ] );
				for ( int jj = ii+1; jj < matrix_width; jj++ ) {
					node.r.push_back( a[ ii + 1 ][ jj + 1 ] );
				}
			}

			// step 5, estimate geometry in this node
			DualContouringGeometryNode( node );
			// at level 0, we force every node pass the error test
			node.pass = true;
		}
	}

	free_matrix( a, 1, MAX_MATRIX_SIZE, 1, MAX_MATRIX_SIZE );
	free_vector( b, 1, MAX_MATRIX_SIZE );
	free_vector( c, 1, MAX_MATRIX_SIZE );
	free_vector( d, 1, MAX_MATRIX_SIZE );
}

void CDCGrid::BuildQuadTreeAtLevelN( int l )
{
	DCGridNode_Vector & nodes = m_vecNodes[ l ];
	int sidelength = ( 1 << l );
	int sidenumber = ( 1 << ( m_nLevel - l ) );
	nodes.clear();
	nodes.resize( sidenumber * sidenumber );

	for ( int i = 0; i < sidenumber; i++ ) {
		for ( int j = 0; j < sidenumber; j++ ) {
			NodeLocator loc( l, i, j );
			DCGridNode & node = LocateNode( loc );
			node.level = l;
			node.is_ground = true;

			// step 0, check validity and leaves' pass
			node.pass = true;
			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					DCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					if ( leaf_node.pass == false )
						node.pass = false;
					if ( leaf_node.is_ground == false )
						node.is_ground = false;
				}
			}
			if ( node.pass == false ) {
				continue;
			}

			// step 1, split layers, build indexing structures
			node.num_of_layers = ClusterFromLeafNodes( loc );
			node.points.resize( node.num_of_layers );

			// step 2, topology check
			if ( TopologyCheck( loc ) == false ) {
				node.pass = false;
				continue;
			}

			// step 3, get means
			node.means.resize( node.num_of_layers );
			node.mean_based_on_z = false;
			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					DCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					if ( leaf_node.mean_based_on_z ) {
						node.mean_based_on_z = true;
					}
				}
			}

			CVector3D tempxy( 0, 0, 0 );
			int kk = 0;

			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					DCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					if ( ! node.mean_based_on_z || leaf_node.mean_based_on_z ) {
						tempxy += leaf_node.means[ 0 ];
						kk++;
					}
				}
			}
			tempxy /= ( double )kk;

			std::vector< int > num_of_cluster( node.num_of_layers, 0 );
			for ( int layer = 0; layer < node.num_of_layers; layer++ ) {
				node.means[ layer ] = tempxy;
				node.means[ layer ][ 2 ] = 0.0;
				num_of_cluster[ layer ] = 0;
			}

			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					DCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					for ( int k = 0; k < node.num_of_layers; k++ ) {
						double tempz = 0;
						int tempk = 0;
						for ( int layer = 0; layer < leaf_node.num_of_layers; layer++ ) {
							if ( leaf_node.leaf_cluster_to_root_cluster[ layer ] == k ) {
								tempz += leaf_node.means[ layer ][ 2 ];
								tempk++;
							}
						}
						if ( tempk > 0 ) {
							tempz /= ( double )tempk;
							node.means[ k ][ 2 ] += tempz;
							num_of_cluster[ k ]++;
						}
					}
				}
			}

			for ( int layer = 0; layer < node.num_of_layers; layer++ ) {
				node.means[ layer ][ 2 ] /= ( double )num_of_cluster[ layer ];
			}

			// step 4, compute R
			int matrix_width = node.num_of_layers + 3;
			int matrix_height = 0;
			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					DCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					if ( node.is_ground || ! leaf_node.is_ground )
						matrix_height += ( leaf_node.num_of_layers + 3 );
				}
			}
			int matrix_size = matrix_height > matrix_width ? matrix_height : matrix_width;

			int sing;
			float **a, **temp, *b, *c, *d;
			a = matrix( 1, matrix_size, 1, matrix_size );
			temp = matrix( 1, matrix_size, 1, matrix_size );
			b = vector( 1, matrix_size );
			c = vector( 1, matrix_size );
			d = vector( 1, matrix_size );

			for ( int ii = 0; ii < matrix_size; ii++ )
				for ( int jj = 0; jj < matrix_size; jj++ )
					a[ ii + 1 ][ jj + 1 ] = 0.0f;

			int k = 0;
			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					DCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					if ( node.is_ground || ! leaf_node.is_ground ) {
						int kkk = 0;
						for ( int iii = 1; iii <= leaf_node.num_of_layers + 3; iii++ ) {
							for ( int jjj = 1; jjj < iii; jjj++ )
								temp[ iii ][ jjj ] = 0.0f;
							for ( int jjj = iii; jjj <= leaf_node.num_of_layers + 3; jjj++ ) {
								temp[ iii ][ jjj ] = leaf_node.r[ kkk ];
								kkk++;
							}
						}

						for ( int iii = 0; iii < leaf_node.num_of_layers + 3; iii++ ) {
							a[ k + iii + 1 ][ 1 ] = temp[ iii + 1 ][ 1 ];
							a[ k + iii + 1 ][ 2 ] = temp[ iii + 1 ][ 2 ];
							for ( int jjj = 0; jjj < leaf_node.num_of_layers; jjj++ ) {
								a[ k + iii + 1 ][ leaf_node.leaf_cluster_to_root_cluster[ jjj ] + 3 ] = temp[ iii + 1 ][ jjj + 3 ];
							}
							a[ k + iii + 1 ][ node.num_of_layers + 3 ] = temp[ iii + 1 ][ leaf_node.num_of_layers + 3 ];
						}

						k += ( leaf_node.num_of_layers + 3 );
					}
				}
			}

			qrdcmp_ext( a, matrix_size, c, d, &sing );

			k = 0;
			for ( int ii = 0; ii < matrix_width; ii++ ) {
				node.r.push_back( d[ ii + 1 ] );
				for ( int jj = ii+1; jj < matrix_width; jj++ ) {
					node.r.push_back( a[ ii + 1 ][ jj + 1 ] );
				}
			}

			free_matrix( a, 1, matrix_size, 1, matrix_size );
			free_matrix( temp, 1, matrix_width, 1, matrix_width );
			free_vector( b, 1, matrix_size );
			free_vector( c, 1, matrix_size );
			free_vector( d, 1, matrix_size );

			// step 5, estimate geometry in this node
			DualContouringGeometryNode( node );
		}
	}
}

void CDCGrid::DualContouringGeometryNode( DCGridNode & node )
{
	int matrix_width = node.num_of_layers + 3;

	float **aa, *bb, rr;
	aa = matrix( 1, matrix_width - 1, 1, matrix_width - 1 );
	bb = vector( 1, matrix_width - 1);
	float **u, **v, *w;
	u = matrix( 1, matrix_width - 1, 1, matrix_width - 1 );
	v = matrix( 1, matrix_width - 1, 1, matrix_width - 1 );
	w = vector( 1, matrix_width - 1 );
	float *xx, *b_Ax;
	xx = vector( 1, matrix_width - 1 );
	b_Ax = vector( 1, matrix_width - 1 );

	int k = 0;
	for ( int i = 1; i <= matrix_width - 1; i++ ) {
		for ( int j = 1; j < i; j++ )
			u[ i ][ j ] = aa[ i ][ j ] = 0.0f;
		for ( int j = i; j <= matrix_width - 1; j++ ) {
			u[ i ][ j ] = aa[ i ][ j ] = node.r[ k ];
			k++;
		}
		bb[ i ] = node.r[ k ];
		k++;
	}
	rr = node.r[ k ];

	xx[ 1 ] = ( float )node.means[ 0 ][ 0 ];
	xx[ 2 ] = ( float )node.means[ 0 ][ 1 ];
	for ( int i = 0; i < ( int )node.means.size(); i++ )
		xx[ i + 3 ] = ( float )node.means[ i ][ 2 ];

	svdcmp_ext( u, matrix_width - 1, matrix_width - 1, w, v );

	// sort the matrix eigenvalues and eigenvectors
	if ( node.mean_based_on_z == false ) {
		int num = 4 * ( 1 << ( 2 * node.level ) );
		for ( int i = 1; i <= 3; i++ ) {
			node.sigma2[ i - 1 ] = w[ i ] * w[ i ] / ( double )num;
			node.v[ i - 1 ] = CVector3D( v[ 1 ][ i ], v[ 2 ][ i ], v[ 3 ][ i ] );
		}
		// sort
		for ( int i = 0; i < 3; i++ ) {
			for ( int j = i + 1; j < 3; j++ ) {
				if ( node.sigma2[ i ] < node.sigma2[ j ] ) {
					double temp_sigma2 = node.sigma2[ i ];
					node.sigma2[ i ] = node.sigma2[ j ];
					node.sigma2[ j ] = temp_sigma2;
					CVector3D temp_v = node.v[ i ];
					node.v[ i ] = node.v[ j ];
					node.v[ j ] = temp_v;
				}
			}
		}
	}

	// handling singularity
	float max_singular = 0.0;
	for ( int i = 1; i <= matrix_width - 1; i++ )
		if ( w[ i ] > max_singular )
			max_singular = w[ i ];
	if ( max_singular > 0.0f ) {
		for ( int i = 1; i <= matrix_width - 1; i++ ) {
			if ( w[ i ] / max_singular > m_dbSingularTolerance ) {
				w[ i ] = 1.0f / w[ i ];
			} else {
				w[ i ] = 0.0f;
			}
		}
	}

	// compute the solution
	// v = mean + V W^{+} U^{T} ( b - A * mean )
	for ( int i = 1; i <= matrix_width - 1; i++ ) {
		b_Ax[ i ] = bb[ i ];
		for ( int j = 1; j <= matrix_width - 1; j++ ) {
			b_Ax[ i ] -= aa[ i ][ j ] * xx[ j ];
		}
	}
	for ( int i = 1; i <= matrix_width - 1; i++ ) {
		for ( int j = 1; j <= matrix_width - 1; j++ ) {
			for ( int k = 1; k <= matrix_width - 1; k++ ) {
				xx[ i ] += v[ i ][ j ] * w[ j ] * u[ k ][ j ] * b_Ax[ k ];
			}
		}
	}

	for ( int i = 0; i < node.num_of_layers; i++ ) {
		node.points[ i ] = CVector3D( xx[ 1 ], xx[ 2 ], xx[ i + 3 ] );
		node.points[ i ] = RelativeToAbsolute( node.points[ i ] );
	}

	node.error = rr * rr;
	node.pass = ( rr * rr < m_dbErrorTolerance * m_dbErrorTolerance );

	// clean up
	free_matrix( aa, 1, matrix_width - 1, 1, matrix_width - 1 );
	free_vector( bb, 1, matrix_width - 1 );
	free_matrix( u, 1, matrix_width - 1, 1, matrix_width - 1 );
	free_matrix( v, 1, matrix_width - 1, 1, matrix_width - 1 );
	free_vector( w, 1, matrix_width - 1 );
	free_vector( xx, 1, matrix_width - 1 );
	free_vector( b_Ax, 1, matrix_width - 1 );
}

bool CDCGrid::TopologyCheck( NodeLocator & loc )
{
	if ( m_bTopologyCheck == false )
		return true;

	DCGridNode & node = LocateNode( loc );
	int sidelength = ( 1 << loc.l );
	int leaf_sidelength = ( 1 << ( loc.l - 1 ) );

	// check #1 : if the parent node is manifold
	int cluster[ 3 ][ 3 ];

	for ( int iii = 0; iii < 3; iii += 1 ) {
		for ( int jjj = 0; jjj < 3; jjj += 1 ) {
			cluster[ iii ][ jjj ] = node.cluster[ iii * leaf_sidelength * ( sidelength + 1 ) + jjj * leaf_sidelength ];
		}
	}
	if ( cluster[ 0 ][ 0 ] == cluster[ 2 ][ 2 ] && cluster[ 0 ][ 0 ] != cluster[ 0 ][ 2 ] && cluster[ 0 ][ 0 ] != cluster[ 2 ][ 0 ] ) {
		// non-manifold, reject #1
		return false;
	}
	if ( cluster[ 0 ][ 2 ] == cluster[ 2 ][ 0 ] && cluster[ 0 ][ 2 ] != cluster[ 0 ][ 0 ] && cluster[ 0 ][ 2 ] != cluster[ 2 ][ 2 ] ) {
		// non-manifold, reject #1
		return false;
	}

	// check #2 : if all the leaf nodes are manifold
	// check #3 : if two clusters are merged into one cluster
	for ( int ii = 0; ii < 2; ii++ ) {
		for ( int jj = 0; jj < 2; jj++ ) {
			DCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );

			int leaf_cluster[ 2 ][ 2 ];
			for ( int iii = 0; iii < 2; iii++ ) {
				for ( int jjj = 0; jjj < 2; jjj++ ) {
					leaf_cluster[ iii ][ jjj ] = leaf_node.cluster[ iii * leaf_sidelength * ( leaf_sidelength + 1 ) + jjj * leaf_sidelength ];
				}
			}
			if ( leaf_cluster[ 0 ][ 0 ] == leaf_cluster[ 1 ][ 1 ] && leaf_cluster[ 0 ][ 0 ] != leaf_cluster[ 0 ][ 1 ] && leaf_cluster[ 0 ][ 0 ] != leaf_cluster[ 1 ][ 0 ] ) {
				// non-manifold, reject #2
				return false;
			}
			if ( leaf_cluster[ 0 ][ 1 ] == leaf_cluster[ 1 ][ 0 ] && leaf_cluster[ 0 ][ 1 ] != leaf_cluster[ 0 ][ 0 ] && leaf_cluster[ 0 ][ 1 ] != leaf_cluster[ 1 ][ 1 ] ) {
				// non-manifold, reject #2
				return false;
			}

			for ( int i1 = 0; i1 < 2; i1++ ) {
				for ( int j1 = 0; j1 < 2; j1++ ) {
					for ( int i2 = 0; i2 < 2; i2++ ) {
						for ( int j2 = 0; j2 < 2; j2++ ) {
							if ( leaf_cluster[ i1 ][ j1 ] != leaf_cluster[ i2 ][ j2 ] && cluster[ ii + i1 ][ jj + j1 ] == cluster[ ii + i2 ][ jj + j2 ] ) {
								// two cluster merged, reject #3
								// note that as long as we use topology check, the cluster# at any grid point must agree with one of the four corners of the surrounding node
								return false;
							}
						}
					}
				}
			}
		}
	}

	// check #4 : if topology is preserved
	// check 4 edge centers
	if ( cluster[ 0 ][ 1 ] != cluster[ 0 ][ 0 ] && cluster[ 0 ][ 1 ] != cluster[ 0 ][ 2 ] )
		return false;
	if ( cluster[ 1 ][ 0 ] != cluster[ 0 ][ 0 ] && cluster[ 1 ][ 0 ] != cluster[ 2 ][ 0 ] )
		return false;
	if ( cluster[ 2 ][ 1 ] != cluster[ 2 ][ 0 ] && cluster[ 2 ][ 1 ] != cluster[ 2 ][ 2 ] )
		return false;
	if ( cluster[ 1 ][ 2 ] != cluster[ 0 ][ 2 ] && cluster[ 1 ][ 2 ] != cluster[ 2 ][ 2 ] )
		return false;
	// check node center
	if ( cluster[ 1 ][ 1 ] != cluster[ 0 ][ 0 ] && cluster[ 1 ][ 1 ] != cluster[ 0 ][ 2 ] && cluster[ 1 ][ 1 ] != cluster[ 2 ][ 0 ] && cluster[ 1 ][ 1 ] != cluster[ 2 ][ 2 ] )
		return false;

	return true;
}
