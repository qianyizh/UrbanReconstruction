#pragma once

#include <vector>

#include "Vector3D.h"

class CMesh
{
public:
	CMesh(void);
	~CMesh(void);

public:
	struct MeshVertex {
		CVector3D v;
		MeshVertex( const CVector3D & vv ) : v( vv ) { }
		MeshVertex() {}
	};
	struct MeshTriangle {
		int i[ 3 ];
		MeshTriangle( int i0, int i1, int i2 ) {
			i[ 0 ] = i0;
			i[ 1 ] = i1;
			i[ 2 ] = i2;
		}
		MeshTriangle() {}
	};
	struct MeshQuad {
		int i[ 4 ];
		MeshQuad( int i0, int i1, int i2, int i3 ) {
			i[ 0 ] = i0;
			i[ 1 ] = i1;
			i[ 2 ] = i2;
			i[ 3 ] = i3;
		}
		MeshQuad() {}
	};

public:
	std::vector< MeshVertex > m_vecVertex;
	std::vector< MeshTriangle > m_vecTriangle;
	std::vector< MeshQuad > m_vecQuad;

public:
	void SaveToObj( char filename[] );
};
