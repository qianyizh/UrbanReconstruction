#pragma once

#include "Miscs\BPCommon.h"
#include "Lidar\LidarCommon.h"

class CBPReader
{
public:
	CBPReader(void);
	~CBPReader(void);

protected:
	FILE * m_pFile;
	
public:
	void OpenFile( char filename[] );
	BPHeader ReadHeader( );
	BPPoint ReadPoint( );
	void CloseFile();
};
