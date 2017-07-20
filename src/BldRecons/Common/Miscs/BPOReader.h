#pragma once

#include "Miscs\BPOCommon.h"
#include "Geometry\Outline.h"

class CBPOReader
{
public:
	CBPOReader(void);
	~CBPOReader(void);

protected:
	FILE * m_pFile;
	
public:
	void OpenFile(const char filename[]);
	BPOHeader ReadHeader( );
	void ReadOutline( COutline & outline );
	void CloseFile();
};
