#include "StdAfx.h"
#include "ConvertGrid.h"
#include "ParamManager.h"
#include "Miscs\TimeMeter.h"

CConvertGrid::CConvertGrid(void)
{
}

CConvertGrid::~CConvertGrid(void)
{
}

void CConvertGrid::Convert()
{

	CParamManager * manager = CParamManager::GetParamManager();
	CTimeMeter timer;
	timer.Start();
	fprintf_s( stderr, "==================== Pass 1, count last cell ====================\n" );

	InitCount();

	fprintf_s( stderr, "Processing progress ... " );
	InitPrintProgress();

	int index;

	while ( ( index = ReadNextChunk() ) != -1 ) {

		PrintProgress(  );

		CConvertChunk * chunk = m_vecPointer[ index ];

		CountChunk( chunk );

		delete m_vecPointer[ index ];
		m_vecPointer[ index ] = NULL;

	}

	fprintf_s( stderr, " ... done!\n" );

	FinCount();

	fprintf_s( stderr, "Total processing time is " );
	timer.End();
	timer.Print();
	fprintf_s( stderr, ".\n\n" );

	timer.Start();
	fprintf_s( stderr, "==================== Pass 2, output ====================\n" );

	InitWrite();

	fprintf_s( stderr, "Processing progress ... " );
	InitPrintProgress();

	while ( ( index = ReadNextChunk() ) != -1 ) {

		PrintProgress(  );

		CConvertChunk * chunk = m_vecPointer[ index ];

		WriteChunk( chunk );

		delete m_vecPointer[ index ];
		m_vecPointer[ index ] = NULL;

	}

	fprintf_s( stderr, " ... done!\n" );

	FinWrite();

	fprintf_s( stderr, "Total processing time is " );
	timer.End();
	timer.Print();
	fprintf_s( stderr, ".\n" );

}

