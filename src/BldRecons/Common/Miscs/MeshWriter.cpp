#include "StdAfx.h"
#include "MeshWriter.h"

CMeshWriter::CMeshWriter(void)
{
	m_pFile = NULL;
}

CMeshWriter::~CMeshWriter(void)
{
}

void CMeshWriter::OpenFile( char filename[] )
{
	fopen_s( & m_pFile, filename, "w" );
}

void CMeshWriter::WriteHeader()
{
	fprintf_s( m_pFile, "#\n" );
}

void CMeshWriter::WriteVertex( const double v[ 3 ] )
{
	fprintf_s( m_pFile, "v %.8f %.8f %.8f\n", v[0], v[1], v[2] );
}

void CMeshWriter::WriteFace( const int i[ 3 ] )
{
	fprintf_s( m_pFile, "f %d %d %d\n", i[0]+1, i[1]+1, i[2]+1 );
}

void CMeshWriter::WriteQuad( const int i[ 4 ], bool split )
{
	if ( split ) {
		fprintf_s( m_pFile, "f %d %d %d\n", i[0]+1, i[1]+1, i[2]+1 );
		fprintf_s( m_pFile, "f %d %d %d\n", i[2]+1, i[3]+1, i[0]+1 );
	} else {
		fprintf_s( m_pFile, "f %d %d %d %d\n", i[0]+1, i[1]+1, i[2]+1, i[3]+1 );
	}
}

void CMeshWriter::CloseFile()
{
	fclose( m_pFile );
}
