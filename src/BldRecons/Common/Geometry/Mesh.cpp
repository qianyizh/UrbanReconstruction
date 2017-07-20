#include "StdAfx.h"
#include "Mesh.h"

#include "Miscs\MeshWriter.h"

CMesh::CMesh(void)
{
}

CMesh::~CMesh(void)
{
}

void CMesh::SaveToObj( char filename[] )
{
	CMeshWriter writer;
	writer.OpenFile( filename );
	writer.WriteHeader();

	for ( int i = 0; i < ( int )m_vecVertex.size(); i++ ) {
		writer.WriteVertex( m_vecVertex[ i ].v.pVec );
	}

	for ( int i = 0; i < ( int )m_vecTriangle.size(); i++ ) {
		writer.WriteFace( m_vecTriangle[ i ].i );
	}

	for ( int i = 0; i < ( int )m_vecQuad.size(); i++ ) {
		writer.WriteQuad( m_vecQuad[ i ].i, false );
	}

	writer.CloseFile();
}