#include "StdAfx.h"
#include "HistGrid.h"
#include "Grid\StreamingGrid.h"
#include "Streaming\SPBReader.h"
#include "nr\nr.h"
#include "nr\nrutil.h"

CHistGrid::CHistGrid(void)
{
	m_nGridNumber[ 0 ] = m_nGridNumber[ 1 ] = 0;
}

CHistGrid::~CHistGrid(void)
{
}

void CHistGrid::Init( char filename[], char histfilename[] )
{
	LoadFromHist( histfilename );

	CStreamingGrid grid;
	CSPBReader reader;
	reader.OpenFile( filename );
	reader.RegisterGrid( & grid );
	reader.ReadHeader();
	reader.CloseFile();

	m_cBoundingBox = grid.m_cBoundingBox;
	m_nGridNumber[ 0 ] = ( int )( m_cBoundingBox.GetLength( 0 ) / m_cHeader.center_distance ) + 1;
	m_nGridNumber[ 1 ] = ( int )( m_cBoundingBox.GetLength( 1 ) / m_cHeader.center_distance ) + 1;
}

void CHistGrid::LoadFromHist( char filename[] )
{
	m_cReader.OpenFile( filename );

	m_cHeader = m_cReader.ReadHeader();

	m_vecCenter.resize( m_cHeader.number );
	m_vecHistogram.resize( m_cHeader.number );

	for ( int i = 0; i < m_cHeader.number; i++ ) {
		m_cReader.ReadCenter( m_vecCenter[ i ] );
		m_cReader.ReadHistogram( m_vecHistogram[ i ] );
	}

	m_cReader.CloseFile();
}

CHistogram & CHistGrid::LocateHistogram( CVector3D & v )
{
	CVector3D diff = v - m_cBoundingBox.m_vMin;
	int x = ( int )( diff[0] / m_cHeader.center_distance );
	if ( x < 0 )
		x = 0;
	if ( x >= m_nGridNumber[ 0 ] )
		x = m_nGridNumber[ 0 ] - 1;

	int y = ( int )( diff[1] / m_cHeader.center_distance );
	if ( y < 0 )
		y = 0;
	if ( y >= m_nGridNumber[ 1 ] )
		y = m_nGridNumber[ 1 ] - 1;

	return m_vecHistogram[ x * m_nGridNumber[ 1 ] + y ];
}

void CHistGrid::Simplify( CDCContourer & contourer, double error_tolerance, double minimum_length )
{
	m_pMesh = & contourer.m_cMesh;
	m_pBoundary = & contourer.m_cBoundary;
	m_pGrid = contourer.m_pGrid;
	m_dbErrorTolerance = error_tolerance;
	m_dbMinimumLength = minimum_length;

	m_pBoundary->Init( m_pMesh );

	for ( int i = 0; i < ( int )m_pBoundary->m_vecBoundary.size(); i++ ) {
		FittingPrincipalDirection( m_pBoundary->m_vecBoundarySeq[ i ] );
		FixVertices( m_pBoundary->m_vecBoundarySeq[ i ] );
	}
}

