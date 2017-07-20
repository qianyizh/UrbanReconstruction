#include "StdAfx.h"
#include "HistWriter.h"

#include <io.h>
#include <fcntl.h>

CHistWriter::CHistWriter(void)
{
	m_pFile = NULL;
}

CHistWriter::~CHistWriter(void)
{
}

void CHistWriter::OpenFile( char filename[] )
{
	fopen_s( &m_pFile, filename, "wb" );
}

void CHistWriter::CloseFile()
{
	fclose( m_pFile );
}

void CHistWriter::WriteHeader( int number, double center_distance, double histogram_distance )
{
	if ( m_pFile == NULL )
		return;

	//struct HistHeader {
	//	char signature[16];
	//	int version;
	//	int number;
	//	double center_distance;
	//	double histogram_distance;
	//};

	HistHeader header;

	memset( header.signature, 0, 16 );
	memcpy_s( header.signature, 16, signature_hist, strlen( signature_hist ) );

	header.version = 1;

	header.number = number;

	header.center_distance = center_distance;

	header.histogram_distance = histogram_distance;

	fwrite( &header, sizeof( HistHeader ), 1, m_pFile );
}

void CHistWriter::WriteCenter( CVector3D & center )
{
	fwrite( &center, sizeof( CVector3D ), 1, m_pFile );
}

void CHistWriter::WriteHistogram( CHistogram & histogram )
{
	histogram.Save( m_pFile );
}
