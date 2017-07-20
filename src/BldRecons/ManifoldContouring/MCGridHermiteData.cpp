#include "StdAfx.h"
#include "MCGrid.h"

//////////////////////////////////////////////////////////////////////////
// HermiteData functions
//////////////////////////////////////////////////////////////////////////

void CMCGrid::ComputeHermiteData_XY()
{
	for ( int i = 0; i < m_nHDSideNumber; i++ ) {
		for ( int j = 0; j < m_nHDSideNumber; j++ ) {
			ComputeHermiteData_XY( i, j );
		}
	}
}

void CMCGrid::ComputeHermiteData_XY( int i, int j )
{
	// segment points in its four adjacent cells
	CVector3DPointer_Vector points;
	std::vector< int > points_idx;

	// surrounded by (i,j) (i+1,j) (i,j+1) (i+1,j+1)
	for ( int ii = 0; ii <= 1; ii++ ) {
		for ( int jj = 0; jj <= 1; jj++ ) {
			if ( IIsValid( ii + i ) && JIsValid( jj + j ) ) {
				Grid_Index & grid_index = m_vecIndex[ Index( ii + i, jj + j ) ];
				for ( int k = 0; k < ( int )grid_index.size(); k++ ) {
					int idx = grid_index[ k ];
					points_idx.push_back( idx );
					points.push_back( &( m_pPointCloud->m_vecPoint[ idx ] ) );
				}
			}
		}
	}
	std::vector< int > points_cluster;
	DistanceSegmentationEx( points, points_cluster, m_dbSegmentationDistance * m_dbSegmentationDistance, m_dbSegmentationZ );

	// estimate the height of each cluster
	int c = CompressCluster( points_cluster );
	std::vector< double > cluster_height( c, 0.0 );
	std::vector< int > cluster_number( c, 0 );

	for ( int ci = 0; ci < ( int )points_cluster.size(); ci++ ) {
		cluster_height[ points_cluster[ ci ] ] += points[ ci ]->pVec[ 2 ];
		cluster_number[ points_cluster[ ci ] ] ++;
	}
	for ( int k = 0; k < c; k++ ) {
		cluster_height[ k ] /= ( double )( cluster_number[ k ] );
	}

	// find the highest cluster covering each adjacent cell
	int covering_cluster[ 2 ][ 2 ] = { { -1, -1 }, { -1, -1 } };
	double covering_height[ 2 ][ 2 ] = { { -1e300, -1e300 }, { -1e300, -1e300 } };
	int ci = 0;
	for ( int ii = 0; ii <= 1; ii++ ) {
		for ( int jj = 0; jj <= 1; jj++ ) {
			if ( IIsValid( ii + i ) && JIsValid( jj + j ) ) {
				Grid_Index & grid_index = m_vecIndex[ Index( ii + i, jj + j ) ];
				for ( int k = 0; k < ( int )grid_index.size(); k++ ) {
					if ( cluster_number[ points_cluster[ ci ] ] >= m_nAcceptNumber && cluster_height[ points_cluster[ ci ] ] > covering_height[ ii ][ jj ] ) {
						covering_height[ ii ][ jj ] = cluster_height[ points_cluster[ ci ] ];
						covering_cluster[ ii ][ jj ] = points_cluster[ ci ];
					}
					ci++;
				}
			}
		}
	}

	// find the lowest cluster covering the grid point
	double point_covering_height = 1e300;
	int point_covering_cluster = -1;
	for ( int ii = 0; ii <= 1; ii++ ) {
		for ( int jj = 0; jj <= 1; jj++ ) {
			if ( covering_height[ ii ][ jj ] < point_covering_height ) {
				point_covering_height = covering_height[ ii ][ jj ];
				point_covering_cluster = covering_cluster[ ii ][ jj ];
			}
		}
	}

	// finally, compute x-y hermite data
	HermiteData & hd = m_vecHermiteDataXY[ HDIndex( i, j ) ];
	hd.v = CVector3D( i, j, 0 ) * m_dbGridLength + m_pPointCloud->m_cBoundingBox.m_vMin;
	hd.n = CVector3D( 0.0, 0.0, 1.0 );
	if ( point_covering_cluster == -1 ) {
		// if the covering cluster is a ground
		hd.v[ 2 ] = m_pPointCloud->m_dbGroundZ;
	} else {
		std::vector< DistanceSorting > sorting_result;
		SortDistance2D( points, hd.v, points_cluster, point_covering_cluster, sorting_result );
		hd.n = CVector3D( 0.0, 0.0, 0.0 );
		hd.v[ 2 ] = 0.0;
		for ( int k = 0; k < m_nAcceptNumber; k++ ) {
			hd.v[ 2 ] += m_pPointCloud->m_vecPoint[ points_idx[ sorting_result[ k ].index ] ][ 2 ];
			hd.n += m_pPointCloud->m_vecNormal[ points_idx[ sorting_result[ k ].index ] ];
		}
		hd.v[ 2 ] /= ( double )m_nAcceptNumber;
		hd.n.normalize();
	}
}

