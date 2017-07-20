#include "StdAfx.h"
#include "DCGrid.h"

CDCGrid::CDCGrid(void)
{
}

CDCGrid::~CDCGrid(void)
{
}

//////////////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////////////

void CDCGrid::AssignPointCloud( CPointCloud * pointcloud, double unit_length, double ground_z, bool enable_topology_check )
{
	CIndexingGrid::BuildIndexingGrid( pointcloud, unit_length );
	pointcloud->m_dbGroundZ = ground_z;
	m_bTopologyCheck = enable_topology_check;

}

void CDCGrid::ComputeHermiteData( int accept_number, double relative_distance, double relative_z )
{
	// parameters set the segmentation criteria
	// points are assigned to the same segmentation iff. point_distance < relative_distance && z_difference < relative_z
	// segments with less than accept_number points are dropped as outliers

	m_nAcceptNumber = accept_number;
	m_dbSegmentationDistance = relative_distance * m_dbGridLength;
	m_dbSegmentationZ = relative_z * m_dbGridLength;

	double max_length = m_nHeight > m_nWidth ? m_nHeight - 1 : m_nWidth - 1;
	m_nLevel = 1 + ( int )( log( ( double )max_length ) / log( 2.0 ) );
	m_nHDSideNumber = ( 1 << m_nLevel ) + 1;
	m_vecHermiteDataXY.resize( m_nHDSideNumber * m_nHDSideNumber );
	m_vecHermiteDataZ.resize( m_nHDSideNumber * m_nHDSideNumber * 2 );

	ComputeHermiteData_XY();
	ComputeHermiteData_Z();
}

void CDCGrid::DualContouringGeometry( double boundary_weight, double error_tolerance, double singular_tolerance )
{
	m_dbBoundaryWeight = boundary_weight;
	m_dbErrorTolerance = error_tolerance;
	m_dbSingularTolerance = singular_tolerance;

	m_vecNodes.clear();
	m_vecNodes.resize( m_nLevel + 1 );

	BuildQuadTreeAtLevel0();

	for ( int l = 1; l <= m_nLevel; l++ ) {
		BuildQuadTreeAtLevelN( l );
	}
	//PrintLayerNumber();
}
