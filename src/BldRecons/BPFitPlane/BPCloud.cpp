#include "StdAfx.h"
#include "BPCloud.h"

CBPCloud::CBPCloud(void)
{
}

CBPCloud::~CBPCloud(void)
{
}

void CBPCloud::LoadFromBP( char filename[] )
{

	m_cReader.OpenFile( filename );
	BPHeader header = m_cReader.ReadHeader();

	m_n64Patch = header.patch;
	m_dbGridLength = header.grid_length;
	m_dbGroundZ = header.ground_z;
	m_cBoundingBox.Reset();

	m_vecPoint.clear();
	m_vecPoint.resize( header.number );

	for ( int i = 0; i < header.number; i++ ) {
		BPPoint point = m_cReader.ReadPoint();

		m_vecPoint[ i ].type = PT_Building;
		m_vecPoint[ i ].patch.base = point.plane;
		m_vecPoint[ i ].patch.num = 0;
		m_vecPoint[ i ].v = CVector3D( point.pos );
		m_vecPoint[ i ].n = CVector3D( point.n );
		m_vecPoint[ i ].flatness = point.flatness;
 
		m_cBoundingBox.Push( m_vecPoint[ i ].v );
	}

	m_cReader.CloseFile();

	m_nUnitNumber[ 0 ] = ( int )( m_cBoundingBox.GetLength( 0 ) / m_dbGridLength ) + 3;
	m_nUnitNumber[ 1 ] = ( int )( m_cBoundingBox.GetLength( 1 ) / m_dbGridLength ) + 3;

}

void CBPCloud::SaveToBP( char filename[] )
{
	
	m_cWriter.OpenFile( filename );

	m_cWriter.WriteHeader( m_n64Patch, ( int )m_vecPoint.size(), m_dbGroundZ, m_dbGridLength );

	for ( int i = 0; i < ( int )m_vecPoint.size(); i++ ) {
		BPPoint point;

		point.plane = m_vecPoint[ i ].patch.base;
		point.flatness = m_vecPoint[ i ].flatness;
		point.pos[ 0 ] = m_vecPoint[ i ].v[ 0 ];
		point.pos[ 1 ] = m_vecPoint[ i ].v[ 1 ];
		point.pos[ 2 ] = m_vecPoint[ i ].v[ 2 ];
		point.n[ 0 ] = m_vecPoint[ i ].n[ 0 ];
		point.n[ 1 ] = m_vecPoint[ i ].n[ 1 ];
		point.n[ 2 ] = m_vecPoint[ i ].n[ 2 ];

		m_cWriter.WritePoint( point );
	}

	m_cWriter.CloseFile();

}
