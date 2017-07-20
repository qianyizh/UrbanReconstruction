#pragma once

#include "Lidar\LidarCommon.h"
#include "Streaming\SPACommon.h"

class CSplitGrid;

class CSplitChunk
{
public:
	enum SplitChunkState {
		SCS_New = 0,
		SCS_Read = 1,
		SCS_Splitted = 2,
		SCS_Written = 3
	};

public:
	CSplitChunk( int index, int number, CSplitGrid * grid );
	~CSplitChunk( void );

public:
	std::vector< PatchPointData > m_vecPatchPointData;
	std::vector< PatchPointDataVector > m_vecGridIndex;
	
	int m_iPointIndex;
	int m_iIndex;
	int m_iX;
	int m_iY;
	CVector3D m_vPosition;

protected:
	CSplitGrid * m_pGrid;

public:
	void PushPoint( SPAPoint * p );
	void BuildGridIndex();

public:
	int Index( const CVector3D & v );
	int Index( int x, int y );
	void RegularizeX( int & x );
	void RegularizeY( int & y );
};
