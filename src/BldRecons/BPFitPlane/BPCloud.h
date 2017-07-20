#pragma once

#include "Geometry\BoundingBox.h"
#include "Geometry\Outline.h"
#include "Lidar\LidarCommon.h"
#include "Miscs\BPReader.h"
#include "Miscs\BPWriter.h"

class CBPCloud
{
public:
	CBPCloud(void);
	~CBPCloud(void);

public:
	std::vector< PatchPointData > m_vecPoint;
	CBPReader m_cReader;
	CBPWriter m_cWriter;
	CBoundingBox m_cBoundingBox;
	PatchIndex m_n64Patch;
	double m_dbGridLength;
	double m_dbGroundZ;
	int m_nUnitNumber[ 2 ];

public:
	void LoadFromBP( char filename[] );
	void SaveToBP( char filename[] );
};
