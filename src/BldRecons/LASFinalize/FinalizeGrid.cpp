#include "stdafx.h"
#include "FinalizeGrid.h"
#include "ParamManager.h"

#include "liblas\lasfile.hpp"
using namespace liblas;

CFinalizeGrid::CFinalizeGrid(void)
{
}

CFinalizeGrid::~CFinalizeGrid(void)
{
}

void CFinalizeGrid::ComputeBoundingBox()
{
	
	fprintf_s( stderr, "==================== Pass 1, compute bounding box ====================\n" );

	CParamManager * manager = CParamManager::GetParamManager();
	m_nCellDepth = manager->m_nCellDepth;
	m_dbGridLength = manager->m_dbGridLength;

	m_cBoundingBox.Reset();

	for ( int i = 0; i < ( int )manager->m_vecInputFiles.size(); i++ ) {

		fprintf_s( stderr, "Reading header of %s ... ", manager->m_vecInputFiles[ i ].name );
		LASFile file( std::string( manager->m_vecInputFiles[ i ].name ) );

		const LASHeader & header = file.GetHeader();
		double dbTemp[3];
		dbTemp[0] = header.GetMinX() * manager->m_dbScale;
		dbTemp[1] = header.GetMinY() * manager->m_dbScale;
		dbTemp[2] = header.GetMinZ() * manager->m_dbScale;
		m_cBoundingBox.Push( dbTemp );
		dbTemp[0] = header.GetMaxX() * manager->m_dbScale;
		dbTemp[1] = header.GetMaxY() * manager->m_dbScale;
		dbTemp[2] = header.GetMaxZ() * manager->m_dbScale;
		m_cBoundingBox.Push( dbTemp );

		fprintf_s( stderr, "succeed!\n" );

	}

	m_cBoundingBox.PrintInfo();
	ComputeGridLength();

	fprintf_s( stderr, "\n" );

}

void CFinalizeGrid::ComputeGridLength()
{
	m_nSideNumber = 1;
	m_nSideNumber <<= m_nCellDepth;
	
	// unit_length = unit_number * grid_length
	// unit_length > box_length / side_number
	double x = m_cBoundingBox.GetLength( 0 ) / ( double )m_nSideNumber;
	m_nUnitNumber[ 0 ] = __max( ( int )( x / m_dbGridLength ) + 1, 2 );
	m_dbUnit[ 0 ] = m_dbGridLength * m_nUnitNumber[ 0 ];
	double y = m_cBoundingBox.GetLength( 1 ) / ( double )m_nSideNumber;
	m_nUnitNumber[ 1 ] = __max( ( int )( y / m_dbGridLength ) + 1, 2 );
	m_dbUnit[ 1 ] = m_dbGridLength * m_nUnitNumber[ 1 ];

	fprintf_s( stderr, "Grid resolution %dx%d:\n\tlength_x = %.2f x %d = %.2f\n\tlength_y = %.2f x %d = %.2f\n",
		m_nSideNumber, m_nSideNumber, m_dbGridLength, m_nUnitNumber[ 0 ], m_dbUnit[ 0 ], m_dbGridLength, m_nUnitNumber[ 1 ], m_dbUnit[ 1 ] );
}

void CFinalizeGrid::Stream_ComputeGridIndex()
{

	fprintf_s( stderr, "==================== Pass 2, count index for each chunk ====================\n" );

	CParamManager * manager = CParamManager::GetParamManager();
	m_vecGridIndex.clear();
	m_vecGridIndex.resize( m_nSideNumber * m_nSideNumber, -1 );
	m_vecGridNumber.clear();
	m_vecGridNumber.resize( m_nSideNumber * m_nSideNumber, 0 );
	m_nPointNumber = 0;

	for ( int i = 0; i < ( int )manager->m_vecInputFiles.size(); i++ ) {

		fprintf_s( stderr, "Reading %s ...   0%%", manager->m_vecInputFiles[ i ].name );

		LASFile file( std::string( manager->m_vecInputFiles[ i ].name ) );

		const LASHeader & header = file.GetHeader();
		LASReader & reader = file.GetReader();
		double total = ( double )header.GetPointRecordsCount();
		int prog = 0;
		int num = 0;

		while ( reader.ReadNextPoint() ) {
			const LASPoint & point = reader.GetPoint();
			if ( CheckPoint( point ) ) {
				int idx = Index( CVector3D( point.GetX() * manager->m_dbScale, point.GetY() * manager->m_dbScale, point.GetZ() * manager->m_dbScale ) );
				m_vecGridIndex[ idx ] = m_nPointNumber;
				m_vecGridNumber[ idx ]++;
				m_nPointNumber++;
			}

			// output
			num++;
			int temp_prog = ( int )( num * 100.0 / total );
			if ( temp_prog > prog ) {
				prog = temp_prog;
				fprintf_s( stderr, "\b\b\b\b%3d%%", prog );
			}
		}

		fprintf_s( stderr, " ... done.\n" );
	}

	fprintf_s( stderr, "\n" );

}