void CHistGrid::FittingPrincipalDirection( int ibdr )
{
	CMeshBoundary::CAuxBoundary & bdr = m_pBoundary->m_vecBoundary[ ibdr ];
	int loop_num = ( int )bdr.vi.size();

	std::vector< AuxVertex > verts( bdr.vi.size() );
	std::vector< AuxLine > lines;

	bool line_added = true;

	while ( line_added ) {
		line_added = false;

		AuxLine max_line;
		max_line.acc_length = 0.0;
		max_line.acc_error = 0.0;

		for ( int i = 0; i < ( int )bdr.vi.size(); i++ ) {

			int vi = bdr.vi[ i ];
			if ( verts[ i ].t == FT_Free || ( verts[ i ].t == FT_HalfFixed && verts[ i ].li[ 1 ] != -1 ) ) {

				CVector3D loc( m_pMesh->m_vecVertex[ vi ].v );
				std::vector< double > & peak = LocateHistogram( loc ).m_vecPeak;

				for ( int j = 0; j < ( int )peak.size(); j++ ) {
					CLine temp_line;
					temp_line.p = CVector3D( m_pMesh->m_vecVertex[ vi ].v );
					temp_line.d = CVector3D( cos( peak[j] ), sin( peak[j] ), 0.0 );
					int k_forward = 0, k_backward = 0;

					int iteration_step = m_pBoundary->m_vecGroupInfo[ vi ].fixed ? 1 : 3;
					double acc_length = 0.0;
					double acc_error = 0.0;

					for ( int step = 0; step < iteration_step; step++ ) {

						acc_length = 0.0;
						acc_error = 0.0;

						if ( ( acc_error = temp_line.dis( CVector3D( m_pMesh->m_vecVertex[ vi ].v ) ) ) > m_dbErrorTolerance ) {
							k_forward = k_backward = 0;
							break;
						}

						for ( k_forward = 1; k_forward < loop_num; k_forward++ ) {
							int idx = ( i + k_forward ) % loop_num;
							double error = temp_line.dis( CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ idx ] ].v ) );
							if ( m_pBoundary->m_vecGroupInfo[ bdr.vi[ idx ] ].fixed
								|| verts[ idx ].t == FT_Fixed
								|| ( verts[ idx ].t == FT_HalfFixed && verts[ idx ].li[1] != -1 )
								|| error > m_dbErrorTolerance )
							{
								break;
							} else if ( verts[ idx ].t == FT_HalfFixed )
							{
								if ( lines[ verts[ idx ].li[ 0 ] ].pi == j ) {
									break;
								} else {
									CVector3D vdiff = CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ idx ] ].v ) - CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ ( i + k_forward - 1 ) % loop_num ] ].v );
									vdiff.pVec[2] = 0.0;
									acc_length += vdiff.length();
									acc_error += error;
									k_forward++;
									break;
								}
							} else {
								CVector3D vdiff = CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ idx ] ].v ) - CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ ( i + k_forward - 1 ) % loop_num ] ].v );
								vdiff.pVec[2] = 0.0;
								acc_length += vdiff.length();
								acc_error += error;
							}
						}

						if ( k_forward < loop_num ) {
							for ( k_backward = -1; k_backward > -loop_num; k_backward-- ) {
								int idx = ( i + k_backward + loop_num ) % loop_num;
								double error = temp_line.dis( CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ idx ] ].v ) );
								if ( m_pBoundary->m_vecGroupInfo[ bdr.vi[ idx ] ].fixed
									|| verts[ idx ].t == FT_Fixed
									|| ( verts[ idx ].t == FT_HalfFixed && verts[ idx ].li[1] != -1 )
									|| error > m_dbErrorTolerance )
								{
									break;
								} else if ( verts[ idx ].t == FT_HalfFixed )
								{
									if ( lines[ verts[ idx ].li[ 0 ] ].pi == j ) {
										break;
									} else {
										CVector3D vdiff = CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ idx ] ].v ) - CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ ( i + k_backward + loop_num + 1 ) % loop_num ] ].v );
										vdiff.pVec[2] = 0.0;
										acc_length += vdiff.length();
										acc_error += error;
										k_backward--;
										break;
									}
								} else {
									CVector3D vdiff = CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ idx ] ].v ) - CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ ( i + k_backward + loop_num + 1 ) % loop_num ] ].v );
									vdiff.pVec[2] = 0.0;
									acc_length += vdiff.length();
									acc_error += error;
								}
							}
							k_backward++;
						}

						if ( m_pBoundary->m_vecGroupInfo[ vi ].fixed == false ) {
							// use iteration
							// update temp_line.p
							if ( acc_length > m_dbMinimumLength ) {
								temp_line.p = CVector3D( 0.0, 0.0, 0.0 );
								for ( int k = k_backward; k < k_forward; k++ ) {
									temp_line.p += CVector3D( m_pMesh->m_vecVertex[ bdr.vi[ ( i + k + loop_num ) % loop_num ] ].v );
								}
								temp_line.p /= ( double )( k_forward - k_backward );
							}
						}
					}

					int k_number = k_forward - k_backward;
					if ( acc_length > m_dbMinimumLength && IsBetterMaxLine( acc_length, acc_error, max_line ) ) {
						//printf_s( "%8.6f\n", peak[ j ] );
						max_line.l = temp_line;
						max_line.pi = j;
						max_line.acc_length = acc_length;
						max_line.vi.clear();
						for ( int k = k_backward; k < k_forward; k++ ) {
							max_line.vi.push_back( ( i + k + loop_num ) % loop_num );
						}
					}
				}
			}
		}

		if ( max_line.acc_length > m_dbMinimumLength && ( int )max_line.vi.size() < loop_num ) {
			// push max_line to aux


			int line_idx = ( int )lines.size();
			lines.push_back( max_line );

			for ( int i = 0; i <= ( int )( max_line.vi.size() - 1 ); i += ( int )( max_line.vi.size() - 1 ) ) {
				verts[ max_line.vi[ i ] ].t = FT_HalfFixed;
				if ( verts[ max_line.vi[ i ] ].li[ 0 ] == -1 ) {
					verts[ max_line.vi[ i ] ].li[ 0 ] = line_idx;
				} else {
					verts[ max_line.vi[ i ] ].li[ 1 ] = line_idx;
				}
			}

			for ( int i = 1; i < ( int )max_line.vi.size() - 1; i++ ) {
				verts[ max_line.vi[ i ] ].t = FT_Fixed;
				verts[ max_line.vi[ i ] ].li[ 0 ] = line_idx;
			}

			line_added = true;

		}
	}

	FittingSimplify( bdr, verts, lines );
}

