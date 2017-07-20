#pragma once

#include "Miscs\HistCommon.h"
#include "Miscs\Histogram.h"
#include "Geometry\Vector3D.h"

class CHistReader
{
public:
	CHistReader(void);
	~CHistReader(void);

protected:
	FILE * m_pFile;
	
public:
	void OpenFile( char filename[] );
	HistHeader ReadHeader( );
	void ReadCenter( CVector3D & center );
	void ReadHistogram( CHistogram & histogram );
	void CloseFile();
};
