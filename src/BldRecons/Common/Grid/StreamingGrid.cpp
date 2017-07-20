#include "StdAfx.h"
#include "StreamingGrid.h"

CStreamingGrid::CStreamingGrid(void)
{
}

CStreamingGrid::~CStreamingGrid(void)
{
}

int CStreamingGrid::Index( const CVector3D & v )
{
	CVector3D diff = v - m_cBoundingBox.m_vMin;
	int x = ( int )( diff[0] / m_dbUnit[0] );
	Regularize( x );
	int y = ( int )( diff[1] / m_dbUnit[1] );
	Regularize( y );
	return Index( x, y );
}

void CStreamingGrid::Regularize( int & i )
{
	if ( i < 0 )
		i = 0;
	if ( i >= m_nSideNumber )
		i = m_nSideNumber - 1;
}

void CStreamingGrid::InitPrintProgress()
{
	m_nProgressNumber = 0;
	m_nReadNumber = 0;
	fprintf_s( stderr, "  0.0%%" );
}

void CStreamingGrid::PrintProgress()
{
	int progress = ( int )( m_nReadNumber * 1000.0 / m_nPointNumber );
	if ( progress > m_nProgressNumber ) {
		m_nProgressNumber = progress;
		fprintf_s( stderr, "\b\b\b\b\b\b%3d.%1d%%", m_nProgressNumber / 10, m_nProgressNumber % 10 );
	}
}

void CStreamingGrid::IncReadNumber()
{
	m_nReadNumber++;
}