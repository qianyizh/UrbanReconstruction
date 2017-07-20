#pragma once

#include "Miscs\HistCommon.h"
#include "Miscs\Histogram.h"
#include "Geometry\Vector3D.h"

class CHistWriter
{
public:
	CHistWriter(void);
	~CHistWriter(void);

protected:
	FILE * m_pFile;
	
public:
	void OpenFile( char filename[] );
	void WriteHeader( int number, double center_distance, double histogram_distance );
	void WriteCenter( CVector3D & center );
	void WriteHistogram( CHistogram & histogram );
	void CloseFile();
};
