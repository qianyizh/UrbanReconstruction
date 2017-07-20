#include "StdAfx.h"
#include "SPBReader.h"

#include <io.h>
#include <fcntl.h>

CSPBReader::CSPBReader(void)
{
	m_pFile = NULL;
	m_pGrid = NULL;
}

CSPBReader::~CSPBReader(void)
{
}

void CSPBReader::OpenFile( char filename[] )
{
	if ( _strcmpi( filename, "stdin" ) == 0 ) {
		_setmode( _fileno( stdout ), _O_BINARY );
		m_pFile = stdin;
	} else {
		fopen_s( &m_pFile, filename, "rb" );
	}
}

void CSPBReader::CloseFile()
{
	if ( m_pFile == stdin ) {
	} else {
		fclose( m_pFile );
	}
}

void CSPBReader::RegisterGrid( CStreamingGrid *grid )
{
	m_pGrid = grid;
}

void CSPBReader::ReadHeader()
{
	if ( m_pFile == NULL || m_pGrid == NULL )
		return;

	//struct SPBHeader {
	//	char signature[16];
	//	int version;
	//	int number;
	//	double pos_min[3];
	//	double pos_max[3];
	//	int cell_depth;
	//	double grid_length;
	//	int unit_number[2];
	//};

	SPBHeader header;
	fread( &header, sizeof( SPBHeader ), 1, m_pFile );

	if ( strcmp( header.signature, signature_spb ) != 0 ) {
		fprintf_s( stderr, "SPB file signature unrecognized!\n" );
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

void CSPBReader::InitBuffer()
{
	m_nElementsInBuffer = 31;
	m_pElementInBuffer = NULL;
}

bool CSPBReader::ReadNextElement()
{
	if ( m_nElementsInBuffer == 31 ) {
		m_nElementsInBuffer = 0;
		m_pElementInBuffer = m_cBuffer.buffer;
		fread( &m_cBuffer, sizeof( ElementBufferB ), 1, m_pFile );
	} else {
		m_nElementsInBuffer++;
		m_pElementInBuffer += BLOCK_SIZE_B;
	}
	return (bool)( ( m_cBuffer.descriptor >> m_nElementsInBuffer ) & 0x1 );
}

SPBPoint * CSPBReader::GetPoint()
{
	return ( SPBPoint * )m_pElementInBuffer;
}

SPBCell * CSPBReader::GetCell()
{
	return ( SPBCell * )m_pElementInBuffer;
}
