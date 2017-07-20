#include "StdAfx.h"
#include "HistReader.h"

#include <io.h>
#include <fcntl.h>

CHistReader::CHistReader(void)
{
	m_pFile = NULL;
}

CHistReader::~CHistReader(void)
{
}

void CHistReader::OpenFile( char filename[] )
{
	fopen_s( &m_pFile, filename, "rb" );
}

void CHistReader::CloseFile()
{
	fclose( m_pFile );
}

HistHeader CHistReader::ReadHeader()
{
	//struct HistHeader {
	//	char signature[16];
	//	int version;
	//	int number;
	//	double center_distance;
	//	double histogram_distance;
	//};

	HistHeader header;
	fread( &header, sizeof( HistHeader ), 1, m_pFile );

	if ( strcmp( header.signature, signature_hist ) != 0 ) {
		fprintf_s( stderr, "HIST file signature unrecognized!\n" );
	}

	return header;
}

void CHistReader::ReadCenter( CVector3D & center )
{
	fread( &center, sizeof( CVector3D ), 1, m_pFile );
}

void CHistReader::ReadHistogram( CHistogram & histogram )
{
	histogram.Load( m_pFile );
}
