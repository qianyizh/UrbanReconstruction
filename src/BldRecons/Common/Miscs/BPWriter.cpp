#include "StdAfx.h"
#include "BPWriter.h"

#include <io.h>
#include <fcntl.h>

CBPWriter::CBPWriter(void)
{
	m_pFile = NULL;
}

CBPWriter::~CBPWriter(void)
{
}

void CBPWriter::OpenFile( char filename[] )
{
	fopen_s( &m_pFile, filename, "wb" );
}

void CBPWriter::CloseFile()
{
	fclose( m_pFile );
}

void CBPWriter::WriteHeader( PatchIndex patch, int number, double ground_z, double grid_length )
{
	if ( m_pFile == NULL )
		return;

	//struct BPHeader {
	//	char signature[16];
	//	int version;
	//	int element_size;
	//	__int64 patch;
	//	double grid_length;
	//};

	BPHeader header;

	memset( header.signature, 0, 16 );
	memcpy_s( header.signature, 16, signature_bp, strlen( signature_bp ) );

	header.version = 1;

	header.patch = patch;

	header.number = number;

	header.ground_z = ground_z;

	header.grid_length = grid_length;

	fwrite( &header, sizeof( BPHeader ), 1, m_pFile );
}

void CBPWriter::WritePoint( const double pos[3], const double n[3], const double flatness, const __int64 plane )
{
	BPPoint point;
	point.plane = plane;
	point.flatness = flatness;
	point.pos[0] = pos[0];
	point.pos[1] = pos[1];
	point.pos[2] = pos[2];
	point.n[0] = n[0];
	point.n[1] = n[1];
	point.n[2] = n[2];
	fwrite( &point, sizeof( BPPoint ), 1, m_pFile );
}

void CBPWriter::WritePoint( const BPPoint & point )
{
	fwrite( &point, sizeof( BPPoint ), 1, m_pFile );
}