void CMCGrid::ComputeHermiteData_Z()
{
	// first dir along Y
	for ( int i = 0; i < m_nHDSideNumber; i++ ) {
		for ( int j = 0; j < m_nHDSideNumber - 1; j++ ) {
			ComputeHermiteData_Z( i, j, 0 );
		}
	}

	// second dir along X
	for ( int i = 0; i < m_nHDSideNumber - 1; i++ ) {
		for ( int j = 0; j < m_nHDSideNumber; j++ ) {
			ComputeHermiteData_Z( i, j, 1 );
		}
	}
}

void CMCGrid::ComputeHermiteData_Z( int i, int j, int dir )
{
	HermiteData & hd = m_vecHermiteDataZ[ HDIndex( i, j ) * 2 + dir ];
	CVector3D v[ 2 ] = {
		m_vecHermiteDataXY[ HDIndex( i, j ) ].v,
		m_vecHermiteDataXY[ HDIndex( i + dir, j + 1 - dir ) ].v
	};
	CVector3DPointer_Vector points;
	points.push_back( & v[ 0 ] );
	points.push_back( & v[ 1 ] );

	if ( IIsValid( i + dir ) && JIsValid( j + 1 - dir ) ) {
		Grid_Index & grid_index_0 = m_vecIndex[ Index( i + dir, j + 1 - dir ) ];
		for ( int k = 0; k < ( int )grid_index_0.size(); k++ )
			points.push_back( & m_pPointCloud->m_vecPoint[ grid_index_0[ k ] ] );
	}
	if ( IIsValid( i + 1 ) && JIsValid( j + 1 ) ) {
		Grid_Index & grid_index_1 = m_vecIndex[ Index( i + 1, j + 1 ) ];
		for ( int k = 0; k < ( int )grid_index_1.size(); k++ )
			points.push_back( & m_pPointCloud->m_vecPoint[ grid_index_1[ k ] ] );
	}

	ComputeHermiteData_Z( hd, v, points );
}

