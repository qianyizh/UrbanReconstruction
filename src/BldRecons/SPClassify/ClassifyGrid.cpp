#include "StdAfx.h"
#include "ClassifyGrid.h"
#include "ParamManager.h"

#include "FeatureCalculator.h"
#include "Miscs\TimeMeter.h"

CClassifyGrid::CClassifyGrid(void)
{
}

CClassifyGrid::~CClassifyGrid(void)
{
}

void CClassifyGrid::Classify()
{
	CParamManager * manager = CParamManager::GetParamManager();
	CTimeMeter timer;
	timer.Start();

	fprintf_s( stderr, "==================== Classify points ====================\n" );

	Init();

	fprintf_s( stderr, "Initialize ... finished.\n" );

	fprintf_s( stderr, "Processing progress ... " );
	InitPrintProgress();

	int index;
	while ( ( index = ReadNextChunk() ) != -1 ) {

		PrintProgress(  );

		CClassifyChunk * chunk = m_vecPointer[ index ];
		chunk->BuildGridIndex();

		// check neighborhood of index chunk
		m_vecState[ chunk->m_iIndex ] = CClassifyChunk::CCS_Read;
		NotifyCCSRead( chunk->m_iX, chunk->m_iY );

		if ( manager->m_bDebugPrintOut ) {
			PrintGridState( manager->m_pDebugPrintOutFile );
		}

	}

	fprintf_s( stderr, " ... done!\n" );

	Fin();

	timer.End();
	timer.Print();

	fprintf_s( stderr, ".\n" );
}

void CClassifyGrid::Init()
{
	CParamManager * manager = CParamManager::GetParamManager();
	m_cReader.OpenFile( manager->m_pInputFile );
	m_cReader.RegisterGrid( this );
	m_cReader.ReadHeader();

	m_cWriter.OpenFile( manager->m_pOutputFile );
	m_cWriter.RegisterGrid( this );
	m_cWriter.WriteHeader();

	m_vecPointer.clear();
	m_vecPointer.resize( m_nSideNumber * m_nSideNumber, NULL );
	m_vecState.clear();
	m_vecState.resize( m_nSideNumber * m_nSideNumber, CClassifyChunk::CCS_New );

	m_nTotalNumber = m_nNoiseNumber = m_nBuildingNumber = m_nTreeNumber = 0;
}

void CClassifyGrid::Fin()
{
	m_cWriter.WriteEOF();
	m_cWriter.CloseFile();

	m_cReader.CloseFile();

	fprintf_s( stderr, "Summary :\n\tBuilding points - %d\n\tTree points - %d\n\tNoise points - %d\nTotally %d points written in ", m_nBuildingNumber, m_nTreeNumber, m_nNoiseNumber, m_nTotalNumber );
}

int CClassifyGrid::ReadNextChunk()
{
	// return the index of the finalized chunk
	// return -1 indicating a EOF has been read
	bool bNoneChunkEnd = true;
	int index;

	while ( bNoneChunkEnd ) {

		if ( m_cReader.ReadNextElement() ) {		// read chunk information

			SPCell * cell = m_cReader.GetCell();

			switch ( cell->type ) {
			case 0:									// begin chunk
				m_vecPointer[ cell->chunk_index ] = new CClassifyChunk( cell->chunk_index, cell->point_number, this );
				break;
			case 1:									// end chunk
				bNoneChunkEnd = false;
				index = cell->chunk_index;
				break;
			case -1:								// EOF
				bNoneChunkEnd = false;
				index = -1;
				break;
			}

		} else {

			SPPoint * point = m_cReader.GetPoint();
			CVector3D v( point->pos[0], point->pos[1], point->pos[2] );
			CClassifyChunk * chunk = m_vecPointer[ Index( v ) ];
			chunk->PushPoint( point );

			IncReadNumber();

		}
	}

	return index;
}

//////////////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////////////

void CClassifyGrid::NotifyCCSRead( int ix, int iy )
{
	for ( int x = MinBound( ix ); x <= MaxBound( ix ); x++ )
		for ( int y = MinBound( iy ); y <= MaxBound( iy ); y++ ) {

			bool bAllRead = true;

			for ( int xx = MinBound( x ); xx <= MaxBound( x ) && bAllRead; xx++ )
				for ( int yy = MinBound( y ); yy <= MaxBound( y ) && bAllRead; yy++ ) {
					if ( InBound( xx, yy ) && CheckCCSRead( Index( xx, yy ) ) == false ) {
						bAllRead = false;
					}
				}

			if ( bAllRead ) {
				NormalChunk( m_vecPointer[ Index( x, y ) ] );
			}
		}
}

