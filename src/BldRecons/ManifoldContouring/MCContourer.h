#pragma once

#include "Geometry\Mesh.h"
#include "MCGrid.h"
#include "MeshBoundary.h"

class CMCContourer
{
public:
	CMCContourer(void);
	~CMCContourer(void);

public:
	friend class CHistGrid;

protected:
	enum EdgeDirection {
		ED_Along_X = 0,
		ED_Along_Y = 1
	};

protected:
	CMCGrid * m_pGrid;

	CMesh m_cMesh;

	bool m_bWallRectangle;

	CMeshBoundary m_cBoundary;

public:
	//////////////////////////////////////////////////////////////////////////
	// entrance functions
	//////////////////////////////////////////////////////////////////////////
	void AssignMCGrid( CMCGrid * grid );
	void Contouring( bool wall_rectangle );
	void SaveToObj( char filename[] ) {
		m_cMesh.SaveToObj( filename );
	}

protected:
	//////////////////////////////////////////////////////////////////////////
	// create vertex functions
	//////////////////////////////////////////////////////////////////////////
	void CreateVertex();
	void CreateVertex_HPoint( CMCGrid::MCHyperPoint & hpoint );

protected:
	//////////////////////////////////////////////////////////////////////////
	// create face functions
	//////////////////////////////////////////////////////////////////////////
	void CreateFace();
	void CreateSurfaceFace();
	void CreateSurfaceFace( CMCGrid::MCGridNode * n[ 2 ][ 2 ] );
	void CreateBoundaryFace();
	void CreateBoundaryFace( CMCGrid::MCGridNode * n[ 2 ], EdgeDirection dir );

protected:
	//////////////////////////////////////////////////////////////////////////
	// small functions
	//////////////////////////////////////////////////////////////////////////
	bool IsDegenrateTriangle( CMesh::MeshTriangle & f ) {
		if ( f.i[0] == -1 || f.i[1] == -1 || f.i[2] == -1 || f.i[0] == f.i[1] || f.i[1] == f.i[2] || f.i[2] == f.i[0] )
			return true;
		else
			return false;
	}

	bool IsDegenrateQuad( CMesh::MeshQuad & q ) {
		if ( q.i[0] == -1 || q.i[1] == -1 || q.i[2] == -1 || q.i[3] == -1 )
			return true;
		else
			return false;
	}

	void PushTriangleSafe( CMesh::MeshTriangle & f ) {
		if ( ! IsDegenrateTriangle( f ) )
			m_cMesh.m_vecTriangle.push_back( f );
	}

	void PushQuadSafe( CMesh::MeshQuad & q ) {
		if ( ! IsDegenrateQuad( q ) )
			m_cMesh.m_vecQuad.push_back( q );
	}

	void PushFaceWithoutGround( int i, int j, int k ) {
		double g = m_pGrid->m_pPointCloud->m_dbGroundZ;
		CMesh::MeshTriangle f( i, j, k );
		if ( ! IsDegenrateTriangle( f ) ) {
			if ( abs( m_cMesh.m_vecVertex[f.i[0]].v[2] - g ) > 1e-5 || abs( m_cMesh.m_vecVertex[f.i[1]].v[2] - g ) > 1e-5 || abs( m_cMesh.m_vecVertex[f.i[2]].v[2] - g ) > 1e-5 ) {
				m_cMesh.m_vecTriangle.push_back( f );
			}
		}
	}

	CMCGrid::MCGridNode & LocateNode( int l, int i, int j ) {
		return m_pGrid->m_vecNodes[ l ][ ( i << ( m_pGrid->m_nLevel - l ) ) + j ];
	}

	int GetVI_HPoint( CMCGrid::MCHyperPoint * start, int k ) {
		int layer[ 2 ] = { k, k };
		CMCGrid::MCHyperPoint * pt = GetCluster_HPoint( start, layer );
		if ( pt->vi == -1 )
			return -1;
		else
			return ( pt->vi + layer[ 0 ] );
	}

	void Vote( CMCGrid::MCHyperPoint * start, CVector3D dir[ 2 ], double vote[ 2 ] ) {
		CMCGrid::MCHyperPoint * pt = start;
		while ( pt->parent != pt && pt->vi == -1 && pt->Is_1HPoint() ) {
			if ( pt->sigma2[ 2 ] < 0.01 ) {						// plane or ridge
				vote[ 0 ] += pt->sigma2[ 1 ] * abs( dir[ 0 ] * pt->v[ 2 ] );
				vote[ 1 ] += pt->sigma2[ 1 ] * abs( dir[ 1 ] * pt->v[ 2 ] );
			}
			pt = pt->parent;
		}
	}

	CMCGrid::MCHyperPoint * GetCluster_HPoint( CMCGrid::MCHyperPoint * start, int layer[ 2 ] ) {
		CMCGrid::MCHyperPoint * pt = start;
		while ( pt->parent != pt && pt->vi == -1 ) {
			layer[ 0 ] = pt->leaf_to_parent[ layer[ 0 ] ];
			layer[ 1 ] = pt->leaf_to_parent[ layer[ 1 ] ];
			pt = pt->parent;
		}
		return pt;
	}
};