void CHistGrid::FittingSimplify( CMeshBoundary::CAuxBoundary & bdr, std::vector< AuxVertex > & verts, std::vector< AuxLine > & lines )
{
	for ( int i = 0; i < ( int )bdr.vi.size(); i++ ) {
		int vi = bdr.vi[ i ];
		CVector3D v( m_pMesh->m_vecVertex[ vi ].v );

		if ( verts[ i ].t == FT_Fixed || ( verts[ i ].t == FT_HalfFixed && verts[ i ].li[ 1 ] == -1 ) ) {
			v = lines[ verts[ i ].li[ 0 ] ].l.project( v );
		} else if ( verts[ i ].t == FT_HalfFixed ) {
			CVector3D inter = lines[ verts[ i ].li[ 0 ] ].l ^ lines[ verts[ i ].li[ 1 ] ].l;
			inter.pVec[ 2 ] = v.pVec[ 2 ];
			if ( ( inter - v ).length() < m_dbErrorTolerance * 1.414 ) {
				v = inter;
			}
		} else {
			// do nothing
		}

		m_pMesh->m_vecVertex[ vi ].v[ 0 ] = v.pVec[ 0 ];
		m_pMesh->m_vecVertex[ vi ].v[ 1 ] = v.pVec[ 1 ];
	}
}

void CHistGrid::FixVertices( int ibdr )
{
	CMeshBoundary::CAuxBoundary & bdr = m_pBoundary->m_vecBoundary[ ibdr ];

	for ( int i = 0; i < ( int )bdr.vi.size(); i++ ) {
		int vi = bdr.vi[ i ];
		CMeshBoundary::CVertexGroupInfo & hi = m_pBoundary->m_vecGroupInfo[ vi ];

		for ( int j = 0; j < hi.number; j++ ) {
			int vvi = vi - hi.index + j;
			m_pMesh->m_vecVertex[ vvi ].v[ 0 ] = m_pMesh->m_vecVertex[ vi ].v[ 0 ];
			m_pMesh->m_vecVertex[ vvi ].v[ 1 ] = m_pMesh->m_vecVertex[ vi ].v[ 1 ];
			m_pBoundary->m_vecGroupInfo[ vvi ].fixed = true;
		}
	}
}
