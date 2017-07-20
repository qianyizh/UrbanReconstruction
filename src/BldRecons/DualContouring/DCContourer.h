#pragma once

#include "MeshBoundary.h"
#include "DCGrid.h"

class CDCContourer
{
public:
	CDCContourer(void);
	~CDCContourer(void);

public:
	friend class CHistGrid;

protected:
	enum EdgeDirection {
		ED_Along_X = 0,
		ED_Along_Y = 1
	};

protected:
	CDCGrid * m_pGrid;

	CMesh m_cMesh;
	CMeshBoundary m_cBoundary;

	bool m_bWallRectangle;
	bool m_bAntiNonManifold;

public:
	//////////////////////////////////////////////////////////////////////////
	// entrance functions
	//////////////////////////////////////////////////////////////////////////
	void AssignDCGrid( CDCGrid * grid );
	void Contouring( bool wall_rectangle, bool anti_non_manifold );
	void SaveToObj( char filename[] ) {
		m_cMesh.SaveToObj( filename );
	}

protected:
	//////////////////////////////////////////////////////////////////////////
	// pre-processing functions
	//////////////////////////////////////////////////////////////////////////
	void PreProcessing( CDCGrid::NodeLocator & loc );

protected:
	//////////////////////////////////////////////////////////////////////////
	// create vertex functions
	//////////////////////////////////////////////////////////////////////////
	void CreateVertex( CDCGrid::NodeLocator & loc );
	int AntiManifoldSplitDirection( CDCGrid::DCGridNode & node );

protected:
	//////////////////////////////////////////////////////////////////////////
	// create face functions
	//////////////////////////////////////////////////////////////////////////
	void CreateFacePerNode( CDCGrid::NodeLocator & loc );
	void CreateFacePerEdge( CDCGrid::NodeLocator loc[ 2 ], EdgeDirection dir );
	void CreateFacePerPoint( CDCGrid::NodeLocator loc[ 2 ][ 2 ] );

protected:
	//////////////////////////////////////////////////////////////////////////
	// small functions
	//////////////////////////////////////////////////////////////////////////
	CDCGrid::NodeLocator Root() { return CDCGrid::NodeLocator( m_pGrid->m_nLevel, 0, 0 ); }

	int RootCluster_NodeCornerIndex( CDCGrid::NodeLocator & loc, int i, int j ) {
		int ci = ( loc.i + i ) << loc.l;
		int cj = ( loc.j + j ) << loc.l;
		return m_pGrid->RootCluster_Index( ci, cj );
	}

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

	void PushFaceWithoutGround( CMesh::MeshTriangle & f, int v[], int i, int j, int k ) {
		double g = m_pGrid->m_pPointCloud->m_dbGroundZ;
		f.i[0] = v[i];
		f.i[1] = v[j];
		f.i[2] = v[k];
		if ( ! IsDegenrateTriangle( f ) ) {
			if ( abs( m_cMesh.m_vecVertex[f.i[0]].v[2] - g ) > 1e-5 || abs( m_cMesh.m_vecVertex[f.i[1]].v[2] - g ) > 1e-5 || abs( m_cMesh.m_vecVertex[f.i[2]].v[2] - g ) > 1e-5 ) {
				m_cMesh.m_vecTriangle.push_back( f );
			}
		}
	}

	void PushTriangleSafe( CMesh::MeshTriangle & f ) {
		if ( ! IsDegenrateTriangle( f ) )
			m_cMesh.m_vecTriangle.push_back( f );
	}

	void PushQuadSafe( CMesh::MeshQuad & q ) {
		if ( ! IsDegenrateQuad( q ) )
			m_cMesh.m_vecQuad.push_back( q );
	}

	static const double AntiManifoldShift;
};
