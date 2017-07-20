#include "StdAfx.h"
#include "SPReader.h"

#include <io.h>
#include <fcntl.h>

CSPReader::CSPReader(void)
{
	m_pFile = NULL;
	m_pGrid = NULL;
}

CSPReader::~CSPReader(void)
{
}

void CSPReader::OpenFile( char filename[] )
{
	if ( _strcmpi( filename, "stdin" ) == 0 ) {
		_setmode( _fileno( stdout ), _O_BINARY );
		m_pFile = stdin;
	} else {
		fopen_s( &m_pFile, filename, "rb" );
	}
}

void CSPReader::CloseFile()
{
	if ( m_pFile == stdin ) {
	} else {
		fclose( m_pFile );
	}
}

void CSPReader::RegisterGrid( CStreamingGrid *grid )
{
	m_pGrid = grid;
}

void CSPReader::ReadHeader()
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

	SPHeader header;
	fread( &header, sizeof( SPHeader ), 1, m_pFile );

	if ( strcmp( header.signature, signature_sp ) != 0 ) {
		fprintf_s( stderr, "SP file signature unrecognized!\n" );
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

void CSPReader::InitBuffer()
{
	m_nElementsInBuffer = 31;
	m_pElementInBuffer = NULL;
}

bool CSPReader::ReadNextElement()
{
	if ( m_nElementsInBuffer == 31 ) {
		m_nElementsInBuffer = 0;
		m_pElementInBuffer = m_cBuffer.buffer;
		fread( &m_cBuffer, sizeof( ElementBuffer ), 1, m_pFile );
	} else {
		m_nElementsInBuffer++;
		m_pElementInBuffer += BLOCK_SIZE;
	}
	return (bool)( ( m_cBuffer.descriptor >> m_nElementsInBuffer ) & 0x1 );
}

SPPoint * CSPReader::GetPoint()
{
	return ( SPPoint * )m_pElementInBuffer;
}

SPCell * CSPReader::GetCell()
{
	return ( SPCell * )m_pElementInBuffer;
}
