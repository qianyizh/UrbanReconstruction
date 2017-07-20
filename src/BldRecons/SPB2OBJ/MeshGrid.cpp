#include "StdAfx.h"
#include "MeshGrid.h"
#include "ParamManager.h"
#include "Miscs\TimeMeter.h"

CMeshGrid::CMeshGrid(void)
{
}

CMeshGrid::~CMeshGrid(void)
{
}

void CMeshGrid::Mesh()
{

	CParamManager * manager = CParamManager::GetParamManager();
	CTimeMeter timer;
	timer.Start();
	fprintf_s( stderr, "==================== Pass 1, meshing ====================\n" );

	Init();

	fprintf_s( stderr, "Processing progress ... " );
	InitPrintProgress();

	int index;

	while ( ( index = ReadNextChunk() ) != -1 ) {

		PrintProgress(  );

		CMeshChunk * chunk = m_vecPointer[ index ];
		chunk->BuildGridIndex();

		ComputeGroundHeight( chunk );

		delete m_vecPointer[ index ];
		m_vecPointer[ index ] = NULL;

	}

	fprintf_s( stderr, " ... done!\n" );

	Fin();

	fprintf_s( stderr, "Total processing time is " );
	timer.End();
	timer.Print();
	fprintf_s( stderr, ".\n\n" );

	timer.Start();
	fprintf_s( stderr, "==================== Pass 2, interpolate holes ====================\n" );
	fprintf_s( stderr, "Processing ... " );

	Interpolate();

	fprintf_s( stderr, "done!\n" );
	fprintf_s( stderr, "Total processing time is " );
	timer.End();
	timer.Print();
	fprintf_s( stderr, ".\n\n" );

	timer.Start();
	fprintf_s( stderr, "==================== Pass 3, output ====================\n" );
	fprintf_s( stderr, "Writing %s ...", manager->m_pOutputFile );

	WriteMesh( manager->m_nSampleGrid );

	fprintf_s( stderr, "succeed!\n" );
	fprintf_s( stderr, "Total %d vertices and %d faces written in ", m_nVertexNumber, m_nFaceNumber );
	timer.End();
	timer.Print();
	fprintf_s( stderr, ".\n" );

}

int CMeshGrid::ReadNextChunk()
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
				m_vecPointer[ cell->chunk_index ] = new CMeshChunk( cell->chunk_index, cell->point_number, this );
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
			CMeshChunk * chunk = m_vecPointer[ Index( v ) ];
			chunk->PushPoint( point );

			IncReadNumber();

		}
	}

	return index;
}

//////////////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////////////

void CMeshGrid::Init()
{

	CParamManager * manager = CParamManager::GetParamManager();

	m_cReader.OpenFile( manager->m_pInputFile );
	m_cReader.RegisterGrid( this );
	m_cReader.ReadHeader();

	m_vecPointer.clear();
	m_vecPointer.resize( m_nSideNumber * m_nSideNumber, NULL );

	InitClip();

}

void CMeshGrid::ComputeGroundHeight( CMeshChunk * chunk )
{
	for ( int x = 0; x < m_nUnitNumber[ 0 ]; x++ )
		for ( int y = 0; y < m_nUnitNumber[ 1 ]; y++ ) {

			int index = ClipIndex( x + chunk->m_iX * m_nUnitNumber[ 0 ], y + chunk->m_iY * m_nUnitNumber[ 1 ] );

			if ( index != -1 ) {

				PatchPointDataVector & cell_vector = chunk->m_vecGridIndex[ chunk->Index( x, y ) ];
				m_vecHeight[ index ] = 1e300;

				for ( int i = 0; i < ( int )cell_vector.size(); i++ ) {

					PatchPointData & point = *( cell_vector[ i ] );

					if ( point.type == PT_Ground && point.v[ 2 ] < m_vecHeight[ index ] ) {

						m_vecHeight[ index ] = point.v[ 2 ];

					}

				}
				
			}

		}
}

void CMeshGrid::Fin()
{

	m_cReader.CloseFile();
	
}

void CMeshGrid::Interpolate()
{
	int last_solid;
	double last_height, height;

	for ( int y = m_nClip[ 1 ][ 0 ]; y <= m_nClip[ 1 ][ 1 ]; y++ ) {
		last_solid = m_nClip[ 0 ][ 0 ] - 1;

		for ( int x = m_nClip[ 0 ][ 0 ]; x <= m_nClip[ 0 ][ 1 ]; x++ ) {
			int index = ClipIndex( x, y );
			if ( m_vecHeight[ index ] != 1e300 ) {
				// has data, solid!
				height = m_vecHeight[ index ];
				if ( last_solid == m_nClip[ 0 ][ 0 ] - 1 ) {
					last_height = height;
				} else {
					last_height = m_vecHeight[ ClipIndex( last_solid, y ) ];
				}

				for ( int i = last_solid + 1; i < x; i++ ) {
					m_vecHeight[ ClipIndex( i, y ) ] = height * ( double )( i - last_solid ) / ( double )( x - last_solid )
						+ last_height * ( double )( x - i ) / ( double )( x - last_solid );
				}

				last_solid = x;
			}
		}

		if ( last_solid != m_nClip[ 0 ][ 1 ] ) {
			int x = m_nClip[ 0 ][ 1 ] + 1;
			if ( last_solid == m_nClip[ 0 ][ 0 ] - 1 ) {
				last_height = height = 0.0;					// default height
			} else {
				last_height = height = m_vecHeight[ ClipIndex( last_solid, y ) ];
			}
			for ( int i = last_solid + 1; i < x; i++ ) {
				m_vecHeight[ ClipIndex( i, y ) ] = height * ( double )( i - last_solid ) / ( double )( x - last_solid )
					+ last_height * ( double )( x - i ) / ( double )( x - last_solid );
			}
		}
	}
}