void CClassifyGrid::NotifyCCSNormaled( int ix, int iy )
{
	for ( int x = MinBound( ix ); x <= MaxBound( ix ); x++ )
		for ( int y = MinBound( iy ); y <= MaxBound( iy ); y++ ) {

			bool bAllNormaled = true;

			for ( int xx = MinBound( x ); xx <= MaxBound( x ) && bAllNormaled; xx++ )
				for ( int yy = MinBound( y ); yy <= MaxBound( y ) && bAllNormaled; yy++ ) {
					if ( InBound( xx, yy ) && CheckCCSNormaled( Index( xx, yy ) ) == false ) {
						bAllNormaled = false;
					}
				}

			if ( bAllNormaled ) {
				ClassifyChunk( m_vecPointer[ Index( x, y ) ] );
			}
		}
}

void CClassifyGrid::NotifyCCSClassified( int ix, int iy )
{
	for ( int x = MinBound( ix ); x <= MaxBound( ix ); x++ )
		for ( int y = MinBound( iy ); y <= MaxBound( iy ); y++ ) {

			bool bAllClassified = true;

			for ( int xx = MinBound( x ); xx <= MaxBound( x ) && bAllClassified; xx++ )
				for ( int yy = MinBound( y ); yy <= MaxBound( y ) && bAllClassified; yy++ ) {
					if ( InBound( xx, yy ) && CheckCCSClassified( Index( xx, yy ) ) == false ) {
						bAllClassified = false;
					}
				}

			if ( bAllClassified ) {
				RefineChunk( m_vecPointer[ Index( x, y ) ] );
			}
		}
}

void CClassifyGrid::NotifyCCSRefined( int ix, int iy )
{
	for ( int x = MinBound( ix ); x <= MaxBound( ix ); x++ )
		for ( int y = MinBound( iy ); y <= MaxBound( iy ); y++ ) {

			bool bAllRefined = true;

			for ( int xx = MinBound( x ); xx <= MaxBound( x ) && bAllRefined; xx++ )
				for ( int yy = MinBound( y ); yy <= MaxBound( y ) && bAllRefined; yy++ ) {
					if ( InBound( xx, yy ) && CheckCCSRefined( Index( xx, yy ) ) == false ) {
						bAllRefined = false;
					}
				}

			if ( bAllRefined ) {
				WriteChunk( m_vecPointer[ Index( x, y ) ] );
				delete m_vecPointer[ Index( x, y ) ];
				m_vecPointer[ Index( x, y ) ] = NULL;
			}
		}
}

void CClassifyGrid::NormalChunk( CClassifyChunk * chunk )
{
	for ( int x = 0; x < m_nUnitNumber[ 0 ]; x++ )
		for ( int y = 0; y < m_nUnitNumber[ 1 ]; y++ ) {
			PointDataVector & cell_vector = chunk->m_vecGridIndex[ chunk->Index( x, y ) ];
			for ( int i = 0; i < ( int )cell_vector.size(); i++ ) {
				PointData & point = *( cell_vector[ i ] );
				GetNeighbor( point, 1, chunk, x, y );
				CFeatureCalculator::ComputeCoVariance( point, m_vecPointData );
			}
		}

	m_vecState[ chunk->m_iIndex ] = chunk->CCS_Normaled;
	NotifyCCSNormaled( chunk->m_iX, chunk->m_iY );
}

void CClassifyGrid::ClassifyChunk( CClassifyChunk * chunk )
{
	for ( int x = 0; x < m_nUnitNumber[ 0 ]; x++ )
		for ( int y = 0; y < m_nUnitNumber[ 1 ]; y++ ) {
			PointDataVector & cell_vector = chunk->m_vecGridIndex[ chunk->Index( x, y ) ];
			for ( int i = 0; i < ( int )cell_vector.size(); i++ ) {
				PointData & point = *( cell_vector[ i ] );
				GetNeighbor( point, 2, chunk, x, y );
				CFeatureCalculator::ComputeNormalCoVariance( point, m_vecPointData );
			}
		}

	m_vecState[ chunk->m_iIndex ] = chunk->CCS_Classified;
	NotifyCCSClassified( chunk->m_iX, chunk->m_iY );
}

