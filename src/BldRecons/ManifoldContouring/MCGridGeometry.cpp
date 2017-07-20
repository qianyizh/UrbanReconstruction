#include "StdAfx.h"
#include "MCGrid.h"
#include "nr\nr.h"
#include "nr\nrutil.h"

void CMCGrid::BuildQuadTreeAtLevel0()
{
	MCGridNode_Vector & nodes = m_vecNodes[ 0 ];
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
			MCGridNode & node = nodes[ i * sidenumber + j ];
			node.level = 0;
			node.hpoints.resize( 1 );
			MCHyperPoint & hpoint = node.hpoints[ 0 ];
			hpoint.is_ground = true;
			hpoint.is_feature = false;
			hpoint.is_foldingpoint = false;
			hpoint.is_collapsible = true;
			hpoint.parent = & hpoint;
			hpoint.leaf_to_parent[ 0 ] = 0;
			hpoint.leaf_to_parent[ 1 ] = 1;
			hpoint.num_of_leafs = 0;
			hpoint.num_of_samples = 4;

			// step 1, split layers
			CVector3DPointer_Vector points;
			for ( int ii = 0; ii <= 1; ii++ ) {
				for ( int jj = 0; jj <= 1; jj++ ) {
					points.push_back( &( m_vecHermiteDataXY[ HDIndex( ii + i, jj + j ) ].v ) );
					if ( HermiteDataXY_IsGround( m_vecHermiteDataXY[ HDIndex( ii + i, jj + j ) ].v ) == false )
						hpoint.is_ground = false;
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
			hpoint.num_of_layers = node.num_of_layers;
			hpoint.points.resize( node.num_of_layers );
			hpoint.bi.resize( node.num_of_layers, -1 );
			if ( hpoint.num_of_layers > 1 )
				hpoint.is_ground = false;

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
			hpoint.means.resize( node.num_of_layers );
			CVector3D tempxy( 0.0, 0.0, 0.0 );
			if ( z.size() != 0 ) {
				for ( int l = 0; l < ( int )z.size(); l++ ) {
					tempxy += m_vecHermiteDataZ[ z[ l ].i ].v;
				}
				tempxy /= ( double )z.size();
				// hpoint.mean_based_on_z = true;
			} else {
				for ( int l = 0; l < ( int )xy.size(); l++ ) {
					tempxy += m_vecHermiteDataXY[ xy[ l ].i ].v;
				}
				tempxy /= ( double )xy.size();
				// hpoint.mean_based_on_z = false;
			}
			std::vector< int > num_of_cluster( node.num_of_layers, 0 );
			for ( int l = 0; l < node.num_of_layers; l++ ) {
				node.hpoints[ 0 ].means[ l ] = tempxy;
				node.hpoints[ 0 ].means[ l ][ 2 ] = 0.0;
				num_of_cluster[ l ] = 0;
			}
			for ( int l = 0; l < ( int )xy.size(); l++ ) {
				int c = xy[ l ].l;
				hpoint.means[ c ][ 2 ] += m_vecHermiteDataXY[ xy[ l ].i ].v[ 2 ];
				num_of_cluster[ c ]++;
			}
			for ( int l = 0; l < node.num_of_layers; l++ ) {
				hpoint.means[ l ][ 2 ] /= ( double )num_of_cluster[ l ];
			}
			for ( int l = 0; l < node.num_of_layers; l++ ) {
				hpoint.means[ l ] = AbsoluteToRelative( hpoint.means[ l ] );
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
				hpoint.r.push_back( d[ ii + 1 ] );
				for ( int jj = ii+1; jj < matrix_width; jj++ ) {
					hpoint.r.push_back( a[ ii + 1 ][ jj + 1 ] );
				}
			}

			// step 5, estimate geometry in this node
			DualContouringGeometryHPoint( node.hpoints[ 0 ] );
			node.is_collapsible = hpoint.is_collapsible;
			// at level 0, we force every node pass the error test
		}
	}

	free_matrix( a, 1, MAX_MATRIX_SIZE, 1, MAX_MATRIX_SIZE );
	free_vector( b, 1, MAX_MATRIX_SIZE );
	free_vector( c, 1, MAX_MATRIX_SIZE );
	free_vector( d, 1, MAX_MATRIX_SIZE );
}

