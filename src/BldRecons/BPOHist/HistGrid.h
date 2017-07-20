#pragma once

#include "Geometry\Vector3D.h"
#include "Geometry\BoundingBox.h"
#include "Miscs\Histogram.h"
#include "Miscs\HistWriter.h"
#include "Miscs\HistReader.h"

class CHistGrid
{
public:
	CHistGrid(void);
	~CHistGrid(void);

public:
	std::vector< CVector3D > m_vecCenter;
	std::vector< CHistogram > m_vecHistogram;
	CBoundingBox m_cBoundingBox;
	int m_nGridNumber[ 2 ];
	CHistWriter m_cWriter;
	CHistReader m_cReader;

public:
	void Init();
	void Count();

public:
	void SaveToHist( char filename[] );

private:
	double GetAngle( CVector3D & d );

public:
	void Debug_SaveToTxt();
};
