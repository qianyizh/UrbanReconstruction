#include "StdAfx.h"
#include "MeshChunk.h"
#include "MeshGrid.h"

CMeshChunk::CMeshChunk( int index, int number, CMeshGrid * grid ) :
	m_vecPatchPointData( number ),
	m_iPointIndex( 0 ),
	m_pGrid( grid ),
	m_iIndex( index ),
	m_iX( index / grid->m_nSideNumber ),
	m_iY( index % grid->m_nSideNumber ),
	m_vPosition( grid->m_cBoundingBox.m_vMin + CVector3D( m_iX * grid->m_dbUnit[ 0 ], m_iY * grid->m_dbUnit[ 1 ], 0.0 ) )
{
}

CMeshChunk::~CMeshChunk(void)
{
}

void CMeshChunk::PushPoint( SPBPoint * p )
{
	m_vecPatchPointData[ m_iPointIndex ].type = ( PointType )p->type;
	m_vecPatchPointData[ m_iPointIndex ].v = CVector3D( p->pos[ 0 ], p->pos[ 1 ], p->pos[ 2 ] );
	m_vecPatchPointData[ m_iPointIndex ].n = CVector3D( p->n[ 0 ], p->n[ 1 ], p->n[ 2 ] );
	m_vecPatchPointData[ m_iPointIndex ].flatness = p->flatness;
	m_vecPatchPointData[ m_iPointIndex ].patch.base = p->patch;
	m_vecPatchPointData[ m_iPointIndex ].patch.num = 0;
	m_iPointIndex ++;
}

void CMeshChunk::BuildGridIndex()
{
	m_vecGridIndex.clear();
	m_vecGridIndex.resize( m_pGrid->m_nUnitNumber[ 0 ] * m_pGrid->m_nUnitNumber[ 1 ] );

	for ( int i = 0; i < ( int )m_vecPatchPointData.size(); i++ ) {
		m_vecGridIndex[ Index( m_vecPatchPointData[ i ].v ) ].push_back( & ( m_vecPatchPointData[ i ] ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// auxiliary functions
//////////////////////////////////////////////////////////////////////////

int CMeshChunk::Index( int x, int y )
{
	return ( x * m_pGrid->m_nUnitNumber[ 1 ] + y );
}

int CMeshChunk::Index( const CVector3D & v )
{
	CVector3D diff = v - m_vPosition;
	int x = ( int )( diff[0] / m_pGrid->m_dbGridLength ) ;
	RegularizeX( x );
	int y = ( int )( diff[1] / m_pGrid->m_dbGridLength );
	RegularizeY( y );
	return Index( x, y );
}

void CMeshChunk::RegularizeX( int & x )
{
	if ( x < 0 )
		x = 0;
	if ( x >= m_pGrid->m_nUnitNumber[ 0 ] )
		x = m_pGrid->m_nUnitNumber[ 0 ] - 1;
}

void CMeshChunk::RegularizeY( int & y )
{
	if ( y < 0 )
		y = 0;
	if ( y >= m_pGrid->m_nUnitNumber[ 1 ] )
		y = m_pGrid->m_nUnitNumber[ 1 ] - 1;
}
