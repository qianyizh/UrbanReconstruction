#include "StdAfx.h"
#include ".\boundingbox.h"

CBoundingBox::CBoundingBox(void)
{
	Reset();
}

CBoundingBox::~CBoundingBox(void)
{
}

void CBoundingBox::Reset()
{
	m_vMin[0] = m_vMin[1] = m_vMin[2] = 1e10;
	m_vMax[0] = m_vMax[1] = m_vMax[2] = -1e10;
}

void CBoundingBox::Push( float m_fVertex[3] )
{
	for (int i = 0; i < 3; i++) {
		if (m_fVertex[i] < m_vMin[i])
			m_vMin[i] = m_fVertex[i];
		if (m_fVertex[i] > m_vMax[i])
			m_vMax[i] = m_fVertex[i];
	}
}

void CBoundingBox::Push( double m_dbVertex[3] )
{
	for (int i = 0; i < 3; i++) {
		if (m_dbVertex[i] < m_vMin[i])
			m_vMin[i] = m_dbVertex[i];
		if (m_dbVertex[i] > m_vMax[i])
			m_vMax[i] = m_dbVertex[i];
	}
}

void CBoundingBox::Push( const CVector3D & v )
{
	for (int i = 0; i < 3; i++) {
		if (v.pVec[i] < m_vMin[i])
			m_vMin[i] = v.pVec[i];
		if (v.pVec[i] > m_vMax[i])
			m_vMax[i] = v.pVec[i];
	}
}

CVector3D CBoundingBox::GetPosition()
{
	return CVector3D(
		(m_vMin[0] + m_vMax[0]) / 2.0,
		(m_vMin[1] + m_vMax[1]) / 2.0,
		(m_vMin[2] + m_vMax[2]) / 2.0
	);
}

double CBoundingBox::GetLength()
{
	double temp[3] = { m_vMax[0] - m_vMin[0], m_vMax[1] - m_vMin[1], m_vMax[2] - m_vMin[2] };
	return __max( temp[2], __max( temp[0], temp[1] ) );
}

double CBoundingBox::GetLength( int i )
{
	return m_vMax[ i ] - m_vMin[ i ];
}

CVector3D CBoundingBox::GetCorner(int i, int j, int k)
{
	CVector3D vPos = GetPosition();
	double dbLength = GetLength();
	return vPos + CVector3D(
		(i - 0.5) * dbLength,
		(j - 0.5) * dbLength,
		(k - 0.5) * dbLength
	);
}
