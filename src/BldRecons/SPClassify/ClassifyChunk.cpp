#include "StdAfx.h"
#include "ClassifyChunk.h"
#include "ClassifyGrid.h"

CClassifyChunk::CClassifyChunk( int index, int number, CClassifyGrid * grid ) :
	m_vecPointData( number ),
	m_iPointIndex( 0 ),
	m_pGrid( grid ),
	m_iIndex( index ),
	m_iX( index / grid->m_nSideNumber ),
	m_iY( index % grid->m_nSideNumber ),
	m_vPosition( grid->m_cBoundingBox.m_vMin + CVector3D( m_iX * grid->m_dbUnit[ 0 ], m_iY * grid->m_dbUnit[ 1 ], 0.0 ) )
{
}

CClassifyChunk::~CClassifyChunk(void)
{
}

void CClassifyChunk::PushPoint( SPPoint * p )
{
	m_vecPointData[ m_iPointIndex ].type = PT_Unclassified;
	m_vecPointData[ m_iPointIndex ].v = CVector3D( p->pos[ 0 ], p->pos[ 1 ], p->pos[ 2 ] );
	m_iPointIndex ++;
}

void CClassifyChunk::BuildGridIndex()
{
	m_vecGridIndex.clear();
	m_vecGridIndex.resize( m_pGrid->m_nUnitNumber[ 0 ] * m_pGrid->m_nUnitNumber[ 1 ] );

	for ( int i = 0; i < ( int )m_vecPointData.size(); i++ ) {
		m_vecGridIndex[ Index( m_vecPointData[ i ].v ) ].push_back( & ( m_vecPointData[ i ] ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// auxiliary functions
//////////////////////////////////////////////////////////////////////////

int CClassifyChunk::Index( int x, int y )
{
	return ( x * m_pGrid->m_nUnitNumber[ 1 ] + y );
}

int CClassifyChunk::Index( const CVector3D & v )
{
	CVector3D diff = v - m_vPosition;
	int x = ( int )( diff[0] / m_pGrid->m_dbGridLength ) ;
	RegularizeX( x );
	int y = ( int )( diff[1] / m_pGrid->m_dbGridLength );
	RegularizeY( y );
	return Index( x, y );
}

void CClassifyChunk::RegularizeX( int & x )
{
	if ( x < 0 )
		x = 0;
	if ( x >= m_pGrid->m_nUnitNumber[ 0 ] )
		x = m_pGrid->m_nUnitNumber[ 0 ] - 1;
}

void CClassifyChunk::RegularizeY( int & y )
{
	if ( y < 0 )
		y = 0;
	if ( y >= m_pGrid->m_nUnitNumber[ 1 ] )
		y = m_pGrid->m_nUnitNumber[ 1 ] - 1;
}
