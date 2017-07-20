#pragma once

#include "Vector3D.h"

class CBoundingBox
{
public:
	CBoundingBox(void);
	~CBoundingBox(void);

public:
	CVector3D m_vMin;
	CVector3D m_vMax;

	void Reset();
	void Push( float m_fVertex[3] );
	void Push( double m_dbVertex[3] );
	void Push( const CVector3D & v );
	CVector3D GetPosition();
	double GetLength();
	double GetLength( int i );
	CVector3D GetCorner(int i, int j, int k);

	void PrintInfo() {
		fprintf_s( stderr, "Bounding Box:\n\tFrom : ( %.2f, %.2f, %.2f )\n\tTo :   ( %.2f, %.2f, %.2f )\n", m_vMin[0], m_vMin[1], m_vMin[2], m_vMax[0], m_vMax[1], m_vMax[2] );
	}
};
