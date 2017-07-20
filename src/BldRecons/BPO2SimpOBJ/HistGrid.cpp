#include "StdAfx.h"
#include "HistGrid.h"
#include "Grid\StreamingGrid.h"
#include "Streaming\SPBReader.h"
#include "Miscs\BPOReader.h"
#include "Miscs\OBJWriter.h"
#include "ParamManager.h"
#include "AuxOutline.h"
#include "OutlineSimplifier.h"

CHistGrid::CHistGrid(void)
{
	m_nGridNumber[ 0 ] = m_nGridNumber[ 1 ] = 0;
}

CHistGrid::~CHistGrid(void)
{
}

void CHistGrid::Init()
{
	fprintf_s( stderr, "==================== Initialization ====================\n" );
	fprintf_s( stderr, "Initialize ... " );

	CParamManager * manager = CParamManager::GetParamManager();

	LoadFromHist( manager->m_pHistFile );

	CStreamingGrid grid;
	CSPBReader reader;
	reader.OpenFile( manager->m_pSPBFile );
	reader.RegisterGrid( & grid );
	reader.ReadHeader();
	reader.CloseFile();

	m_cBoundingBox = grid.m_cBoundingBox;
	m_nGridNumber[ 0 ] = ( int )( m_cBoundingBox.GetLength( 0 ) / m_cHeader.center_distance ) + 1;
	m_nGridNumber[ 1 ] = ( int )( m_cBoundingBox.GetLength( 1 ) / m_cHeader.center_distance ) + 1;

	fprintf_s( stderr, "finished!\n\n" );
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

void CHistGrid::Simplify()
{

	fprintf_s( stderr, "==================== Simplify outlines ====================\n" );

	CParamManager * manager = CParamManager::GetParamManager();

	char pSearch[ 1024 ];
	sprintf_s( pSearch, 1024, "%s*.bpo", manager->m_pWorkingDir );

	WIN32_FIND_DATA finder;
	HANDLE handle = FindFirstFile( pSearch, & finder );
	BOOL not_finished = ( handle != INVALID_HANDLE_VALUE );

	while ( not_finished ) {

		if ( strcmp( finder.cFileName, "." ) != 0 && strcmp( finder.cFileName, ".." ) != 0 ) {

			if ( ( finder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != FILE_ATTRIBUTE_DIRECTORY ) {

				fprintf_s( stderr, "Processing %s ... ", finder.cFileName );

				char filename[ 1024 ];
				memset( filename, 0, 1024 );
				sprintf_s( filename, 1024, "%s%s", manager->m_pWorkingDir, finder.cFileName );
				CBPOReader reader;
				reader.OpenFile( filename );

				char truename[ 1024 ];
				memset( truename, 0, 1024 );
				strncat_s( truename, 1024, filename, strlen( filename ) - 3 );
				sprintf_s( filename, "%sobj", truename );
				COBJWriter writer;
				writer.OpenFile( filename );

				BPOHeader header = reader.ReadHeader();
				std::vector< COutline > vecOutline( header.number );
				std::vector< COutline > vecSimpOutline( header.number );

				for ( int i = 0; i < header.number; i++ ) {
					reader.ReadOutline( vecOutline[ i ] );
				}

				reader.CloseFile();

				SimplifyOutline( vecOutline, vecSimpOutline );

				writer.WriteHeader();
				for ( int i = 0; i < header.number; i++ ) {
					writer.WriteOutline( vecSimpOutline[ i ], header.ground_z, manager->m_bWallRectangle );
				}
				writer.CloseFile();

				fprintf_s( stderr, "done.\n" );

			}

			not_finished = FindNextFile( handle, &finder );

		}

	}

	FindClose( handle );

}

void CHistGrid::SimplifyOutline( std::vector< COutline > & vecOutline, std::vector< COutline > & vecSimpOutline )
{
	CParamManager * manager = CParamManager::GetParamManager();

	std::vector< CAuxOutline > vecAuxOutline( vecOutline.size() );

	for ( int i = 0; i < ( int )vecOutline.size(); i++ ) {

		COutline & outline = vecOutline[ i ];
		CAuxOutline & aux_outline = vecAuxOutline[ i ];
		aux_outline.RegisterOutline( & outline );
		std::vector< int > vertex_loop;

		int touched_num = 0;
		int num = ( int )outline.m_vecVertex.size();
		while ( touched_num < num ) {
			int idx;
			for ( idx = 0; idx < num; idx++ ) {
				if ( aux_outline.m_vecLineIndex[ idx ] == -1 )
					break;
			}

			double angle_sum = 0.0;
			const double pi = 3.1415926535897932384626;
			vertex_loop.clear();
			vertex_loop.push_back( idx );
			idx = outline.m_vecVertex[ idx ].next;
			
			while ( idx != vertex_loop[ 0 ] ) {
				vertex_loop.push_back( idx );
				idx = outline.m_vecVertex[ idx ].next;
			}
			touched_num += ( int )vertex_loop.size();

			int fitting_line_cnt = FittingPrincipalDirection( outline, aux_outline, vertex_loop );
			if ( fitting_line_cnt == 0 ) {
				vertex_loop.push_back( idx );
				COutlineSimplifier::Simplify( outline, aux_outline, vertex_loop, manager->m_dbLineErrorTolerance, manager->m_nFittingMode );
			} else {
				for ( int i = ( int )aux_outline.m_vecLine.size() - fitting_line_cnt; i < ( int )aux_outline.m_vecLine.size(); i++ ) {
					AuxLine & line = aux_outline.m_vecLine[ i ];
					std::vector< int > vlist;
					int seed = outline.m_vecVertex[ line.vi.back() ].next;
					while ( aux_outline.m_vecLineIndex[ seed ] == -1 ) {
						vlist.push_back( seed );
						seed = outline.m_vecVertex[ seed ].next;
					}

					if ( vlist.size() > 0 ) {
						vlist.push_back( seed );
						COutlineSimplifier::Simplify( outline, aux_outline, vlist, manager->m_dbLineErrorTolerance, manager->m_nFittingMode );
					}
				}
			}

		}

	}

	if ( manager->m_bLineSnapping ) {
		SnapLines( vecAuxOutline, vecOutline );
	}

	for ( int i = 0; i < ( int )vecAuxOutline.size(); i++ ) {
		vecAuxOutline[ i ].ProduceOutline( vecSimpOutline[ i ] );
	}

}

int CHistGrid::FittingPrincipalDirection( COutline & outline, CAuxOutline & aux, std::vector<int> & vertex_loop )
{
	CParamManager * manager = CParamManager::GetParamManager();

	int cnt = 0;
	int loop_num = ( int )vertex_loop.size();
	bool line_added = true;

	while ( line_added ) {
		line_added = false;

		AuxLine max_line;

		for ( int i = 0; i < loop_num; i++ ) {

			int vi = vertex_loop[ i ];
			if ( aux.m_vecLineIndex[ vi ] == -1 ) {

				std::vector< double > & peak = LocateHistogram( outline.m_vecVertex[ vi ].v ).m_vecPeak;

				for ( int j = 0; j < ( int )peak.size(); j++ ) {

					CLine temp_line;
					temp_line.p = outline.m_vecVertex[ vi ].v;
					temp_line.d = CVector3D( cos( peak[j] ), sin( peak[j] ), 0.0 );
					int k_forward = 0, k_backward = 0;

					for ( int step = 0; step < manager->m_nLineFitting_IterationStep; step++ ) {

						for ( k_forward = 0; k_forward < loop_num; k_forward++ ) {

							int idx = vertex_loop[ ( i + k_forward ) % loop_num ];
							if ( aux.m_vecLineIndex[ idx ] != -1
								|| temp_line.dis( outline.m_vecVertex[ idx ].v ) > manager->m_dbLineErrorTolerance ) {
									break;
							}

						}
						// go forward
						// vertex_loop[ ( i + k_forward ) % loop_num ] is the first one which does NOT satisfy
						// if all the points satisfy, then k_forward == loop_num

						if ( k_forward < loop_num ) {
							for ( k_backward = 0; k_backward > - loop_num; k_backward-- ) {

								int idx = vertex_loop[ ( i + k_backward + loop_num ) % loop_num ];
								if ( aux.m_vecLineIndex[ idx ] != -1
									|| temp_line.dis( outline.m_vecVertex[ idx ].v ) > manager->m_dbLineErrorTolerance ) {
										break;
								}

							}
							k_backward++;
						}
						// go backward
						// vertex_loop[ ( i + k_backward ) % loop_num ] is the last one which does satisfy
						// if all the points satisfy, then k_backward == 0

						if ( k_forward - k_backward >= manager->m_nSegmentMinimumPointNumber ) {
							// update temp_line.p
							temp_line.p = CVector3D( 0.0, 0.0, 0.0 );
							for ( int k = k_backward; k < k_forward; k++ ) {
								temp_line.p += outline.m_vecVertex[ vertex_loop[ ( i + k + loop_num ) % loop_num ] ].v;
							}
							temp_line.p /= ( double )( k_forward - k_backward );
						}

					}

					int k_number = k_forward - k_backward;
					if ( k_number >= manager->m_nSegmentMinimumPointNumber && k_number > ( int )max_line.vi.size()) {
						max_line.line = temp_line;
						max_line.vi.clear();
						for ( int k = k_backward; k < k_forward; k++ ) {
							max_line.vi.push_back( vertex_loop[ ( i + k + loop_num ) % loop_num ] );
						}
					}

				}

			}

		}

		if ( ( int )max_line.vi.size() >= manager->m_nSegmentMinimumPointNumber ) {
			// push max_line to aux
			int line_idx = ( int )aux.m_vecLine.size();
			aux.m_vecLine.push_back( max_line );
			for ( int i = 0; i < ( int )max_line.vi.size(); i++ ) {
				aux.m_vecLineIndex[ max_line.vi[ i ] ] = line_idx;
			}
			cnt++;
			line_added = true;
		}

	}

	return cnt;
}

void CHistGrid::SnapLines( std::vector< CAuxOutline > & vecAuxOutline, std::vector< COutline > & vecOutline )
{
	CParamManager * manager = CParamManager::GetParamManager();

	for ( int i = 0; i < ( int )vecAuxOutline.size(); i++ ) {
		for ( int j = i + 1; j < ( int )vecAuxOutline.size(); j++ ) {
			for ( int ii = 0; ii < ( int )vecAuxOutline[ i ].m_vecLine.size(); ii++ ) {
				for ( int jj = 0; jj < ( int )vecAuxOutline[ j ].m_vecLine.size(); jj++ ) {
					AuxLine & l0 = vecAuxOutline[ i ].m_vecLine[ ii ];
					AuxLine & l1 = vecAuxOutline[ j ].m_vecLine[ jj ];

					if ( ( !l0.snapped || !l1.snapped ) 
						&& CompareVector2D( l0.line.d, l1.line.d ) == 0
						&& l0.line.dis( l1.line.p ) < manager->m_dbLineErrorTolerance )
					{
						// now need to check if there is overlapping
						CVector3D v_end[2][2] = { 
							{ l0.line.project( vecOutline[ i ].m_vecVertex[ l0.vi[ 0 ] ].v ), l0.line.project( vecOutline[ i ].m_vecVertex[ l0.vi.back() ].v ) },
							{ l0.line.project( vecOutline[ j ].m_vecVertex[ l1.vi[ 0 ] ].v ), l0.line.project( vecOutline[ j ].m_vecVertex[ l1.vi.back() ].v ) }
						};
						int small_vi[2] = { 
							( 1 + CompareVector2D( v_end[0][0], v_end[0][1] ) ) / 2,
							( 1 + CompareVector2D( v_end[1][0], v_end[1][1] ) ) / 2
						};
						int large_vi[2] = { 1 - small_vi[0], 1 - small_vi[1] };
						if ( CompareVector2D( v_end[0][small_vi[0]], v_end[1][large_vi[1]] ) == -1
							&& CompareVector2D( v_end[1][small_vi[1]], v_end[0][large_vi[0]] ) == -1 )
						{
							// now we need to snap
							if ( l0.snapped ) {
								l1.line.p = l0.line.project( l1.line.p );
							} else if ( l1.snapped ) {
								l0.line.p = l1.line.project( l0.line.p );
							} else {
								if ( l0.vi.size() > l1.vi.size() ) {
									l1.line.p = l0.line.project( l1.line.p );
								} else {
									l0.line.p = l1.line.project( l0.line.p );
								}
							}
						}
					}
				}
			}
		}
	}
}