int CConvertGrid::ReadNextChunk()
{
	// return the index of the finalized chunk
	// return -1 indicating a EOF has been read
	bool bNoneChunkEnd = true;
	int index;

	while ( bNoneChunkEnd ) {

		if ( m_cReader.ReadNextElement() ) {		// read chunk information

			SPBCell * cell = m_cReader.GetCell();

			switch ( cell->type ) {
			case 0:									// begin chunk
				m_vecPointer[ cell->chunk_index ] = new CConvertChunk( cell->chunk_index, cell->point_number, this );
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

			SPBPoint * point = m_cReader.GetPoint();
			CVector3D v( point->pos[0], point->pos[1], point->pos[2] );
			CConvertChunk * chunk = m_vecPointer[ Index( v ) ];
			chunk->PushPoint( point );

			IncReadNumber();

		}
	}

	return index;
}

//////////////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////////////

void CConvertGrid::InitCount()
{

	CParamManager * manager = CParamManager::GetParamManager();

	m_cReader.OpenFile( manager->m_pInputFile );
	m_cReader.RegisterGrid( this );
	m_cReader.ReadHeader();

	m_vecPointer.clear();
	m_vecPointer.resize( m_nSideNumber * m_nSideNumber, NULL );

	m_hashWriterInfo.clear();
	m_hashFinalizedPatch.clear();
}

void CConvertGrid::CountChunk( CConvertChunk * chunk )
{
	CParamManager * manager = CParamManager::GetParamManager();

	double min_z = MinGroundZ( chunk );

	for ( int i = 0; i < ( int )( chunk->m_vecPatchPointData.size() ); i++ ) {

		PatchPointData & point = chunk->m_vecPatchPointData[ i ];

		if ( CheckPoint( point ) ) {

			stdext::hash_map< PatchIndex, PatchWriterInfo >::iterator it = m_hashWriterInfo.find( point.patch.base );

			if ( it == m_hashWriterInfo.end() ) {
				
				PatchWriterInfo info;
				info.begin_cell = chunk->m_iIndex;
				info.final_cell = chunk->m_iIndex;
				info.ground_z = min_z;
				info.number = 1;

				m_hashWriterInfo.insert( std::pair< PatchIndex, PatchWriterInfo >( point.patch.base, info ) );

			} else {

				it->second.final_cell = chunk->m_iIndex;
				if ( it->second.ground_z > min_z )
					it->second.ground_z = min_z;
				it->second.number++;

			}

		}

	}
}

void CConvertGrid::FinCount()
{

	m_cReader.CloseFile();

	for ( stdext::hash_map< PatchIndex, PatchWriterInfo >::iterator it = m_hashWriterInfo.begin(); it != m_hashWriterInfo.end(); it++ ) {

		stdext::hash_map< int, std::vector< PatchIndex > >::iterator itt = m_hashFinalizedPatch.find( it->second.final_cell );

		if ( itt == m_hashFinalizedPatch.end() ) {
			std::vector< PatchIndex > temp;
			temp.push_back( it->first );
			m_hashFinalizedPatch.insert( std::pair< int, std::vector< PatchIndex > >( it->second.final_cell, temp ) );
		} else {
			itt->second.push_back( it->first );
		}

	}

	fprintf_s( stderr, "Total %d roof patch are going to be written.\n", ( int )m_hashWriterInfo.size() );

}

void CConvertGrid::InitWrite()
{

	CParamManager * manager = CParamManager::GetParamManager();

	m_cReader.OpenFile( manager->m_pInputFile );
	m_cReader.RegisterGrid( this );
	m_cReader.ReadHeader();

	m_vecPointer.clear();
	m_vecPointer.resize( m_nSideNumber * m_nSideNumber, NULL );

	m_hashWriter.clear();
}

void CConvertGrid::WriteChunk( CConvertChunk * chunk )
{

	CParamManager * manager = CParamManager::GetParamManager();

	for ( int i = 0; i < ( int )( chunk->m_vecPatchPointData.size() ); i++ ) {

		PatchPointData & point = chunk->m_vecPatchPointData[ i ];

		if ( CheckPoint( point ) ) {

			stdext::hash_map< PatchIndex, CBPWriter >::iterator it = m_hashWriter.find( point.patch.base );

			if ( it == m_hashWriter.end() ) {

				stdext::hash_map< PatchIndex, PatchWriterInfo >::iterator itinfo = m_hashWriterInfo.find( point.patch.base );
				if ( itinfo->second.ground_z == 1e300 ) {
					itinfo->second.ground_z = m_cBoundingBox.m_vMin[ 2 ];
				}

				char filename[ 1024 ];
				sprintf_s( filename, 1024, "%sPatch_%016I64x.bp", manager->m_pOutputDir, point.patch.base );

				CBPWriter writer;
				writer.OpenFile( filename );
				writer.WriteHeader( point.patch.base, itinfo->second.number, itinfo->second.ground_z, m_dbGridLength );
				writer.WritePoint( point.v.pVec, point.n.pVec, point.flatness, -1 );
				m_hashWriter.insert( std::pair< PatchIndex, CBPWriter >( point.patch.base, writer ) );

			} else {

				it->second.WritePoint( point.v.pVec, point.n.pVec, point.flatness, -1 );

			}
		}
	}

	stdext::hash_map< int, std::vector< PatchIndex > >::iterator itt = m_hashFinalizedPatch.find( chunk->m_iIndex );

	if ( itt != m_hashFinalizedPatch.end() ) {

		std::vector< PatchIndex > & temp = itt->second;

		for ( int k = 0; k < ( int )temp.size(); k++ ) {
			stdext::hash_map< PatchIndex, CBPWriter >::iterator it = m_hashWriter.find( temp[ k ] );
			it->second.CloseFile();
		}

	}
	
}

void CConvertGrid::FinWrite()
{

	m_cReader.CloseFile();

}

//////////////////////////////////////////////////////////////////////////
// auxiliary functions
//////////////////////////////////////////////////////////////////////////

bool CConvertGrid::CheckPoint( PatchPointData & point )
{

	CParamManager * manager = CParamManager::GetParamManager();

	if ( manager->m_bClip == false || 
		( ( point.v[0] >= manager->m_dbClip[0][0] )
		&& ( point.v[0] <= manager->m_dbClip[0][1] )
		&& ( point.v[1] >= manager->m_dbClip[1][0] )
		&& ( point.v[1] <= manager->m_dbClip[1][1] ) ) )
	{
		if ( point.type == PT_Building ) {
			return true;
		}
	}

	return false;

}

double CConvertGrid::MinGroundZ( CConvertChunk * chunk )
{
	double min_z = 1e300;

	for ( int i = 0; i < ( int )( chunk->m_vecPatchPointData.size() ); i++ ) {
		PatchPointData & point = chunk->m_vecPatchPointData[ i ];
		if ( point.type == PT_Ground && point.v[ 2 ] < min_z ) {
			min_z = point.v[ 2 ];
		}
	}

	return min_z;
}
