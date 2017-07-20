#pragma once

#include "Geometry\Outline.h"

struct AuxLine {
	CLine line;
	std::vector< int > vi;
	bool snapped;

	AuxLine() : snapped( false ) {}
};

class CAuxOutline
{
public:
	CAuxOutline(void);
	~CAuxOutline(void);

public:
	COutline * m_pOutline;
	std::vector< AuxLine > m_vecLine;
	std::vector< int > m_vecLineIndex;

public:
	void RegisterOutline( COutline * outline );
	void ProduceOutline( COutline & outline );

private:
	bool CheckDistance2D( CVector3D & v, CVector3D & v0, double corner_tolerance );
	bool CheckAngle2D( CVector3D & v, int l0, int l1, double cos_angle_tolerance );

private:
	//////////////////////////////////////////////////////////////////////////
	// Basic querying structure for CAuxOutline
	//
	// line -> vertex : m_vecLine::vi
	// vertex -> line : m_vecLineIndex
	// line -> first vertex : m_vecLine::vi[0]
	// line -> last vertex : m_vecLine::vi[n-1]
	// line -> prev line : m_vecLineIndex[ m_vecLine::vi[0]::prev ]
	// line -> next line : m_vecLineIndex[ m_vecLine::vi[n-1]::next ]
	//////////////////////////////////////////////////////////////////////////
	int Line_VertexIndex( int l, int k ) { return m_vecLine[ l ].vi[ k ]; }
	int Vertex_LineIndex( int v ) { return m_vecLineIndex[ v ]; }

	int Line_FirstVertexIndex( int l ) {
		return m_vecLine[ l ].vi[ 0 ];
	}
	int Line_LastVertexIndex( int l ) {
		return m_vecLine[ l ].vi.back();
	}

	int Line_PrevLineIndex( int l ) {
		int first_v = Line_FirstVertexIndex( l );
		int prev_v = m_pOutline->m_vecVertex[ first_v ].prev;
		return m_vecLineIndex[ prev_v ];
	}
	int Line_NextLineIndex( int l ) {
		int last_v = Line_LastVertexIndex( l );
		int next_v = m_pOutline->m_vecVertex[ last_v ].next;
		return m_vecLineIndex[ next_v ];
	}
};
