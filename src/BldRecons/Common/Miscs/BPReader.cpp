#include "StdAfx.h"
#include "BPReader.h"

#include <io.h>
#include <fcntl.h>

CBPReader::CBPReader(void)
{
	m_pFile = NULL;
}

CBPReader::~CBPReader(void)
{
}

void CBPReader::OpenFile( char filename[] )
{
	fopen_s( &m_pFile, filename, "rb" );
}

void CBPReader::CloseFile()
{
	fclose( m_pFile );
}

BPHeader CBPReader::ReadHeader()
{
	//struct BPHeader {
	//	char signature[16];
	//	int version;
	//	int number;
	//	__int64 patch;
	//	double grid_length;
	//};

	BPHeader header;
	fread( &header, sizeof( BPHeader ), 1, m_pFile );

	if ( strcmp( header.signature, signature_bp ) != 0 ) {
		fprintf_s( stderr, "BP file signature unrecognized!\n" );
	}

	return header;
}

BPPoint CBPReader::ReadPoint()
{
	BPPoint point;
	fread( &point, sizeof( BPPoint ), 1, m_pFile );

	return point;
}