void CMCGrid::ComputeHermiteData_Z( HermiteData & hd, CVector3D v[ 2 ], CVector3DPointer_Vector & points )
{
	// Parameters:
	// points is used for segmentation, and v is used for calculating
	// note that points[0..1] == &v[0..1]

	std::vector< int > points_cluster;
	DistanceSegmentationEx( points, points_cluster, m_dbSegmentationDistance * m_dbSegmentationDistance, m_dbSegmentationZ );

	if ( ( HermiteDataXY_IsGround( v[ 0 ] ) && HermiteDataXY_IsGround( v[ 1 ] ) ) || points_cluster[ 0 ] == points_cluster[ 1 ] ) {
		// they are in the same cluster
		// do nothing
	} else {
		// otherwise...
		int higher_v_index = -1;
		if ( HermiteDataXY_IsGround( v[ 0 ] ) ) {
			higher_v_index = 1;
		} else if ( HermiteDataXY_IsGround( v[ 1 ] ) ) {
			higher_v_index = 0;
		} else if ( v[ 0 ][ 2 ] > v[ 1 ][ 2 ] ) {
			higher_v_index = 0;
		} else {
			higher_v_index = 1;
		}

		int higher_cluster = points_cluster[ higher_v_index ];
		CVector3D & lower_v = v[ 1 - higher_v_index ];

		std::vector< DistanceSorting > sorting_result;
		SortDistance2D( points, lower_v, points_cluster, higher_cluster, sorting_result );

		// now apply 2D support vector machine
		CVector3D v_key;
		CVector3D v_dir;

		// first check if sorting_result[0] is the only active set
		bool pass = true;
		v_key = * points[ sorting_result[ 0 ].index ];
		for ( int i = 1; i < ( int )sorting_result.size(); i++ ) {
			CVector3D & v = * points[ sorting_result[ i ].index ];
			if ( ( lower_v - v_key ).XY() * ( v - v_key ).XY() > 0 ) {
				pass = false;
				break;
			}
		}

		if ( pass ) {
			CVector3D v_norm = lower_v - v_key;
			v_dir = CVector3D( v_norm[ 1 ], - v_norm[ 0 ], 0.0 );
		} else {
			// check active set {i, j}
			double best_dis = 0.0;
			CVector3D v_key_temp;
			CVector3D v_dir_temp;

			for ( int i = 0; i < ( int )sorting_result.size(); i++ ) {
				v_key_temp = * points[ sorting_result[ i ].index ];
				for ( int j = i + 1; j < ( int )sorting_result.size(); j++ ) {
					v_dir_temp = * points[ sorting_result[ j ].index ] - v_key_temp;
					CVector3D v_norm_temp = CVector3D( v_dir_temp[ 1 ], - v_dir_temp[ 0 ], 0.0 );
					pass = true;

					for ( int k = 0; k < ( int )sorting_result.size(); k++ ) {
						CVector3D & v = * points[ sorting_result[ k ].index ];
						if ( k != i && k != j && ( ( lower_v - v_key_temp ) * v_norm_temp ) * ( ( v - v_key_temp ) * v_norm_temp ) > 0.0 ) {
							pass = false;
							break;
						}
					}

					if ( pass && fabs( ( lower_v - v_key_temp ) * v_norm_temp ) > best_dis ) {
						v_key = v_key_temp;
						v_dir = v_dir_temp;
						best_dis = fabs( ( lower_v - v_key_temp ) * v_norm_temp );
					}
				}
			}
			if ( best_dis == 0.0 ) {
				v_key = * points[ sorting_result[ 0 ].index ];
				v_dir = * points[ sorting_result[ 1 ].index ] - v_key;
			}
		}

		ComputeHermiteData_Z_Key( hd, v, v_key, v_dir );
	}

}

void CMCGrid::ComputeHermiteData_Z_Ill( HermiteData & hd, CVector3D v[ 2 ] )
{
	hd.v = ( v[ 0 ] + v[ 1 ] ) * 0.5;

	if ( v[ 0 ][ 2 ] > v[ 1 ][ 2 ] ) {
		hd.n = v[ 1 ] - v[ 0 ];
	} else {
		hd.n = v[ 0 ] - v[ 1 ];
	}
	hd.n[ 2 ] = 0.0;
	hd.n.normalize();
}

void CMCGrid::ComputeHermiteData_Z_Key( HermiteData & hd, CVector3D v[ 2 ], CVector3D & v_key, CVector3D & v_dir )
{
	double temp = ( v_dir.XY() ^ ( v[ 0 ] - v[ 1 ] ).XY() )[ 2 ];
	if ( abs( temp ) / ( v[ 0 ] - v[ 1 ] ).XY().length() < 1e-6 ) {

		ComputeHermiteData_Z_Ill( hd, v );

	} else {

		double a = ( ( ( v[ 0 ] - v_key ).XY() ^ ( v[ 1 ] - v_key ).XY() )[ 2 ] ) / temp;
		hd.v = v_key - a * v_dir;
		if ( ( v[ 0 ] - hd.v ) * ( v[ 0 ] - v[ 1 ] ).XY() < 0.0 )
			hd.v = v[ 0 ];
		if ( ( v[ 1 ] - hd.v ) * ( v[ 1 ] - v[ 0 ] ).XY() < 0.0 )
			hd.v = v[ 1 ];
		hd.v[ 2 ] = ( v[ 0 ][ 2 ] + v[ 1 ][ 2 ] ) * 0.5;

		hd.n = CVector3D( v_dir[ 1 ], - v_dir[ 0 ], 0.0 );
		hd.n.normalize();

		if ( hd.n * ( v[ 0 ] - v[ 1 ] ) * ( v[ 0 ][ 2 ] - v[ 1 ][ 2 ] ) > 0.0 ) 
			hd.n *= -1.0;

	}
}
