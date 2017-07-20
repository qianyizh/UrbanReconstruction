#pragma once

#include "Miscs\BPCommon.h"
#include "Lidar\LidarCommon.h"

class CBPWriter
{
public:
	CBPWriter(void);
	~CBPWriter(void);

protected:
	FILE * m_pFile;
	
public:
	void OpenFile( char filename[] );
	void WriteHeader( PatchIndex patch, int number, double ground_z, double grid_length );
	void WritePoint( const double pos[3], const double n[3], const double flatness, const __int64 plane );
	void WritePoint( const BPPoint & point );
	void CloseFile();
};
