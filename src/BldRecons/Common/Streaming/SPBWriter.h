#pragma once

#include "Streaming\SPBCommon.h"
#include "Grid\StreamingGrid.h"
#include "Lidar\LidarCommon.h"

class CSPBWriter
{
public:
	CSPBWriter(void);
	~CSPBWriter(void);

protected:
	FILE * m_pFile;
	CStreamingGrid * m_pGrid;
	ElementBufferB m_cBuffer;
	int m_nElementsInBuffer;
	
public:
	void OpenFile( char filename[] );
	void RegisterGrid( CStreamingGrid * grid );
	void WriteHeader();
	void WritePoint( const CVector3D & v );
	void WritePoint( const PatchPointData & point );
	void WriteBeginCell( int index, int number );
	void WriteFinalizeCell( int index, int number );
	void WriteEOF();
	void CloseFile();

private:
	void InitBuffer();
};
