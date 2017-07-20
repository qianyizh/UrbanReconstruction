#pragma once

#include "Lidar\LidarCommon.h"
#include "Streaming\SPBCommon.h"

class CMeshGrid;

class CMeshChunk
{
public:
	CMeshChunk( int index, int number, CMeshGrid * grid );
	~CMeshChunk( void );

public:
	std::vector< PatchPointData > m_vecPatchPointData;
	std::vector< PatchPointDataVector > m_vecGridIndex;
	
	int m_iPointIndex;
	int m_iIndex;
	int m_iX;
	int m_iY;
	CVector3D m_vPosition;

protected:
	CMeshGrid * m_pGrid;

public:
	void PushPoint( SPBPoint * p );
	void BuildGridIndex();

public:
	int Index( const CVector3D & v );
	int Index( int x, int y );
	void RegularizeX( int & x );
	void RegularizeY( int & y );
};