void CMeshGrid::WriteMesh( int sample_grid )
{
	CParamManager * manager = CParamManager::GetParamManager();

	m_cWriter.OpenFile( manager->m_pOutputFile );
	m_cWriter.WriteHeader();
	
	m_nVertexNumber = m_nFaceNumber = 0;

	double data[3];
	int idata[3];

	for ( int x = m_nClip[ 0 ][ 0 ]; x <= m_nClip[ 0 ][ 1 ]; x += sample_grid )
		for ( int y = m_nClip[ 1 ][ 0 ]; y <= m_nClip[ 1 ][ 1 ]; y += sample_grid ) {
			data[0] = m_cBoundingBox.m_vMin[0] + m_dbGridLength * ( x + 0.5 );
			data[1] = m_cBoundingBox.m_vMin[1] + m_dbGridLength * ( y + 0.5 );
			data[2] = m_vecHeight[ ClipIndex( x, y ) ];
			m_cWriter.WriteVertex( data );
			m_nVertexNumber++;
		}

	int y_range = ( m_nClip[ 1 ][ 1 ] - m_nClip[ 1 ][ 0 ] ) / sample_grid + 1;

	for ( int x = m_nClip[ 0 ][ 0 ]; x <= m_nClip[ 0 ][ 1 ] - sample_grid; x += sample_grid )
		for ( int y = m_nClip[ 1 ][ 0 ]; y <= m_nClip[ 1 ][ 1 ] - sample_grid; y += sample_grid ) {
			int truex = ( x - m_nClip[ 0 ][ 0 ] ) / sample_grid;
			int truey = ( y - m_nClip[ 1 ][ 0 ] ) / sample_grid;

			idata[0] = ( truex ) * y_range + ( truey );
			idata[1] = ( truex + 1 ) * y_range + ( truey );
			idata[2] = ( truex + 1 ) * y_range + ( truey + 1 );
			m_cWriter.WriteFace( idata );
			m_nFaceNumber++;

			idata[0] = ( truex ) * y_range + ( truey );
			idata[1] = ( truex + 1 ) * y_range + ( truey + 1 );
			idata[2] = ( truex ) * y_range + ( truey + 1 );
			m_cWriter.WriteFace( idata );
			m_nFaceNumber++;
		}

	m_cWriter.CloseFile();

}

//////////////////////////////////////////////////////////////////////////
// auxiliary functions
//////////////////////////////////////////////////////////////////////////

void CMeshGrid::InitClip()
{
	CParamManager * manager = CParamManager::GetParamManager();

	if ( manager->m_bClip == false ) {
		manager->m_dbClip[ 0 ][ 0 ] = m_cBoundingBox.m_vMin[ 0 ];
		manager->m_dbClip[ 0 ][ 1 ] = m_cBoundingBox.m_vMax[ 0 ];
		manager->m_dbClip[ 1 ][ 0 ] = m_cBoundingBox.m_vMin[ 1 ];
		manager->m_dbClip[ 1 ][ 1 ] = m_cBoundingBox.m_vMax[ 1 ];
	}

	m_nClip[ 0 ][ 0 ] = ( int )( ( manager->m_dbClip[ 0 ][ 0 ] - m_cBoundingBox.m_vMin[ 0 ] ) / m_dbGridLength );
	m_nClip[ 0 ][ 1 ] = ( int )( ( manager->m_dbClip[ 0 ][ 1 ] - m_cBoundingBox.m_vMin[ 0 ] ) / m_dbGridLength );
	m_nClip[ 1 ][ 0 ] = ( int )( ( manager->m_dbClip[ 1 ][ 0 ] - m_cBoundingBox.m_vMin[ 1 ] ) / m_dbGridLength );
	m_nClip[ 1 ][ 1 ] = ( int )( ( manager->m_dbClip[ 1 ][ 1 ] - m_cBoundingBox.m_vMin[ 1 ] ) / m_dbGridLength );

	m_vecHeight.clear();
	m_vecHeight.resize( ( m_nClip[ 0 ][ 1 ] - m_nClip[ 0 ][ 0 ] + 1 ) * ( m_nClip[ 1 ][ 1 ] - m_nClip[ 1 ][ 0 ] + 1 ), 1e300 );
}

int CMeshGrid::ClipIndex( int x, int y )
{
	if ( x < m_nClip[ 0 ][ 0 ] || x > m_nClip[ 0 ][ 1 ] || y < m_nClip[ 1 ][ 0 ] || y > m_nClip[ 1 ][ 1 ] )
		return -1;
	else
		return ( ( x - m_nClip[ 0 ][ 0 ] ) * ( m_nClip[ 1 ][ 1 ] - m_nClip[ 1 ][ 0 ] + 1 ) + ( y - m_nClip[ 1 ][ 0 ] ) );
}