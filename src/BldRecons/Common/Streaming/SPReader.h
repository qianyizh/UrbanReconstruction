#pragma once

#include "Streaming\SPCommon.h"
#include "Grid\StreamingGrid.h"

class CClassifyGrid;

class CSPReader
{
public:
	CSPReader(void);
	~CSPReader(void);

protected:
	FILE * m_pFile;
	ElementBuffer m_cBuffer;
	CStreamingGrid * m_pGrid;
	int m_nElementsInBuffer;
	char * m_pElementInBuffer;

public:
	void OpenFile( char filename[] );
	void RegisterGrid( CStreamingGrid * grid );
	void ReadHeader();
	bool ReadNextElement();
	SPPoint * GetPoint();
	SPCell * GetCell();
	void CloseFile();

private:
	void InitBuffer();
};