void CClassifyGrid::RefineChunk( CClassifyChunk * chunk )
{
	for ( int x = 0; x < m_nUnitNumber[ 0 ]; x++ )
		for ( int y = 0; y < m_nUnitNumber[ 1 ]; y++ ) {
			PointDataVector & cell_vector = chunk->m_vecGridIndex[ chunk->Index( x, y ) ];
			for ( int i = 0; i < ( int )cell_vector.size(); i++ ) {
				PointData & point = *( cell_vector[ i ] );
				GetNeighbor( point, 2, chunk, x, y );
				CFeatureCalculator::RefineClassification( point, m_vecPointData );
			}
		}

	m_vecState[ chunk->m_iIndex ] = chunk->CCS_Refined;
	NotifyCCSRefined( chunk->m_iX, chunk->m_iY );
}

void CClassifyGrid::WriteChunk( CClassifyChunk *chunk )
{
	CParamManager * manager = CParamManager::GetParamManager();

	m_cWriter.WriteBeginCell( chunk->m_iIndex, ( int )chunk->m_vecPointData.size() );

	for ( int i = 0; i < ( int )( chunk->m_vecPointData.size() ); i++ ) {
		PointData & point = chunk->m_vecPointData[ i ];

		m_nTotalNumber++;

		if ( point.type == PT_Noise ) {
			m_nNoiseNumber++;
		} else if ( point.buildingness > manager->m_cParam.c_refine ) {
			point.type = PT_Building;
			m_nBuildingNumber++;
		} else {
			point.type = PT_Tree;
			m_nTreeNumber++;
		}

		m_cWriter.WritePoint( point );
	}

	m_cWriter.WriteFinalizeCell( chunk->m_iIndex, ( int )chunk->m_vecPointData.size() );

	m_vecState[ chunk->m_iIndex ] = chunk->CCS_Written;
}

//////////////////////////////////////////////////////////////////////////
// auxiliary functions
//////////////////////////////////////////////////////////////////////////

void CClassifyGrid::GetNeighbor( PointData & point, int dis, CClassifyChunk * chunk, int x, int y )
{
	m_vecPointData.clear();
	double distance = m_dbGridLength * dis;
	double distance2 = distance * distance;

	for ( int xx = x - dis; xx <= x + dis; xx++ )
		for ( int yy = y - dis; yy <= y + dis; yy++ ) {
			int i = xx;
			int j = yy;
			CClassifyChunk * data_chunk = chunk;
			while ( i < 0 && data_chunk != NULL ) {
				i += m_nUnitNumber[ 0 ];
				data_chunk = InBound( data_chunk->m_iX - 1, data_chunk->m_iY ) ? m_vecPointer[ Index( data_chunk->m_iX - 1, data_chunk->m_iY ) ] : NULL;
			}
			while ( i > m_nUnitNumber[ 0 ] - 1 && data_chunk != NULL ) {
				i -= m_nUnitNumber[ 0 ];
				data_chunk = InBound( data_chunk->m_iX + 1, data_chunk->m_iY ) ? m_vecPointer[ Index( data_chunk->m_iX + 1, data_chunk->m_iY ) ] : NULL;
			}
			while ( j < 0 && data_chunk != NULL ) {
				j += m_nUnitNumber[ 1 ];
				data_chunk = InBound( data_chunk->m_iX, data_chunk->m_iY - 1 ) ? m_vecPointer[ Index( data_chunk->m_iX, data_chunk->m_iY - 1 ) ] : NULL;
			}
			while ( j > m_nUnitNumber[ 1 ] - 1 && data_chunk != NULL ) {
				j -= m_nUnitNumber[ 1 ];
				data_chunk = InBound( data_chunk->m_iX, data_chunk->m_iY + 1 ) ? m_vecPointer[ Index( data_chunk->m_iX, data_chunk->m_iY + 1 ) ] : NULL;
			}

			if ( data_chunk != NULL ) {
				PointDataVector & cell_vector = data_chunk->m_vecGridIndex[ data_chunk->Index( i, j ) ];
				for ( int k = 0; k < ( int )cell_vector.size(); k++ ) {
					CVector3D diff = point.v - ( cell_vector[ k ] )->v;
					if ( diff.length2() < distance2 ) {
						m_vecPointData.push_back( cell_vector[ k ] );
					}
				}
			}
		}
}

void CClassifyGrid::PrintGridState( const char filename[] )
{
	FILE * file;
	fopen_s( &file, filename, "a" );

	for ( int i = 0; i < m_nSideNumber; i++ ) {
		for ( int j = 0; j < m_nSideNumber; j++ ) {
			int idx = Index( i, j );
			fprintf_s( file, "%d ", m_vecState[ idx ] );
		}
		fprintf_s( file, "\n" );
	}
	
	fclose( file );
}
