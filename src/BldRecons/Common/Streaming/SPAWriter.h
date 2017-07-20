#pragma once

#include "Streaming\SPACommon.h"
#include "Grid\StreamingGrid.h"
#include "Lidar\LidarCommon.h"

class CClassifyGrid;

class CSPAWriter
{
public:
	CSPAWriter(void);
	~CSPAWriter(void);

protected:
	FILE * m_pFile;
	CStreamingGrid * m_pGrid;
	ElementBufferA m_cBuffer;
	int m_nElementsInBuffer;
	
public:
	void OpenFile( char filename[] );
	void RegisterGrid( CStreamingGrid * grid );
	void WriteHeader();
	void WritePoint( const CVector3D & v );
	void WritePoint( const PointData & point );
	void WriteBeginCell( int index, int number );
	void WriteFinalizeCell( int index, int number );
	void WriteEOF();
	void CloseFile();

private:
	void InitBuffer();
};
