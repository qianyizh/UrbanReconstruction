#include "StdAfx.h"
#include "HistGrid.h"
#include "Grid\StreamingGrid.h"
#include "Streaming\SPBReader.h"
#include "Miscs\BPOReader.h"
#include "ParamManager.h"

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

	CStreamingGrid grid;
	CSPBReader reader;
	reader.OpenFile( manager->m_pSPBFile );
	reader.RegisterGrid( & grid );
	reader.ReadHeader();
	reader.CloseFile();

	m_cBoundingBox = grid.m_cBoundingBox;
	m_nGridNumber[ 0 ] = ( int )( m_cBoundingBox.GetLength( 0 ) / manager->m_dbCenterDistance ) + 1;
	m_nGridNumber[ 1 ] = ( int )( m_cBoundingBox.GetLength( 1 ) / manager->m_dbCenterDistance ) + 1;

	m_vecCenter.resize( m_nGridNumber[ 0 ] * m_nGridNumber[ 1 ] );
	m_vecHistogram.resize( m_nGridNumber[ 0 ] * m_nGridNumber[ 1 ] );

	for ( int x = 0; x < m_nGridNumber[ 0 ]; x++ )
		for ( int y = 0; y < m_nGridNumber[ 1 ]; y++ ) {
			m_vecCenter[ x * m_nGridNumber[ 1 ] + y ] = m_cBoundingBox.m_vMin + 
				CVector3D( ( 0.5 + x ) * manager->m_dbCenterDistance, ( 0.5 + y ) * manager->m_dbCenterDistance, 0.0 );
			m_vecHistogram[ x * m_nGridNumber[ 1 ] + y ].Init( manager->m_nHistogramPrecision, 0.0, __pi );
		}

	fprintf_s( stderr, "finished!\n\n" );
}

void CHistGrid::Count()
{

	fprintf_s( stderr, "==================== Processing data ====================\n" );

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

				BPOHeader header = reader.ReadHeader();
				std::vector< COutline > vecOutline( header.number );

				for ( int i = 0; i < header.number; i++ ) {

					reader.ReadOutline( vecOutline[ i ] );

					COutline & outline = vecOutline[ i ];
					for ( int j = 0; j < ( int )outline.m_vecVertex.size(); j++ ) {

						COutlineVertex & v = outline.m_vecVertex[ j ];

						for ( int k = 0; k < ( int )m_vecCenter.size(); k++ ) {

							CHistogram & histogram = m_vecHistogram[ k ];
							CVector3D diff = v.v - m_vecCenter[ k ];
							diff[ 2 ] = 0.0;

							if ( diff.length() <= manager->m_dbHistogramDistance ) {
								histogram.Add( GetAngle( v.d ) );
							}

						}

					}

				}

				reader.CloseFile();

				fprintf_s( stderr, "done.\n" );

			}

			not_finished = FindNextFile( handle, &finder );

		}

	}

	FindClose( handle );

	fprintf_s( stderr, "Generating histograms ... " );

	for ( int i = 0; i < ( int )m_vecHistogram.size(); i++ ) {
		m_vecHistogram[ i ].GetPeak( manager->m_dbHistogramSmoothSigma, manager->m_dbHistogramPeakTolerance );
	}

	fprintf_s( stderr, "finished!\n\n" );
}

double CHistGrid::GetAngle( CVector3D & d ) {
	double temp = atan2( d[ 1 ], d[ 0 ] );
	if ( temp < 0 )
		temp += __pi;
	return temp;
}


void CHistGrid::SaveToHist( char filename[] )
{
	fprintf_s( stderr, "==================== Saving  ====================\n" );
	fprintf_s( stderr, "Saving histograms to %s ... \n", filename );

	CParamManager * manager = CParamManager::GetParamManager();

	m_cWriter.OpenFile( filename );

	m_cWriter.WriteHeader( ( int )m_vecCenter.size(), manager->m_dbCenterDistance, manager->m_dbHistogramDistance );

	for ( int i = 0; i < ( int )m_vecCenter.size(); i++ ) {
		fprintf_s( stderr, "Histogram #%d at (%.2f, %.2f), with peak:\n", i, m_vecCenter[ i ][ 0 ], m_vecCenter[ i ][ 1 ] );
		for ( int j = 0; j < ( int )m_vecHistogram[ i ].m_vecPeak.size(); j++ ) {
			fprintf_s( stderr, " %.3f", m_vecHistogram[ i ].m_vecPeak[ j ] );
		}
		fprintf_s( stderr, "\n" );

		m_cWriter.WriteCenter( m_vecCenter[ i ] );
		m_cWriter.WriteHistogram( m_vecHistogram[ i ] );
	}

	m_cWriter.CloseFile();

	fprintf_s( stderr, "Saving data finished!\n" );
}

//////////////////////////////////////////////////////////////////////////
// Debug use
//////////////////////////////////////////////////////////////////////////

void CHistGrid::Debug_SaveToTxt()
{
	CParamManager * manager = CParamManager::GetParamManager();

	for ( int i = 0; i < ( int )m_vecCenter.size(); i++ ) {

		char filename[ 1024 ];
		memset( filename, 0, 1024 );
		sprintf_s( filename, 1024, "%sHist_%.2f_%.2f.txt", manager->m_pWorkingDir, m_vecCenter[ i ][ 0 ], m_vecCenter[ i ][ 1 ]  );

		FILE * f;
		fopen_s( &f, filename, "w" );

		for ( int j = 0; j < ( int )m_vecHistogram[ i ].m_vecHist.size(); j++ ) {
			fprintf_s( f, "%d ", m_vecHistogram[ i ].m_vecHist[ j ] );
		}

		fprintf_s( f, "\n" );

		for ( int j = 0; j < ( int )m_vecHistogram[ i ].m_vecHist.size(); j++ ) {
			fprintf_s( f, "%.2f ", m_vecHistogram[ i ].m_vecSmoothHist[ j ] );
		}

		fclose( f );

	}
}
