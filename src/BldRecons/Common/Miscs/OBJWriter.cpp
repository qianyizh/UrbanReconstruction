#include "StdAfx.h"
#include "OBJWriter.h"

#include <io.h>
#include <fcntl.h>

COBJWriter::COBJWriter(void)
{
	m_pFile = NULL;
}

COBJWriter::~COBJWriter(void)
{
}

void COBJWriter::OpenFile( char filename[] )
{
	fopen_s( &m_pFile, filename, "w" );
}

void COBJWriter::CloseFile()
{
	fclose( m_pFile );
}

void COBJWriter::WriteHeader(  )
{
	if ( m_pFile == NULL )
		return;

	fprintf_s( m_pFile, "#\n" );
	m_nVertexOffset = 1;
}

void COBJWriter::WriteOutline( COutline & outline, double ground_z, bool wall_rectangle )
{
	outline.GenerateRoofTriangle();
	outline.WriteOBJVertex( m_pFile, ground_z );
	outline.WriteOBJFace( m_pFile, m_nVertexOffset, wall_rectangle );
	m_nVertexOffset += ( int )outline.m_vecVertex.size() * 2;
}
