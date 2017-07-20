#include "StdAfx.h"
#include "BPOReader.h"

#include <io.h>
#include <fcntl.h>

CBPOReader::CBPOReader(void)
{
	m_pFile = NULL;
}

CBPOReader::~CBPOReader(void)
{
}

void CBPOReader::OpenFile(const char filename[])
{
	fopen_s( &m_pFile, filename, "rb" );
}

void CBPOReader::CloseFile()
{
	fclose( m_pFile );
}

BPOHeader CBPOReader::ReadHeader()
{
	//struct BPOHeader {
	//	char signature[16];
	//	int version;
	//	int number;
	//	__int64 patch;
	//	double ground_z;
	//	double grid_length;
	//};

	BPOHeader header;
	fread( &header, sizeof( BPOHeader ), 1, m_pFile );

	if ( strcmp( header.signature, signature_bpo ) != 0 ) {
		fprintf_s( stderr, "BPO file signature unrecognized!\n" );
	}

	return header;
}

void CBPOReader::ReadOutline( COutline & outline )
{
	outline.Load( m_pFile );
}