void CMCGrid::BuildQuadTreeAtLevelN( int l )
{
	MCGridNode_Vector & nodes = m_vecNodes[ l ];
	int sidelength = ( 1 << l );
	int sidenumber = ( 1 << ( m_nLevel - l ) );
	nodes.clear();
	nodes.resize( sidenumber * sidenumber );

	for ( int i = 0; i < sidenumber; i++ ) {
		for ( int j = 0; j < sidenumber; j++ ) {
			NodeLocator loc( l, i, j );
			MCGridNode & node = LocateNode( loc );
			node.level = l;
			//node.is_collapsible = false;

			//for ( int ii = 0; ii < 2; ii++ ) {
			//	for ( int jj = 0; jj < 2; jj++ ) {
			//		MCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
			//		if ( leaf_node.is_collapsible == true ) {
			//			node.is_collapsible = true;
			//		}
			//	}
			//}

			//if ( node.is_collapsible == true ) {

			// step 1, split layers, build indexing structures
			// node.num_of_layers = ClusterFromLeafNodes( loc );

			int i_shift = i * sidelength;
			int j_shift = j * sidelength;
			// step 2, sweep all the surface polygons
			for ( int ii = 0; ii < sidelength - 1; ii++ ) {
				MCGridNode * n[ 2 ][ 2 ] = {
					{ & LocateNode( NodeLocator( 0, i_shift + ii, j_shift + sidelength / 2 - 1 ) ), & LocateNode( NodeLocator( 0, i_shift + ii, j_shift + sidelength / 2 ) ) },
					{ & LocateNode( NodeLocator( 0, i_shift + ii + 1, j_shift + sidelength / 2 - 1 ) ), & LocateNode( NodeLocator( 0, i_shift + ii + 1, j_shift + sidelength / 2 ) ) }
				};
				HP_SweepSurface( n );
			}
			for ( int jj = 0; jj < sidelength - 1; jj++ ) {
				MCGridNode * n[ 2 ][ 2 ] = {
					{ & LocateNode( NodeLocator( 0, i_shift + sidelength / 2 - 1, j_shift + jj ) ), & LocateNode( NodeLocator( 0, i_shift + sidelength / 2 - 1, j_shift + jj ) ) },
					{ & LocateNode( NodeLocator( 0, i_shift + sidelength / 2, j_shift + jj + 1 ) ), & LocateNode( NodeLocator( 0, i_shift + sidelength / 2, j_shift + jj + 1 ) ) }
				};
				HP_SweepSurface( n );
			}
			
			// step 3, sweep all the boundary polygons
			for ( int ii = 0; ii < sidelength; ii++ ) {
				MCGridNode * n[ 2 ] = {
					& LocateNode( NodeLocator( 0, i_shift + ii, j_shift + sidelength / 2 - 1 ) ), & LocateNode( NodeLocator( 0, i_shift + ii, j_shift + sidelength / 2 ) )
				};
				HP_SweepBoundary( n, ED_Along_X );
			}
			for ( int jj = 0; jj < sidelength; jj++ ) {
				MCGridNode * n[ 2 ] = {
					& LocateNode( NodeLocator( 0, i_shift + sidelength / 2 - 1, j_shift + jj ) ), & LocateNode( NodeLocator( 0, i_shift + sidelength / 2, j_shift + jj ) )
				};
				HP_SweepBoundary( n, ED_Along_Y );
			}

			// step 4, check how many clusters need to try being collapsed
			int hpoint_size = 0;
			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					MCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					for ( int k = 0; k < ( int )leaf_node.hpoints.size(); k++ ) {
						MCHyperPoint & leaf_hpoint = leaf_node.hpoints[ k ];
						if ( ( leaf_hpoint.Is_1HPoint() || leaf_hpoint.Is_2HPoint() ) && leaf_hpoint.parent == & leaf_hpoint ) {
							hpoint_size ++;
						}
					}
				}
			}

			node.hpoints.resize( hpoint_size );
			int kh = 0;

			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					MCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					for ( int k = 0; k < ( int )leaf_node.hpoints.size(); k++ ) {
						MCHyperPoint & leaf_hpoint = leaf_node.hpoints[ k ];
						if ( ( leaf_hpoint.Is_1HPoint() || leaf_hpoint.Is_2HPoint() ) && leaf_hpoint.parent == & leaf_hpoint ) {
							MCHyperPoint & hpoint = node.hpoints[ kh ];
							kh++;
							
							hpoint.is_feature = false;
							hpoint.is_collapsible = true;
							hpoint.is_foldingpoint = false;
							hpoint.is_ground = true;

							leaf_hpoint.parent = & hpoint;
							hpoint.parent = & hpoint;
							hpoint.leaf_to_parent[ 0 ] = 0;
							hpoint.leaf_to_parent[ 1 ] = 1;
							hpoint.num_of_leafs = 0;
							hpoint.num_of_samples = 0;

							hpoint.num_of_layers = leaf_hpoint.num_of_layers;
							hpoint.r.resize( leaf_hpoint.r.size(), 0.0 );
							hpoint.means.resize( leaf_hpoint.means.size(), CVector3D( 0.0, 0.0, 0.0 ) );
							hpoint.points.resize( leaf_hpoint.points.size() );
							hpoint.bi.resize( leaf_hpoint.points.size(), -1 );
							for ( int l = 0; l < ( int )leaf_hpoint.points.size(); l++ ) {
								hpoint.bi[ l ] = leaf_hpoint.bi[ l ];
							}
						}
					}
				}
			}

			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					MCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					for ( int k = 0; k < ( int )leaf_node.hpoints.size(); k++ ) {
						MCHyperPoint & leaf_hpoint = leaf_node.hpoints[ k ];
						if ( ( leaf_hpoint.Is_1HPoint() && leaf_hpoint.parent->Is_1HPoint() ) || ( leaf_hpoint.Is_2HPoint() && leaf_hpoint.parent->Is_2HPoint() ) ) {
							MCHyperPoint * root = HP_FindCluster_Flatten( & leaf_hpoint );
							if ( leaf_hpoint.is_collapsible == false ) {
								root->is_collapsible = false;
							}
							if ( leaf_hpoint.is_ground == false ) {
								root->is_ground = false;
							}
						}
					}
				}
			}

			for ( int ii = 0; ii < 2; ii++ ) {
				for ( int jj = 0; jj < 2; jj++ ) {
					MCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
					for ( int k = 0; k < ( int )leaf_node.hpoints.size(); k++ ) {
						MCHyperPoint & leaf_hpoint = leaf_node.hpoints[ k ];
						if ( ( leaf_hpoint.Is_1HPoint() && leaf_hpoint.parent->Is_1HPoint() ) || ( leaf_hpoint.Is_2HPoint() && leaf_hpoint.parent->Is_2HPoint() ) ) {
							MCHyperPoint * root = leaf_hpoint.parent;
							if ( root->is_collapsible ) {
								if ( root->is_ground || ! leaf_hpoint.is_ground ) {
									for ( int ll = 0; ll < ( int )root->num_of_layers; ll++ ) {
										root->means[ ll ] += leaf_hpoint.means[ leaf_hpoint.leaf_to_parent[ ll ] ];
									}
									AddHPoint( *root, leaf_hpoint, leaf_hpoint.leaf_to_parent, true );
									root->num_of_leafs++;
									root->num_of_samples += leaf_hpoint.num_of_samples;
								} else {
									leaf_hpoint.parent = & leaf_hpoint;
								}
							}
						}
					}
				}
			}

			for ( int k = 0; k < ( int )node.hpoints.size(); k++ ) {
				MCHyperPoint & hpoint = node.hpoints[ k ];
				if ( hpoint.is_collapsible ) {
					for ( int l = 0; l < ( int )hpoint.num_of_layers; l++ ) {
						hpoint.means[ l ] /= ( double )hpoint.num_of_leafs;
					}
					DualContouringGeometryHPoint( hpoint );

					// check boundary
					if ( hpoint.Is_2HPoint() && hpoint.is_collapsible ) {
						for ( int l = 0; l < ( int )hpoint.num_of_layers; l++ ) {
							if ( hpoint.bi[ l ] != -1 ) {
								MCBoundary & bdr = m_vecBoundaries[ hpoint.bi[ l ] ];
								if ( bdr.remaining_v_num - hpoint.num_of_leafs + 1 < m_nMinBoundaryRemaining ) {
									hpoint.is_collapsible = false;
								}
							}
						}
						if ( hpoint.is_collapsible ) {
							for ( int l = 0; l < ( int )hpoint.num_of_layers; l++ ) {
								if ( hpoint.bi[ l ] != -1 ) {
									MCBoundary & bdr = m_vecBoundaries[ hpoint.bi[ l ] ];
									bdr.remaining_v_num -= ( hpoint.num_of_leafs - 1 );
								}
							}
						}
					}
				}
			}

			// step 5, in-cell collapse 1-to-N
			JoinElement_Set jset;

			for ( int ii = 0; ii < sidelength - 1; ii++ ) {
				MCGridNode * n[ 2 ][ 2 ] = {
					{ & LocateNode( NodeLocator( 0, i_shift + ii, j_shift + sidelength / 2 - 1 ) ), & LocateNode( NodeLocator( 0, i_shift + ii, j_shift + sidelength / 2 ) ) },
					{ & LocateNode( NodeLocator( 0, i_shift + ii + 1, j_shift + sidelength / 2 - 1 ) ), & LocateNode( NodeLocator( 0, i_shift + ii + 1, j_shift + sidelength / 2 ) ) }
				};
				HP_SweepSurfaceJoin( n, jset );
			}
			for ( int jj = 0; jj < sidelength - 1; jj++ ) {
				MCGridNode * n[ 2 ][ 2 ] = {
					{ & LocateNode( NodeLocator( 0, i_shift + sidelength / 2 - 1, j_shift + jj ) ), & LocateNode( NodeLocator( 0, i_shift + sidelength / 2 - 1, j_shift + jj ) ) },
					{ & LocateNode( NodeLocator( 0, i_shift + sidelength / 2, j_shift + jj + 1 ) ), & LocateNode( NodeLocator( 0, i_shift + sidelength / 2, j_shift + jj + 1 ) ) }
				};
				HP_SweepSurfaceJoin( n, jset );
			}

			// in-cell collapse
			// in all join elements that leaf->parent == leaf
			// find the minimum join error = ( merge_error - r1->error - r2->error ) while ( merge_error < m_dbErrorTolerance ^ 2 )

			double minimum_join_error = m_dbErrorTolerance * m_dbErrorTolerance;
			JoinElement_Set::iterator minimum_join_element = jset.begin();

			while ( minimum_join_element != jset.end() ) {
				minimum_join_element = jset.end();
				for ( JoinElement_Set::iterator it = jset.begin(); it != jset.end(); it++ ) {
					if ( it->leaf->parent == it->leaf ) {
						double merge_error = AddHPoint( * ( it->parent ), * ( it->leaf ), it->leaf_to_parent, false );
						if ( merge_error < ( m_dbErrorTolerance * m_dbErrorTolerance )
							&& merge_error - it->leaf->error - it->parent->error < minimum_join_error )
						{
							minimum_join_error = merge_error - it->leaf->error - it->parent->error;
							minimum_join_element = it;
						}
					}
				}
				if ( minimum_join_element != jset.end() ) {
					AddHPoint( * ( minimum_join_element->parent ), * ( minimum_join_element->leaf ), minimum_join_element->leaf_to_parent, true );
					DualContouringGeometryHPoint( * ( minimum_join_element->parent ) );
					minimum_join_element->leaf->parent = minimum_join_element->parent;
					minimum_join_element->leaf->leaf_to_parent[ 0 ] = minimum_join_element->leaf_to_parent[ 0 ];
					minimum_join_element->leaf->leaf_to_parent[ 1 ] = minimum_join_element->leaf_to_parent[ 1 ];
				}
			}

			//continue;

			// step 6, in-cell collapse 2-to-N
			jset.clear();

			for ( int ii = 0; ii < sidelength; ii++ ) {
				MCGridNode * n[ 2 ] = {
					& LocateNode( NodeLocator( 0, i_shift + ii, j_shift + sidelength / 2 - 1 ) ), & LocateNode( NodeLocator( 0, i_shift + ii, j_shift + sidelength / 2 ) )
				};
				HP_SweepBoundaryJoin( n, ED_Along_X, jset );
			}
			for ( int jj = 0; jj < sidelength; jj++ ) {
				MCGridNode * n[ 2 ] = {
					& LocateNode( NodeLocator( 0, i_shift + sidelength / 2 - 1, j_shift + jj ) ), & LocateNode( NodeLocator( 0, i_shift + sidelength / 2, j_shift + jj ) )
				};
				HP_SweepBoundaryJoin( n, ED_Along_Y, jset );
			}

			// in-cell collapse
			// in all join elements that leaf->parent == leaf
			// find the minimum join error = ( merge_error - r1->error - r2->error ) while ( merge_error < m_dbErrorTolerance ^ 2 )

			minimum_join_error = m_dbErrorTolerance * m_dbErrorTolerance;
			minimum_join_element = jset.begin();

			while ( minimum_join_element != jset.end() ) {
				minimum_join_element = jset.end();
				for ( JoinElement_Set::iterator it = jset.begin(); it != jset.end(); it++ ) {
					if ( it->leaf->parent == it->leaf ) {
						double merge_error = AddHPoint( * ( it->parent ), * ( it->leaf ), it->leaf_to_parent, false );

						// check boundary
						bool topopass = true;
						if ( it->leaf->Is_2HPoint() ) {
							for ( int l = 0; l < ( int )it->leaf->num_of_layers; l++ ) {
								if ( it->leaf->bi[ l ] != -1 ) {
									MCBoundary & bdr = m_vecBoundaries[ it->leaf->bi[ l ] ];
									if ( bdr.remaining_v_num - 1 < m_nMinBoundaryRemaining ) {
										topopass = false;
									}
								}
							}
						}

						if ( topopass && merge_error < ( m_dbErrorTolerance * m_dbErrorTolerance )
							&& merge_error - it->leaf->error - it->parent->error < minimum_join_error )
						{
							minimum_join_error = merge_error - it->leaf->error - it->parent->error;
							minimum_join_element = it;
						}
					}
				}
				if ( minimum_join_element != jset.end() ) {
					AddHPoint( * ( minimum_join_element->parent ), * ( minimum_join_element->leaf ), minimum_join_element->leaf_to_parent, true );
					DualContouringGeometryHPoint( * ( minimum_join_element->parent ) );

					if ( minimum_join_element->leaf->Is_2HPoint() ) {
						for ( int l = 0; l < ( int )minimum_join_element->leaf->num_of_layers; l++ ) {
							if ( minimum_join_element->leaf->bi[ l ] != -1 ) {
								MCBoundary & bdr = m_vecBoundaries[ minimum_join_element->leaf->bi[ l ] ];
								bdr.remaining_v_num--;
							}
						}
					}

					minimum_join_element->leaf->parent = minimum_join_element->parent;
					minimum_join_element->leaf->leaf_to_parent[ 0 ] = minimum_join_element->leaf_to_parent[ 0 ];
					minimum_join_element->leaf->leaf_to_parent[ 1 ] = minimum_join_element->leaf_to_parent[ 1 ];
					minimum_join_error = m_dbErrorTolerance * m_dbErrorTolerance;
				}
			}
		}
	}
}