void CFinalizeGrid::Stream_WriteGrid()
{

	fprintf_s( stderr, "==================== Pass 3, re-order and output ====================\n" );

	CParamManager * manager = CParamManager::GetParamManager();
	std::vector< ChunkData * > vecPointer;
	vecPointer.clear();
	vecPointer.resize( m_nSideNumber * m_nSideNumber, NULL );
	std::vector< int > vecGridNumber;
	vecGridNumber.clear();
	vecGridNumber.resize( m_nSideNumber * m_nSideNumber, 0 );
	int nPointNumber = 0;

	InitWrite();

	for ( int i = 0; i < m_nSideNumber * m_nSideNumber; i++ ) {
		if ( m_vecGridNumber[ i ] == 0 ) {
			WriteChunk( NULL, i, 0 );
		}
	}

	for ( int i = 0; i < ( int )manager->m_vecInputFiles.size(); i++ ) {

		fprintf_s( stderr, "Reading %s ...   0%%", manager->m_vecInputFiles[ i ].name );

		LASFile file( std::string( manager->m_vecInputFiles[ i ].name ) );

		const LASHeader & header = file.GetHeader();
		LASReader & reader = file.GetReader();
		double total = ( double )header.GetPointRecordsCount();
		int prog = 0;
		int num = 0;

		while ( reader.ReadNextPoint() ) {

			const LASPoint & point = reader.GetPoint();

			if ( CheckPoint( point ) ) {
				CVector3D v( point.GetX() * manager->m_dbScale, point.GetY() * manager->m_dbScale, point.GetZ() * manager->m_dbScale );
				int idx = Index( v );
				if ( vecPointer[ idx ] == NULL ) {
					// first time touch this cell
					vecPointer[ idx ] = new ChunkData();
					( * vecPointer[ idx ] ).resize( m_vecGridNumber[ idx ] );
				}
				( * vecPointer[ idx ] )[ vecGridNumber[ idx ] ] = v;

				if ( m_vecGridIndex[ idx ] == nPointNumber ) {
					// finalize this chunk
					WriteChunk( vecPointer[ idx ], idx, m_vecGridNumber[ idx ] );
					delete vecPointer[ idx ];
					vecPointer[ idx ] = NULL;
				}

				vecGridNumber[ idx ]++;
				nPointNumber++;
			}

			// output
			num++;
			int temp_prog = ( int )( num * 100.0 / total );
			if ( temp_prog > prog ) {
				prog = temp_prog;
				fprintf_s( stderr, "\b\b\b\b%3d%%", prog );
			}

		}

		fprintf_s( stderr, " ... done.\n" );
	}

	FinWrite();

	fprintf_s( stderr, "Totally %d points have been written.\n", nPointNumber );
	fprintf_s( stderr, "\n" );

}

void CFinalizeGrid::InitWrite()
{
	CParamManager * manager = CParamManager::GetParamManager();

	switch ( manager->m_cOutputFormat ) {
	case CParamManager::FOF_SPA:
		m_cAWriter.OpenFile( manager->m_pOutputFile );
		m_cAWriter.RegisterGrid( this );
		m_cAWriter.WriteHeader();
		break;
	case CParamManager::FOF_SPB:
		m_cBWriter.OpenFile( manager->m_pOutputFile );
		m_cBWriter.RegisterGrid( this );
		m_cBWriter.WriteHeader();
		break;
	case CParamManager::FOF_SP:
	default:
		m_cWriter.OpenFile( manager->m_pOutputFile );
		m_cWriter.RegisterGrid( this );
		m_cWriter.WriteHeader();
		break;
	}
}

void CFinalizeGrid::FinWrite()
{
	CParamManager * manager = CParamManager::GetParamManager();

	switch ( manager->m_cOutputFormat ) {
	case CParamManager::FOF_SPA:
		m_cAWriter.WriteEOF();
		m_cAWriter.CloseFile();
		break;
	case CParamManager::FOF_SPB:
		m_cBWriter.WriteEOF();
		m_cBWriter.CloseFile();
		break;
	case CParamManager::FOF_SP:
	default:
		m_cWriter.WriteEOF();
		m_cWriter.CloseFile();
		break;
	}
}

void CFinalizeGrid::WriteChunk( ChunkData * data, int index, int number )
{
	CParamManager * manager = CParamManager::GetParamManager();

	switch ( manager->m_cOutputFormat ) {
	case CParamManager::FOF_SPA:
		m_cAWriter.WriteBeginCell( index, number );
		if ( data != NULL ) {
			for ( int i = 0; i < ( int )( (*data).size() ); i++ ) {
				m_cAWriter.WritePoint( (*data)[i] );
			}
		}
		m_cAWriter.WriteFinalizeCell( index, number );
		break;
	case CParamManager::FOF_SPB:
		m_cBWriter.WriteBeginCell( index, number );
		if ( data != NULL ) {
			for ( int i = 0; i < ( int )( (*data).size() ); i++ ) {
				m_cBWriter.WritePoint( (*data)[i] );
			}
		}
		m_cBWriter.WriteFinalizeCell( index, number );
		break;
	case CParamManager::FOF_SP:
	default:
		m_cWriter.WriteBeginCell( index, number );
		if ( data != NULL ) {
			for ( int i = 0; i < ( int )( (*data).size() ); i++ ) {
				m_cWriter.WritePoint( (*data)[i] );
			}
		}
		m_cWriter.WriteFinalizeCell( index, number );
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// auxiliary functions
//////////////////////////////////////////////////////////////////////////

bool CFinalizeGrid::CheckPoint( const LASPoint & point )
{
	return true;
}
