#pragma once

#include "Geometry\Outline.h"

class COBJWriter
{
public:
	COBJWriter(void);
	~COBJWriter(void);

protected:
	FILE * m_pFile;
	int m_nVertexOffset;
	
public:
	void OpenFile( char filename[] );
	void WriteHeader(  );
	void WriteOutline( COutline & outline, double ground_z, bool wall_rectangle = false );
	void CloseFile();
};
