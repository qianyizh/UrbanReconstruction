#include "StdAfx.h"
#include "BPOWriter.h"

#include <io.h>
#include <fcntl.h>

CBPOWriter::CBPOWriter(void)
{
	m_pFile = NULL;
}

CBPOWriter::~CBPOWriter(void)
{
}

void CBPOWriter::OpenFile( char filename[] )
{
	fopen_s( &m_pFile, filename, "wb" );
}

void CBPOWriter::CloseFile()
{
	fclose( m_pFile );
}

void CBPOWriter::WriteHeader( PatchIndex patch, int number, double ground_z, double grid_length )
{
	if ( m_pFile == NULL )
		return;

	//struct BPOHeader {
	//	char signature[16];
	//	int version;
	//	int element_size;
	//	__int64 patch;
	//	double grid_length;
	//};

	BPOHeader header;

	memset( header.signature, 0, 16 );
	memcpy_s( header.signature, 16, signature_bpo, strlen( signature_bpo ) );

	header.version = 1;

	header.number = number;

	header.patch = patch;

	header.ground_z = ground_z;

	header.grid_length = grid_length;

	fwrite( &header, sizeof( BPOHeader ), 1, m_pFile );
}

void CBPOWriter::WriteOutline( COutline & outline )
{
	outline.Save( m_pFile );
}
