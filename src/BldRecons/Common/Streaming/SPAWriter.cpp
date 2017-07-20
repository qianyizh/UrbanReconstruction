#include "StdAfx.h"
#include "SPAWriter.h"

#include <io.h>
#include <fcntl.h>

CSPAWriter::CSPAWriter(void)
{
	m_pFile = NULL;
	m_pGrid = NULL;
}

CSPAWriter::~CSPAWriter(void)
{
}

void CSPAWriter::OpenFile( char filename[] )
{
	if ( _strcmpi( filename, "stdout" ) == 0 ) {
		_setmode( _fileno( stdout ), _O_BINARY );
		m_pFile = stdout;
	} else {
		fopen_s( &m_pFile, filename, "wb" );
	}
}

void CSPAWriter::CloseFile()
{
	fflush( m_pFile );
	if ( m_pFile == stdout ) {

	} else {
		fclose( m_pFile );
	}
}

void CSPAWriter::RegisterGrid( CStreamingGrid *grid )
{
	m_pGrid = grid;
}

void CSPAWriter::WriteHeader()
{
	if ( m_pFile == NULL || m_pGrid == NULL )
		return;

	//struct SPHeader {
	//	char signature[16];
	//	int version;
	//	int number;
	//	double pos_min[3];
	//	double pos_max[3];
	//	int cell_depth;
	//	double grid_length;
	//	int unit_number[2];
	//};

	SPAHeader header;

	memset( header.signature, 0, 16 );
	memcpy_s( header.signature, 16, signature_spa, strlen( signature_spa ) );

	header.version = 1;

	header.number = m_pGrid->m_nPointNumber;

	header.pos_min[0] = m_pGrid->m_cBoundingBox.m_vMin[0];
	header.pos_min[1] = m_pGrid->m_cBoundingBox.m_vMin[1];
	header.pos_min[2] = m_pGrid->m_cBoundingBox.m_vMin[2];

	header.pos_max[0] = m_pGrid->m_cBoundingBox.m_vMax[0];
	header.pos_max[1] = m_pGrid->m_cBoundingBox.m_vMax[1];
	header.pos_max[2] = m_pGrid->m_cBoundingBox.m_vMax[2];

	header.element_size = BLOCK_SIZE_A;
	header.cell_depth = m_pGrid->m_nCellDepth;

	header.grid_length = m_pGrid->m_dbGridLength;

	header.unit_number[0] = m_pGrid->m_nUnitNumber[0];
	header.unit_number[1] = m_pGrid->m_nUnitNumber[1];

	fwrite( &header, sizeof( SPAHeader ), 1, m_pFile );

	InitBuffer();
}

void CSPAWriter::InitBuffer()
{
	m_nElementsInBuffer = 0;
	m_cBuffer.descriptor = 0;
	memset( m_cBuffer.buffer, 0, BLOCK_SIZE_A * 32 );
}

void CSPAWriter::WritePoint( const CVector3D & v )
{
	// descriptor
	// no need to change

	// buffer
	SPAPoint * pPoint = ( SPAPoint * )( m_cBuffer.buffer + BLOCK_SIZE_A * m_nElementsInBuffer );
	pPoint->type = ( __int64 )PT_Building;
	pPoint->flatness = 0.0;
	pPoint->pos[ 0 ] = v.pVec[ 0 ];
	pPoint->pos[ 1 ] = v.pVec[ 1 ];
	pPoint->pos[ 2 ] = v.pVec[ 2 ];
	pPoint->n[ 0 ] = 0.0;
	pPoint->n[ 1 ] = 0.0;
	pPoint->n[ 2 ] = 1.0;

	m_nElementsInBuffer++;

	// write
	if ( m_nElementsInBuffer == 32 ) {
		fwrite( &m_cBuffer, sizeof( ElementBufferA ), 1, m_pFile );
		InitBuffer();
	}
}

void CSPAWriter::WritePoint( const PointData & point )
{
	// descriptor
	// no need to change

	// buffer
	SPAPoint * pPoint = ( SPAPoint * )( m_cBuffer.buffer + BLOCK_SIZE_A * m_nElementsInBuffer );
	pPoint->type = ( __int64 )point.type;
	pPoint->flatness = point.feature[ 2 ] / 3.0;
	pPoint->pos[ 0 ] = point.v.pVec[ 0 ];
	pPoint->pos[ 1 ] = point.v.pVec[ 1 ];
	pPoint->pos[ 2 ] = point.v.pVec[ 2 ];
	pPoint->n[ 0 ] = point.n.pVec[ 0 ];
	pPoint->n[ 1 ] = point.n.pVec[ 1 ];
	pPoint->n[ 2 ] = point.n.pVec[ 2 ];
	
	m_nElementsInBuffer++;

	// write
	if ( m_nElementsInBuffer == 32 ) {
		fwrite( &m_cBuffer, sizeof( ElementBufferA ), 1, m_pFile );
		InitBuffer();
	}
}

void CSPAWriter::WriteBeginCell( int index, int number )
{
	// descriptor
	m_cBuffer.descriptor |= ( 1 << m_nElementsInBuffer );

	// buffer
	SPACell * pCell = ( SPACell * )( m_cBuffer.buffer + BLOCK_SIZE_A * m_nElementsInBuffer );
	pCell->type = 0;
	pCell->chunk_index = index;
	pCell->point_number = number;
	m_nElementsInBuffer++;

	// write
	if ( m_nElementsInBuffer == 32 ) {
		fwrite( &m_cBuffer, sizeof( ElementBufferA ), 1, m_pFile );
		InitBuffer();
	}
}

void CSPAWriter::WriteFinalizeCell( int index, int number )
{
	// descriptor
	m_cBuffer.descriptor |= ( 1 << m_nElementsInBuffer );

	// buffer
	SPACell * pCell = ( SPACell * )( m_cBuffer.buffer + BLOCK_SIZE_A * m_nElementsInBuffer );
	pCell->type = 1;
	pCell->chunk_index = index;
	pCell->point_number = number;
	m_nElementsInBuffer++;

	// write
	if ( m_nElementsInBuffer == 32 ) {
		fwrite( &m_cBuffer, sizeof( ElementBufferA ), 1, m_pFile );
		InitBuffer();
	}
}

void CSPAWriter::WriteEOF()
{
	// descriptor
	m_cBuffer.descriptor |= ( 1 << m_nElementsInBuffer );

	// buffer
	SPACell * pCell = ( SPACell * )( m_cBuffer.buffer + BLOCK_SIZE_A * m_nElementsInBuffer );
	pCell->type = -1;
	pCell->chunk_index = -1;
	pCell->point_number = -1;
	m_nElementsInBuffer++;

	// write
	fwrite( &m_cBuffer, sizeof( ElementBufferA ), 1, m_pFile );
	InitBuffer();
}
