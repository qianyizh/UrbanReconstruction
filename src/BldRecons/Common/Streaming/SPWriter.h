#pragma once

#include "Streaming\SPCommon.h"
#include "Grid\StreamingGrid.h"

class CSPWriter
{
public:
	CSPWriter(void);
	~CSPWriter(void);

protected:
	FILE * m_pFile;
	CStreamingGrid * m_pGrid;
	ElementBuffer m_cBuffer;
	int m_nElementsInBuffer;
	
public:
	void OpenFile( char filename[] );
	void RegisterGrid( CStreamingGrid * grid );
	void WriteHeader();
	void WritePoint( const CVector3D & v );
	void WriteBeginCell( int index, int number );
	void WriteFinalizeCell( int index, int number );
	void WriteEOF();
	void CloseFile();

private:
	void InitBuffer();
};
