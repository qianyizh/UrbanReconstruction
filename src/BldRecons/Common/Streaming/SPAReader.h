#pragma once

#include "Streaming\SPACommon.h"
#include "Grid\StreamingGrid.h"

class CSplitGrid;

class CSPAReader
{
public:
	CSPAReader(void);
	~CSPAReader(void);

protected:
	FILE * m_pFile;
	ElementBufferA m_cBuffer;
	int m_nElementsInBuffer;
	char * m_pElementInBuffer;
	CStreamingGrid * m_pGrid;

public:
	void OpenFile( char filename[] );
	void RegisterGrid( CStreamingGrid * grid );
	void ReadHeader();
	bool ReadNextElement();
	SPAPoint * GetPoint();
	SPACell * GetCell();
	void CloseFile();

private:
	void InitBuffer();
};
