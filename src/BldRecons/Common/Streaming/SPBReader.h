#pragma once

#include "Streaming\SPBCommon.h"
#include "Grid\StreamingGrid.h"

class CSPBReader
{
public:
	CSPBReader(void);
	~CSPBReader(void);

protected:
	FILE * m_pFile;
	ElementBufferB m_cBuffer;
	int m_nElementsInBuffer;
	char * m_pElementInBuffer;
	CStreamingGrid * m_pGrid;

public:
	void OpenFile( char filename[] );
	void RegisterGrid( CStreamingGrid * grid );
	void ReadHeader();
	bool ReadNextElement();
	SPBPoint * GetPoint();
	SPBCell * GetCell();
	void CloseFile();

private:
	void InitBuffer();
};
