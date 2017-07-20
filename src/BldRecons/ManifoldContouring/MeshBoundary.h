#pragma once

#include "Geometry\Mesh.h"

class CMeshBoundary
{
public:
	CMeshBoundary(void);
	~CMeshBoundary(void);

public:
	friend class CMCContourer;
	friend class CHistGrid;

protected:
	class CVertexGroupInfo {
	public:
		int number;
		int index;
		bool fixed;
		CVertexGroupInfo( int ll, int ii ) : number(ll), index(ii), fixed(false) {}
	};

	class CAuxVertex {
	public:
		std::vector< int > ei;
	};

	class CAuxEdge {
	public:
		int vi[2];
		int fi;
		int twin;
		int next;
	};

	class CAuxFace {
	public:
		int vi[3];
		int ei[3];
	};

	class CAuxBoundary {
	public:
		std::vector< int > vi;
		std::vector< int > ei;
		double height;
	};

protected:
	CMesh * m_pMesh;
	std::vector< CVertexGroupInfo > m_vecGroupInfo;
	std::vector< CAuxVertex > m_vecVertex;
	std::vector< CAuxEdge > m_vecEdge;
	std::vector< CAuxFace > m_vecFace;
	std::vector< CAuxBoundary > m_vecBoundary;
	std::vector< int > m_vecBoundarySeq;

public:
	void Init( CMesh * mesh );

protected:
	void CreateHalfEdgeMesh();
	void CreateBoundary();
	int GetNextEdge( int ei );
	void SortBoundary();
};