double CMCGrid::AddHPoint( MCHyperPoint & base, MCHyperPoint & adding, int leaf_to_root[ 2 ], bool overwrite_base )
{
	if ( adding.is_ground && ! base.is_ground ) {
		return base.error;
	}

	int matrix_width = base.num_of_layers + 3;
	int matrix_size = ( base.num_of_layers + 3 ) + ( adding.num_of_layers + 3 );

	int sing;
	float **a, *b, *c, *d;
	a = matrix( 1, matrix_size, 1, matrix_size );
	b = vector( 1, matrix_size );
	c = vector( 1, matrix_size );
	d = vector( 1, matrix_size );

	for ( int i = 1; i <= matrix_size; i++ )
		for ( int j = 1; j <= matrix_size; j++ )
			a[ i ][ j ] = 0.0f;

	int k = 0;
	for ( int i = 1; i <= base.num_of_layers + 3; i++ ) {
		for ( int j = i; j <= base.num_of_layers + 3; j++ ) {
			a[ i ][ j ] = base.r[ k ];
			k++;
		}
	}

	k = 0;
	for ( int i = 1; i <= adding.num_of_layers + 3; i++ ) {
		for ( int j = i; j <= adding.num_of_layers + 3; j++ ) {
			if ( j < 3 ) {
				a[ i + base.num_of_layers + 3 ][ j ] = adding.r[ k ];
			} else if ( j < adding.num_of_layers + 3 ) {
				a[ i + base.num_of_layers + 3 ][ leaf_to_root[ j - 3 ] + 3 ] = adding.r[ k ];
			} else {
				a[ i + base.num_of_layers + 3 ][ base.num_of_layers + 3 ] = adding.r[ k ];
			}
			k++;
		}
	}
	
	qrdcmp_ext( a, matrix_size, c, d, &sing );

	double rr = d[ matrix_width ];

	if ( overwrite_base ) {
		k = 0;
		for ( int i = 1; i <= matrix_width; i++ ) {
			base.r[ k ] = d[ i ];
			k++;
			for ( int j = i + 1; j <= matrix_width; j++ ) {
				base.r[ k ] = a[ i ][ j ];
				k++;
			}
		}
	}

	free_matrix( a, 1, matrix_size, 1, matrix_size );
	free_vector( b, 1, matrix_size );
	free_vector( c, 1, matrix_size );
	free_vector( d, 1, matrix_size );

	return ( rr * rr );
}

