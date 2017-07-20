#include "StdAfx.h"
#include "DCGrid.h"

//////////////////////////////////////////////////////////////////////////
// print and debug functions
//////////////////////////////////////////////////////////////////////////

void CDCGrid::HermiteData_SaveToTxt( char filename[] )
{
	FILE * f;
	fopen_s( &f, filename, "w" );

	fprintf_s( f, "%d %d %.10f %.10f\n", m_nWidth, m_nHeight, m_dbGridLength, m_pPointCloud->m_dbGroundZ );
	fprintf_s( f, "%.10f %.10f %.10f %.10f %.10f %.10f\n", m_pPointCloud->m_cBoundingBox.m_vMin[0], m_pPointCloud->m_cBoundingBox.m_vMin[1], m_pPointCloud->m_cBoundingBox.m_vMin[2], m_pPointCloud->m_cBoundingBox.m_vMax[0], m_pPointCloud->m_cBoundingBox.m_vMax[1], m_pPointCloud->m_cBoundingBox.m_vMax[2] );	

	for ( int i = 0; i < m_nWidth; i++ ) {
		for ( int j = 0; j < m_nHeight; j++ ) {
			HermiteData & hd = m_vecHermiteDataXY[ HDIndex( i, j ) ];
			fprintf_s( f, "s %.10f %.10f %.10f %.10f %.10f %.10f\n", hd.v[0], hd.v[1], hd.v[2], hd.n[0], hd.n[1], hd.n[2] );
		}
	}

	for ( int i = 0; i < m_nWidth; i++ ) {
		for ( int j = 0; j < m_nHeight; j++ ) {
			HermiteData & hd0 = m_vecHermiteDataZ[ HDIndex( i, j ) * 2 ];
			fprintf_s( f, "b %.10f %.10f %.10f %.10f %.10f %.10f\n", hd0.v[0], hd0.v[1], hd0.v[2], hd0.n[0], hd0.n[1], hd0.n[2] );
			HermiteData & hd1 = m_vecHermiteDataZ[ HDIndex( i, j ) * 2 + 1 ];
			fprintf_s( f, "b %.10f %.10f %.10f %.10f %.10f %.10f\n", hd1.v[0], hd1.v[1], hd1.v[2], hd1.n[0], hd1.n[1], hd1.n[2] );
		}
	}

	if ( m_vecNodes.size() > 0 ) {
		DCGridNode_Vector & nodes = m_vecNodes[ 0 ];
		if ( nodes.size() > 0 ) {
			for ( int i = 0; i < m_nWidth - 1; i++ ) {
				for ( int j = 0; j < m_nHeight - 1; j++ ) {
					DCGridNode & node = nodes[ ( i << m_nLevel ) + j ];
					fprintf_s( f, "c %d %d %d %d\n", node.cluster[ 0 ], node.cluster[ 1 ], node.cluster[ 2 ], node.cluster[ 3 ] );
				}
			}
		}
	}

	fclose( f );
}

void CDCGrid::PrintLayerNumber()
{
	printf_s( "Layer numbers are:\n" );

	int sidenumber = ( 1 << m_nLevel );
	for ( int i = 0; i < m_nWidth; i++ ) {
		for ( int j = 0; j < m_nHeight; j++ ) {
			for ( int l = m_nLevel; l >= 0; l-- ) {
				if ( m_vecNodes[ l ].size() > 0 ) {
					DCGridNode & node = m_vecNodes[ l ][ ( i >> l << ( m_nLevel - l ) ) + ( j >> l) ];
					if ( node.pass ) {
						printf_s( "%d%d ", l, node.num_of_layers );
						break;
					}
				}
			}
		}
		printf_s( "\n" );
	}
}

void CDCGrid::PrintMatrix( float **a, int matrix_size )
{
	for ( int i = 0; i < matrix_size; i++ ) {
		for ( int j = 0; j < matrix_size; j++ ) {
			printf_s( "%.4f ", a[i+1][j+1] );
		}
		printf_s( "\n" );
	}
	printf_s( "\n" );
}

void CDCGrid::PrintVector( float *w, int vector_size )
{
	for ( int i = 0; i < vector_size; i++ ) {
		printf_s( "%.4f ", w[i+1] );
	}
	printf_s( "\n" );
	printf_s( "\n" );
}
