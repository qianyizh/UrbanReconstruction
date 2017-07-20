#pragma once

#include <set>

#include "Geometry\IndexingGrid.h"

class CMCGrid :
	public CIndexingGrid
{
public:
	CMCGrid(void);
	~CMCGrid(void);

public:
	friend class CMCContourer;
	friend class CMCBoundary;

public:
	//////////////////////////////////////////////////////////////////////////
	// definition of all the auxiliary data structures
	//////////////////////////////////////////////////////////////////////////
	enum EdgeDirection {
		ED_Along_X = 0,
		ED_Along_Y = 1
	};

	typedef std::vector< CVector3D * > CVector3DPointer_Vector;

	struct NodeLocator {
		int l;
		int i;
		int j;
		NodeLocator( int ll, int ii, int jj ) : l(ll), i(ii), j(jj) {}
		NodeLocator() {}
	};

	struct HyperPointLocator {
		NodeLocator node;
		int i;
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

	struct MCHyperPoint {
		bool is_feature;
		bool is_collapsible;
		bool is_foldingpoint;
		bool is_ground;
		int vi;
		DistanceSorting_Vector seq;
		double sigma2[ 3 ];
		CVector3D v[ 3 ];

		double error;
		//double minimum_join_error;
		//MCHyperPoint * minimum_join_parent;
		//int minimum_join_leaf_to_parent[ 2 ];

		char num_of_layers;
		char num_of_leafs;
		int num_of_samples;
		std::vector< CVector3D > means;
		std::vector< CVector3D > points;
		std::vector< float > r;
		int leaf_to_parent[ 2 ];			// for 1-point, need only 1, for 2-point, need a flip flag, for more-point, is feature
		
		MCHyperPoint * parent;

		// topology part
		std::vector< int > bi;

		bool Is_1HPoint() { return ( !is_feature && num_of_layers == 1 ); }
		bool Is_2HPoint() { return ( !is_feature && num_of_layers == 2 ); }
	};
	typedef std::vector< MCHyperPoint > MCHyperPoint_Vector;
	typedef std::vector< MCHyperPoint * > MCHyperPointPointer_Vector;

	struct MCHalfEdge {
		CMCGrid::MCHyperPoint * h[ 2 ];
		int hl[ 2 ];
		int next;
		int twin;
		bool valid;
		int bi;
	};
	typedef std::vector< MCHalfEdge > MCHalfEdge_Vector;

	struct MCBoundary {
		int seed_edge;
		int remaining_v_num;
		std::vector< int > ei;
	};
	typedef std::vector< MCBoundary > MCBoundary_Vector;

	struct JoinElement {
		MCHyperPoint * leaf;
		MCHyperPoint * parent;
		int leaf_to_parent[ 2 ];
		
		JoinElement() { leaf = parent = NULL; leaf_to_parent[ 0 ] = 0; leaf_to_parent[ 1 ] = 1; }
		JoinElement( MCHyperPoint * l, MCHyperPoint * p, int lp[ 2 ] ) { leaf = l; parent = p; leaf_to_parent[ 0 ] = lp[ 0 ]; leaf_to_parent[ 1 ] = lp[ 1 ]; }

		bool operator < ( const JoinElement & j ) const {
			return ( leaf < j.leaf || ( ( leaf == j.leaf ) && ( parent < j.parent ) ) );
		}
	};
	typedef std::set< JoinElement > JoinElement_Set;

	struct MCGridNode {
		char level;							// at which level

		bool is_collapsible;				// be false only all the hpoints are not collapsable
		
		MCHyperPoint_Vector hpoints;

		int num_of_layers;
		std::vector< int > cluster;
		std::vector< int > leaf_cluster_to_root_cluster;
	};
	typedef std::vector< MCGridNode > MCGridNode_Vector;

protected:
	//////////////////////////////////////////////////////////////////////////
	// member variables
	//////////////////////////////////////////////////////////////////////////
	int m_nAcceptNumber;
	double m_dbSegmentationDistance;
	double m_dbSegmentationZ;
	double m_dbBoundaryWeight;
	double m_dbErrorTolerance;
	double m_dbSingularTolerance;
	int m_nMinBoundaryRemaining;

	int m_nLevel;
	int m_nHDSideNumber;

	HermiteData_Vector m_vecHermiteDataXY;
	HermiteData_Vector m_vecHermiteDataZ;

	std::vector< MCGridNode_Vector > m_vecNodes;

	MCHalfEdge_Vector m_vecTopoEdges;
	MCBoundary_Vector m_vecBoundaries;

public:
	//////////////////////////////////////////////////////////////////////////
	// entrance functions
	//////////////////////////////////////////////////////////////////////////
	void AssignPointCloud( CPointCloud * pointcloud, double unit_length, double ground_z );
	void ComputeHermiteData( int accept_number = 4, double relative_distance = 1.0, double relative_z = 0.6 );
	void DualContouringGeometry( double boundary_weight, double error_tolerance, double singular_tolerance, int min_boundary_remaining = 3 );

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
	void DualContouringGeometryHPoint( MCHyperPoint & hpoint );
	double AddHPoint( MCHyperPoint & base, MCHyperPoint & adding, int leaf_to_root[ 2 ], bool overwrite_base );

protected:
	//////////////////////////////////////////////////////////////////////////
	// feature detection
	//////////////////////////////////////////////////////////////////////////
	void DetectTopologyFeatures();

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

	MCHyperPoint * HP_FindCluster( MCHyperPoint * start );
	MCHyperPoint * HP_FindCluster( MCHyperPoint * start, int leaf_to_root[ 2 ] );
	MCHyperPoint * HP_FindCluster_Flatten( MCHyperPoint * start );
	bool HP_UnionCluster_Surface( MCHyperPoint * i, MCHyperPoint * j );
	bool HP_UnionCluster_Boundary( MCHyperPoint * i, MCHyperPoint * j, bool flip );
	void HP_SweepSurface( MCGridNode * n[ 2 ][ 2 ] );
	void HP_SweepSurface( MCGridNode * n1, MCGridNode * n2 );
	void HP_SweepSurfaceJoin( MCGridNode * n[ 2 ][ 2 ], JoinElement_Set & jset );
	void HP_SweepSurfaceJoin( MCGridNode * n1, MCGridNode * n2, int join_layer, JoinElement_Set & jset );
	void HP_SweepBoundary( MCGridNode * n[ 2 ], EdgeDirection dir );
	void HP_SweepBoundaryJoin( MCGridNode * n[ 2 ], EdgeDirection dir, JoinElement_Set & jset );
	void HP_SweepBoundaryJoin( MCGridNode * n1, MCGridNode * n2, int join_layer[ 2 ], JoinElement_Set & jset );

protected:
	//////////////////////////////////////////////////////////////////////////
	// sorting functions
	//////////////////////////////////////////////////////////////////////////
	void SortDistance2D( CVector3DPointer_Vector & points, CVector3D & ref_v, std::vector< int > & cluster, int cluster_index, DistanceSorting_Vector & sorting_result );
	void QSort( DistanceSorting_Vector & sorting_result );

protected:
	//////////////////////////////////////////////////////////////////////////
	// numerical solvers, see MCGridNumerical.cpp
	//////////////////////////////////////////////////////////////////////////
	void svdcmp_ext(float **a, int m, int n, float w[], float **v);
	void qrdcmp_ext(float **a, int n, float *c, float *d, int *sing);
	float pythag_ext(float a, float b);

public:
	//////////////////////////////////////////////////////////////////////////
	// print and debug functions
	//////////////////////////////////////////////////////////////////////////
	void HermiteData_SaveToTxt( char filename[] );
	void PrintMatrix( float **a, int matrix_size );
	void PrintVector( float *w, int vector_size );

protected:
	//////////////////////////////////////////////////////////////////////////
	// small, inline functions
	//////////////////////////////////////////////////////////////////////////
	int HDIndex( int i, int j ) {
		return ( i * m_nHDSideNumber + j );
	}

	MCGridNode & LocateNode( NodeLocator & loc ) {
		return m_vecNodes[ loc.l ][ ( loc.i << ( m_nLevel - loc.l ) ) + loc.j ];
	}

	MCHyperPoint & LocateHyperPoint( HyperPointLocator & loc ) {
		return LocateNode( loc.node ).hpoints[ loc.i ];
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
