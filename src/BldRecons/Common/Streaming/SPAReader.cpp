#include "StdAfx.h"
#include "SPAReader.h"

#include <io.h>
#include <fcntl.h>

CSPAReader::CSPAReader(void)
{
	m_pFile = NULL;
	m_pGrid = NULL;
}

CSPAReader::~CSPAReader(void)
{
}

void CSPAReader::OpenFile( char filename[] )
{
	if ( _strcmpi( filename, "stdin" ) == 0 ) {
		_setmode( _fileno( stdout ), _O_BINARY );
		m_pFile = stdin;
	} else {
		fopen_s( &m_pFile, filename, "rb" );
	}
}

void CSPAReader::CloseFile()
{
	if ( m_pFile == stdin ) {
	} else {
		fclose( m_pFile );
	}
}

void CSPAReader::RegisterGrid( CStreamingGrid *grid )
{
	m_pGrid = grid;
}

void CSPAReader::ReadHeader()
{
	if ( m_pFile == NULL || m_pGrid == NULL )
		return;

	//struct SPAHeader {
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
	fread( &header, sizeof( SPAHeader ), 1, m_pFile );

	if ( strcmp( header.signature, signature_spa ) != 0 ) {
		fprintf_s( stderr, "SPA file signature unrecognized!\n" );
		return;
	}

	switch ( header.version ) {
	case 1:
	default:
		m_pGrid->m_nPointNumber = header.number;
		m_pGrid->m_cBoundingBox.m_vMin = CVector3D( header.pos_min[0], header.pos_min[1], header.pos_min[2] );
		m_pGrid->m_cBoundingBox.m_vMax = CVector3D( header.pos_max[0], header.pos_max[1], header.pos_max[2] );
		m_pGrid->m_nCellDepth = header.cell_depth;
		m_pGrid->m_dbGridLength = header.grid_length;
		m_pGrid->m_nUnitNumber[ 0 ] = header.unit_number[ 0 ];
		m_pGrid->m_nUnitNumber[ 1 ] = header.unit_number[ 1 ];
		m_pGrid->m_nSideNumber = ( 1 << m_pGrid->m_nCellDepth );
		m_pGrid->m_dbUnit[ 0 ] = m_pGrid->m_dbGridLength * m_pGrid->m_nUnitNumber[ 0 ];
		m_pGrid->m_dbUnit[ 1 ] = m_pGrid->m_dbGridLength * m_pGrid->m_nUnitNumber[ 1 ];
		break;
	}

	InitBuffer();
}

void CSPAReader::InitBuffer()
{
	m_nElementsInBuffer = 31;
	m_pElementInBuffer = NULL;
}

bool CSPAReader::ReadNextElement()
{
	if ( m_nElementsInBuffer == 31 ) {
		m_nElementsInBuffer = 0;
		m_pElementInBuffer = m_cBuffer.buffer;
		fread( &m_cBuffer, sizeof( ElementBufferA ), 1, m_pFile );
	} else {
		m_nElementsInBuffer++;
		m_pElementInBuffer += BLOCK_SIZE_A;
	}
	return (bool)( ( m_cBuffer.descriptor >> m_nElementsInBuffer ) & 0x1 );
}

SPAPoint * CSPAReader::GetPoint()
{
	return ( SPAPoint * )m_pElementInBuffer;
}

SPACell * CSPAReader::GetCell()
{
	return ( SPACell * )m_pElementInBuffer;
}
