#include "StdAfx.h"
#include "IndexingGrid.h"

CIndexingGrid::CIndexingGrid(void)
{
}

CIndexingGrid::~CIndexingGrid(void)
{
}

void CIndexingGrid::BuildIndexingGrid( CPointCloud * pointcloud, double unit_length )
{
	m_pPointCloud = pointcloud;
	m_dbGridLength = unit_length;
	CBoundingBox & box = pointcloud->m_cBoundingBox;

	// setup protected boundary
	m_nWidth = ( int )( box.GetLength( 0 ) / unit_length ) + 1 + 2;
	m_nHeight = ( int )( box.GetLength( 1 ) / unit_length ) + 1 + 2;

	// create index
	CreateIndex();
}

void CIndexingGrid::CreateIndex()
{
	m_vecIndex.clear();
	m_vecIndex.resize( m_nWidth * m_nHeight );

	for ( int i = 0; i < ( int )m_pPointCloud->m_vecPoint.size(); i++ ) {
		CVector3D & v = m_pPointCloud->m_vecPoint[ i ];
		m_vecIndex[ Index( v[ 0 ], v[ 1 ] ) ].push_back( i );
	}
}