void CMCGrid::DualContouringGeometryHPoint( MCHyperPoint & hpoint )
{
	int matrix_width = hpoint.num_of_layers + 3;

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
			u[ i ][ j ] = aa[ i ][ j ] = hpoint.r[ k ];
			k++;
		}
		bb[ i ] = hpoint.r[ k ];
		k++;
	}
	rr = hpoint.r[ k ];

	xx[ 1 ] = ( float )hpoint.means[ 0 ][ 0 ];
	xx[ 2 ] = ( float )hpoint.means[ 0 ][ 1 ];
	for ( int i = 0; i < ( int )hpoint.means.size(); i++ )
		xx[ i + 3 ] = ( float )hpoint.means[ i ][ 2 ];

	svdcmp_ext( u, matrix_width - 1, matrix_width - 1, w, v );

	if ( hpoint.num_of_layers == 1 ) {
		//double min_sigma = 1e300;
		//for ( int i = 1; i <= 3; i++ ) {
		//	if ( w[ i ] * w[ i ] < min_sigma ) {
		//		min_sigma = w[ i ] * w[ i ];
		//		hpoint.dir = CVector3D( v[ 1 ][ i ], v[ 2 ][ i ], v[ 3 ][ i ] );
		//	}
		//}
		for ( int i = 1; i <= 3; i++ ) {
			hpoint.sigma2[ i - 1 ] = w[ i ] * w[ i ] / ( double )hpoint.num_of_samples;
			hpoint.v[ i - 1 ] = CVector3D( v[ 1 ][ i ], v[ 2 ][ i ], v[ 3 ][ i ] );
		}
		// sort
		for ( int i = 0; i < 3; i++ ) {
			for ( int j = i + 1; j < 3; j++ ) {
				if ( hpoint.sigma2[ i ] < hpoint.sigma2[ j ] ) {
					double temp_sigma2 = hpoint.sigma2[ i ];
					hpoint.sigma2[ i ] = hpoint.sigma2[ j ];
					hpoint.sigma2[ j ] = temp_sigma2;
					CVector3D temp_v = hpoint.v[ i ];
					hpoint.v[ i ] = hpoint.v[ j ];
					hpoint.v[ j ] = temp_v;
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

	for ( int i = 0; i < hpoint.num_of_layers; i++ ) {
		hpoint.points[ i ] = CVector3D( xx[ 1 ], xx[ 2 ], xx[ i + 3 ] );
		hpoint.points[ i ] = RelativeToAbsolute( hpoint.points[ i ] );
	}

	hpoint.error = rr * rr;
	hpoint.is_collapsible = ( rr * rr < m_dbErrorTolerance * m_dbErrorTolerance );

	// clean up
	free_matrix( aa, 1, matrix_width - 1, 1, matrix_width - 1 );
	free_vector( bb, 1, matrix_width - 1 );
	free_matrix( u, 1, matrix_width - 1, 1, matrix_width - 1 );
	free_matrix( v, 1, matrix_width - 1, 1, matrix_width - 1 );
	free_vector( w, 1, matrix_width - 1 );
	free_vector( xx, 1, matrix_width - 1 );
	free_vector( b_Ax, 1, matrix_width - 1 );
}
//
//bool CMCGrid::TopologyCheck( NodeLocator & loc )
//{
//	if ( m_bTopologyCheck == false )
//		return true;
//
//	MCGridNode & node = LocateNode( loc );
//	int sidelength = ( 1 << loc.l );
//	int leaf_sidelength = ( 1 << ( loc.l - 1 ) );
//
//	// check #1 : if the parent node is manifold
//	int cluster[ 3 ][ 3 ];
//
//	for ( int iii = 0; iii < 3; iii += 1 ) {
//		for ( int jjj = 0; jjj < 3; jjj += 1 ) {
//			cluster[ iii ][ jjj ] = node.cluster[ iii * leaf_sidelength * ( sidelength + 1 ) + jjj * leaf_sidelength ];
//		}
//	}
//	if ( cluster[ 0 ][ 0 ] == cluster[ 2 ][ 2 ] && cluster[ 0 ][ 0 ] != cluster[ 0 ][ 2 ] && cluster[ 0 ][ 0 ] != cluster[ 2 ][ 0 ] ) {
//		// non-manifold, reject #1
//		return false;
//	}
//	if ( cluster[ 0 ][ 2 ] == cluster[ 2 ][ 0 ] && cluster[ 0 ][ 2 ] != cluster[ 0 ][ 0 ] && cluster[ 0 ][ 2 ] != cluster[ 2 ][ 2 ] ) {
//		// non-manifold, reject #1
//		return false;
//	}
//
//	// check #2 : if all the leaf nodes are manifold
//	// check #3 : if two clusters are merged into one cluster
//	for ( int ii = 0; ii < 2; ii++ ) {
//		for ( int jj = 0; jj < 2; jj++ ) {
//			MCGridNode & leaf_node = LocateNode( Leaf( loc, ii, jj ) );
//
//			int leaf_cluster[ 2 ][ 2 ];
//			for ( int iii = 0; iii < 2; iii++ ) {
//				for ( int jjj = 0; jjj < 2; jjj++ ) {
//					leaf_cluster[ iii ][ jjj ] = leaf_node.cluster[ iii * leaf_sidelength * ( leaf_sidelength + 1 ) + jjj * leaf_sidelength ];
//				}
//			}
//			if ( leaf_cluster[ 0 ][ 0 ] == leaf_cluster[ 1 ][ 1 ] && leaf_cluster[ 0 ][ 0 ] != leaf_cluster[ 0 ][ 1 ] && leaf_cluster[ 0 ][ 0 ] != leaf_cluster[ 1 ][ 0 ] ) {
//				// non-manifold, reject #2
//				return false;
//			}
//			if ( leaf_cluster[ 0 ][ 1 ] == leaf_cluster[ 1 ][ 0 ] && leaf_cluster[ 0 ][ 1 ] != leaf_cluster[ 0 ][ 0 ] && leaf_cluster[ 0 ][ 1 ] != leaf_cluster[ 1 ][ 1 ] ) {
//				// non-manifold, reject #2
//				return false;
//			}
//
//			for ( int i1 = 0; i1 < 2; i1++ ) {
//				for ( int j1 = 0; j1 < 2; j1++ ) {
//					for ( int i2 = 0; i2 < 2; i2++ ) {
//						for ( int j2 = 0; j2 < 2; j2++ ) {
//							if ( leaf_cluster[ i1 ][ j1 ] != leaf_cluster[ i2 ][ j2 ] && cluster[ ii + i1 ][ jj + j1 ] == cluster[ ii + i2 ][ jj + j2 ] ) {
//								// two cluster merged, reject #3
//								// note that as long as we use topology check, the cluster# at any grid point must agree with one of the four corners of the surrounding node
//								return false;
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//
//	// check #4 : if topology is preserved
//	// check 4 edge centers
//	if ( cluster[ 0 ][ 1 ] != cluster[ 0 ][ 0 ] && cluster[ 0 ][ 1 ] != cluster[ 0 ][ 2 ] )
//		return false;
//	if ( cluster[ 1 ][ 0 ] != cluster[ 0 ][ 0 ] && cluster[ 1 ][ 0 ] != cluster[ 2 ][ 0 ] )
//		return false;
//	if ( cluster[ 2 ][ 1 ] != cluster[ 2 ][ 0 ] && cluster[ 2 ][ 1 ] != cluster[ 2 ][ 2 ] )
//		return false;
//	if ( cluster[ 1 ][ 2 ] != cluster[ 0 ][ 2 ] && cluster[ 1 ][ 2 ] != cluster[ 2 ][ 2 ] )
//		return false;
//	// check node center
//	if ( cluster[ 1 ][ 1 ] != cluster[ 0 ][ 0 ] && cluster[ 1 ][ 1 ] != cluster[ 0 ][ 2 ] && cluster[ 1 ][ 1 ] != cluster[ 2 ][ 0 ] && cluster[ 1 ][ 1 ] != cluster[ 2 ][ 2 ] )
//		return false;
//
//	return true;
//}
