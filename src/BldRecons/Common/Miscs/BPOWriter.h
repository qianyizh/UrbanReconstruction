#pragma once

#include "Miscs\BPOCommon.h"
#include "Lidar\LidarCommon.h"
#include "Geometry\Outline.h"

class CBPOWriter
{
public:
	CBPOWriter(void);
	~CBPOWriter(void);

protected:
	FILE * m_pFile;
	
public:
	void OpenFile( char filename[] );
	void WriteHeader( PatchIndex patch, int number, double ground_z, double grid_length );
	void WriteOutline( COutline & outline );
	void CloseFile();
};
