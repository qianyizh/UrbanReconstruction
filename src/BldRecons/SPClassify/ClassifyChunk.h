#pragma once

#include "Lidar\LidarCommon.h"
#include "Streaming\SPCommon.h"

class CClassifyGrid;

class CClassifyChunk
{
public:
	enum ClassifyChunkState {
		CCS_New = 0,
		CCS_Read = 1,
		CCS_Normaled = 2,
		CCS_Classified = 3,
		CCS_Refined = 4,
		CCS_Written = 5
	};

public:
	CClassifyChunk( int index, int number, CClassifyGrid * grid );
	~CClassifyChunk(void);

public:
	std::vector< PointData > m_vecPointData;
	std::vector< PointDataVector > m_vecGridIndex;
	//ClassifyChunkState m_cState;
	int m_iPointIndex;
	int m_iIndex;
	int m_iX;
	int m_iY;
	CVector3D m_vPosition;

protected:
	CClassifyGrid * m_pGrid;

public:
	void PushPoint( SPPoint * p );
	void BuildGridIndex();

public:
	int Index( const CVector3D & v );
	int Index( int x, int y );
	void RegularizeX( int & x );
	void RegularizeY( int & y );
};
