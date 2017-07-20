#pragma once

#include "DCGrid.h"
#include "common\Mesh.h"

class CDCContourer
{
public:
	CDCContourer(void);
	~CDCContourer(void);

protected:
	enum EdgeDirection {
		ED_Along_X = 0,
		ED_Along_Y = 1
	};

protected:
	CDCGrid * m_pGrid;

	CMesh m_cMesh;

public:
	//////////////////////////////////////////////////////////////////////////
	// entrance functions
	//////////////////////////////////////////////////////////////////////////
	void AssignDCGrid( CDCGrid * grid );
	void Contouring();
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

	bool IsDegenrateFace( CMesh::MeshTriangle & f ) {
		double g = m_pGrid->m_pPointCloud->m_dbGroundZ;
		if ( f.i[0] == -1 || f.i[1] == -1 || f.i[2] == -1 || f.i[0] == f.i[1] || f.i[1] == f.i[2] || f.i[2] == f.i[0] )
			return true;
		else if ( abs( m_cMesh.m_vecVertex[f.i[0]].v[2] - g ) < 1e-5 || abs( m_cMesh.m_vecVertex[f.i[1]].v[2] - g ) < 1e-5 || abs( m_cMesh.m_vecVertex[f.i[2]].v[2] - g ) < 1e-5 )
			return true;
		else
			return false;
	}

	void PushFace( CMesh::MeshTriangle & f, int v[], int i, int j, int k ) {
		f.i[0] = v[i];
		f.i[1] = v[j];
		f.i[2] = v[k];
		if ( ! IsDegenrateFace( f ) )
			m_cMesh.m_vecTriangle.push_back( f );
	}

	void PushTriangleSafe( CMesh::MeshTriangle & f ) {
		if ( ! ( f.i[0] == -1 || f.i[1] == -1 || f.i[2] == -1 || f.i[0] == f.i[1] || f.i[1] == f.i[2] || f.i[2] == f.i[0] ) )
			m_cMesh.m_vecTriangle.push_back( f );
	}
};
