#pragma once

#include <vector>

#include "Vector3D.h"
#include "BoundingBox.h"

class CPointCloud
{
public:
	CPointCloud(void);
	~CPointCloud(void);

public:
	std::vector< CVector3D > m_vecPoint;
	std::vector< CVector3D > m_vecNormal;

	CBoundingBox m_cBoundingBox;
	double m_dbGroundZ;

public:
	void LoadFromXYZN( char filename[] );
	void SaveToXYZN( char filename[] );
};
