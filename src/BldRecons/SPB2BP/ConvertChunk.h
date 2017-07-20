#pragma once

#include "Lidar\LidarCommon.h"
#include "Streaming\SPBCommon.h"

class CConvertGrid;

class CConvertChunk
{
public:
	CConvertChunk( int index, int number, CConvertGrid * grid );
	~CConvertChunk( void );

public:
	std::vector< PatchPointData > m_vecPatchPointData;
	
	int m_iPointIndex;
	int m_iIndex;
	int m_iX;
	int m_iY;
	CVector3D m_vPosition;

protected:
	CConvertGrid * m_pGrid;

public:
	void PushPoint( SPBPoint * p );

public:
	int Index( const CVector3D & v );
	int Index( int x, int y );
	void RegularizeX( int & x );
	void RegularizeY( int & y );
};
