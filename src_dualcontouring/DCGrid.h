#pragma once

#include "common\IndexingGrid.h"

class CDCGrid :
	public CIndexingGrid
{
public:
	CDCGrid(void);
	~CDCGrid(void);

public:
	friend class CDCContourer;

public:
	//////////////////////////////////////////////////////////////////////////
	// definition of all the auxiliary data structures
	//////////////////////////////////////////////////////////////////////////
	typedef std::vector< CVector3D * > CVector3DPointer_Vector;

	struct NodeLocator {
		int l;
		int i;
		int j;
		NodeLocator( int ll, int ii, int jj ) : l(ll), i(ii), j(jj) {}
		NodeLocator() {}
	};

	struct HermiteData {
		CVector3D v;		// position
		CVector3D n;		// normal
	};
	typedef std::vector< HermiteData > HermiteData_Vector;

	struct DistanceSorting {
		int index;
		double distance;
		DistanceSorting( int i, double d ) { index = i; distance = d; }
		DistanceSorting() { index = 0; distance = 0.0; }
	};
	typedef std::vector< DistanceSorting > DistanceSorting_Vector;

	struct HermiteDataIndex {
		int i;
		int l;
		HermiteDataIndex( int ii, int ll ) : i(ii), l(ll) {}
	};
	typedef std::vector< HermiteDataIndex > HermiteDataIndex_Vector;

	struct ClusterAssignment {
		int l[2][2];
		ClusterAssignment() { l[0][0] = l[0][1] = l[1][0] = l[1][1] = -1; }
	};
	typedef std::vector< ClusterAssignment > ClusterAssignment_Vector;

	struct DCGridNode {
		bool pass;							// if the subtree collapse passed the geometry error testing
		char level;							// at which level
		bool is_ground;						// if all the data in this node is ground data

		char num_of_layers;					// total roof layer number in this node. ** IMPORTATNT variable **
		std::vector< CVector3D > means;		// centroid of the intersection points on each roof layer, as "guessed" solution
		std::vector< CVector3D > points;	// final result, ** OUTPUT **
		
		std::vector< float > r;
		std::vector< int > cluster;
		std::vector< int > leaf_cluster_to_root_cluster;

		double error;
		bool mean_based_on_z;

		// for contouring
		int vi;
		DistanceSorting_Vector seq;

		// for shape analysis
		CVector3D v[ 3 ];
		double sigma2[ 3 ];
	};
	typedef std::vector< DCGridNode > DCGridNode_Vector;

protected:
	//////////////////////////////////////////////////////////////////////////
	// member variables
	//////////////////////////////////////////////////////////////////////////
	bool m_bTopologyCheck;
	int m_nAcceptNumber;
	double m_dbSegmentationDistance;
	double m_dbSegmentationZ;
	double m_dbBoundaryWeight;
	double m_dbErrorTolerance;
	double m_dbSingularTolerance;

	int m_nLevel;
	int m_nHDSideNumber;

	HermiteData_Vector m_vecHermiteDataXY;
	HermiteData_Vector m_vecHermiteDataZ;

	std::vector< DCGridNode_Vector > m_vecNodes;
	ClusterAssignment_Vector m_vecClusterAssignment;

public:
	//////////////////////////////////////////////////////////////////////////
	// entrance functions
	//////////////////////////////////////////////////////////////////////////
	void AssignPointCloud( CPointCloud * pointcloud, double unit_length, double ground_z, bool enable_topology_check = true );
	void ComputeHermiteData( int accept_number = 4, double relative_distance = 1.0, double relative_z = 0.6 );
	void DualContouringGeometry( double boundary_weight, double error_tolerance, double singular_tolerance );

protected:
	//////////////////////////////////////////////////////////////////////////
	// HermiteData functions
	//////////////////////////////////////////////////////////////////////////
	void ComputeHermiteData_XY();
	void ComputeHermiteData_XY( int i, int j );
	void ComputeHermiteData_Z();
	void ComputeHermiteData_Z( int i, int j, int dir /*along_y=0; along_x=1*/ );
	void ComputeHermiteData_Z( HermiteData & hd, CVector3D v[ 2 ], CVector3DPointer_Vector & points );
	void ComputeHermiteData_Z_Key( HermiteData & hd, CVector3D v[ 2 ], CVector3D & v_key, CVector3D & v_dir );
	void ComputeHermiteData_Z_Ill( HermiteData & hd, CVector3D v[ 2 ] );

protected:
	//////////////////////////////////////////////////////////////////////////
	// geometry simplification
	//////////////////////////////////////////////////////////////////////////
	void BuildQuadTreeAtLevel0();
	void BuildQuadTreeAtLevelN( int l );
	void DualContouringGeometryNode( DCGridNode & node );
	bool TopologyCheck( NodeLocator & loc );

protected:
	//////////////////////////////////////////////////////////////////////////
	// clustering functions
	//////////////////////////////////////////////////////////////////////////
	int ClusterFromLeafNodes( NodeLocator & loc );

	void DistanceSegmentation( CVector3DPointer_Vector & points, std::vector< int > & cluster, double dis2 );
	void DistanceSegmentationEx( CVector3DPointer_Vector & points, std::vector< int > & cluster, double dis2, double dis_z );

	void InitializeCluster( std::vector< int > & cluster );
	int CompressCluster( std::vector< int > & cluster );
	int CompressClusterEx( std::vector< int > & cluster, int resize_number );
	void FlattenCluster( std::vector< int > & cluster );
	void UnionCluster( std::vector< int > & cluster, int i, int j );
	int FindCluster( std::vector< int > & cluster, int start );

protected:
	//////////////////////////////////////////////////////////////////////////
	// sorting functions
	//////////////////////////////////////////////////////////////////////////
	void SortDistance2D( CVector3DPointer_Vector & points, CVector3D & ref_v, std::vector< int > & cluster, int cluster_index, DistanceSorting_Vector & sorting_result );
	void QSort( DistanceSorting_Vector & sorting_result );

protected:
	//////////////////////////////////////////////////////////////////////////
	// numerical solvers, see DCGridNumerical.cpp
	//////////////////////////////////////////////////////////////////////////
	void svdcmp_ext(float **a, int m, int n, float w[], float **v);
	void qrdcmp_ext(float **a, int n, float *c, float *d, int *sing);
	float pythag_ext(float a, float b);

protected:
	//////////////////////////////////////////////////////////////////////////
	// small, inline functions
	//////////////////////////////////////////////////////////////////////////
	int HDIndex( int i, int j ) {
		return ( i * m_nHDSideNumber + j );
	}

	DCGridNode & LocateNode( NodeLocator & loc ) {
		return m_vecNodes[ loc.l ][ ( loc.i << ( m_nLevel - loc.l ) ) + loc.j ];
	}

	NodeLocator Leaf( NodeLocator & loc, int i, int j, bool is_leaf = false ) {
		return is_leaf ? loc : NodeLocator( loc.l - 1, ( loc.i << 1 ) + i, ( loc.j << 1 ) + j );
	}

	bool HermiteDataXY_IsGround( CVector3D & v ) { return v[ 2 ] == m_pPointCloud->m_dbGroundZ; }

	bool HermiteDataZ_IsInvalid( CVector3D & v ) { return v[ 2 ] == 0.0; }

	CVector3D AbsoluteToRelative( CVector3D & v )
	{
		return v - m_pPointCloud->m_cBoundingBox.m_vMin;
	}

	CVector3D RelativeToAbsolute( CVector3D & u )
	{
		return u + m_pPointCloud->m_cBoundingBox.m_vMin;
	}

	int Cluster_Index( int i, int j, int l ) {
		return ( i * ( ( 1 << l ) + 1 ) + j );
	}

	int RootCluster_Index( int i, int j ) {
		return Cluster_Index( i, j, m_nLevel );
	}
};
