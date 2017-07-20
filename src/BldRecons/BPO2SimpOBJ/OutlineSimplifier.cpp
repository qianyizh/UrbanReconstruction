#include "StdAfx.h"
#include <list>
#include "Miscs\NumericalSolver.h"
#include "OutlineSimplifier.h"

COutlineSimplifier::COutlineSimplifier(void)
{
}

COutlineSimplifier::~COutlineSimplifier(void)
{
}

int COutlineSimplifier::Simplify( COutline & outline, CAuxOutline & aux, std::vector< int > & vlist, double tolerance, CParamManager::LineFittingMode fitting_mode )
//////////////////////////////////////////////////////////////////////////
// Note: in this function, all the vertex index vector represents vertices as [s, s + 1, ..., e - 1 ], but EXCLUDING e
//////////////////////////////////////////////////////////////////////////
{
	std::vector< LineSearchStructure > ls( vlist.size() * vlist.size() );

	for ( int i = 0; i < ( int )vlist.size(); i++ )
		for ( int j = i + 1; j < ( int )vlist.size(); j++ ) {
			if ( fitting_mode == CParamManager::LFM_DirectConnect )
				ls[ i * vlist.size() + j ].line = LineFitting_DirectConnect( outline, vlist, i, j );
			else
				ls[ i * vlist.size() + j ].line = LineFitting_LeastSquare( outline, vlist, i, j );

			ls[ i * vlist.size() + j ].s = i;
			ls[ i * vlist.size() + j ].e = j;

			ls[ i * vlist.size() + j ].valid = CheckTolerance( outline, vlist, i, j, ls[ i * vlist.size() + j ].line, tolerance );
		}

	// breadth-first search
	std::vector< int > parents( vlist.size(), -1 );
	std::vector< int > queue;
	queue.push_back( 0 );
	int queue_it = 0;

	while ( queue_it < ( int )queue.size() ) {
		int node = queue[ queue_it ];
		if ( node == vlist.size() - 1 ) {
			break;
		}

		// check all the node from node to i > node
		for ( int i = ( int )vlist.size() - 1; i >= node + 1; i-- ) {
			// no loops, thus no need to check loop
			// directly check if there is an edge connecting
			if ( parents[ i ] == -1 && ls[ node * vlist.size() + i ].valid ) {
				parents[ i ] = node;
				queue.push_back( i );
			}
		}

		queue_it++;
	}

	// must have a solution, for, [k, k+1] always passes the tolerance test
	int cnt = 0;
	int node = queue[ queue_it ];

	while ( node != queue[ 0 ] ) {
		cnt++;
		int pnode = parents[ node ];
		aux.m_vecLine.resize( aux.m_vecLine.size() + 1 );
		AuxLine & aux_line = aux.m_vecLine.back();

		aux_line.line = ls[ pnode * vlist.size() + node ].line;
		aux_line.vi.clear();
		aux_line.vi.resize( node - pnode );

		for ( int i = 0; i < node - pnode; i++ ) {
			aux_line.vi[ i ] = vlist[ pnode + i ];
			aux.m_vecLineIndex[ vlist[ pnode + i ] ] = aux.m_vecLine.size() - 1;
		}

		node = pnode;
	}

	return cnt;
}

CLine COutlineSimplifier::LineFitting_DirectConnect( COutline & outline, std::vector< int > & vlist, int s, int e )
{
	CLine line;
	CVector3D & v0 = outline.m_vecVertex[ vlist[ s ] ].v;
	CVector3D & v1 = outline.m_vecVertex[ vlist[ e ] ].v;
	line.p = v0;
	line.d = v1 - v0;
	line.d[2] = 0.0;

	// handle the degenerated case
	if ( line.d.normalize() < DOUBLE_TOLERANCE )
		line.d = CVector3D( 1.0, 0.0, 0.0 );

	return line;
}

CLine COutlineSimplifier::LineFitting_LeastSquare( COutline & outline, std::vector< int > & vlist, int s, int e )
{
	CLine line;
	line.p = CVector3D( 0.0, 0.0, 0.0 );
	double m[2][2], v[2][2], d[2];
	m[0][0] = m[0][1] = m[1][0] = m[1][1] = 0.0;

	for ( int i = s; i <= e; i++ ) {
		line.p += outline.m_vecVertex[ vlist[ i ] ].v;
	}
	line.p /= ( double )( e - s + 1 );

	for ( int i = s; i <= e; i++ ) {
		CVector3D diff = line.p - outline.m_vecVertex[ vlist[ i ] ].v;
		m[ 0 ][ 0 ] += diff[ 0 ] * diff[ 0 ];
		m[ 0 ][ 1 ] += diff[ 0 ] * diff[ 1 ];
		m[ 1 ][ 1 ] += diff[ 1 ] * diff[ 1 ];
	}

	m[ 0 ][ 0 ] /= ( double )( e - s + 1 );
	m[ 0 ][ 1 ] /= ( double )( e - s + 1 );
	m[ 1 ][ 1 ] /= ( double )( e - s + 1 );
	m[ 1 ][ 0 ] = m[ 0 ][ 1 ];

	CNumericalSolver::SolveEigenVectors2( m, v, d );
	line.d = CVector3D( v[0][0], v[1][0], 0.0	);

	// handle the degenerated case
	// in CNumericalSolver, if input is degenerated, output will be a random vector
	// no additional operation needed

	return line;
}

bool COutlineSimplifier::CheckTolerance( COutline & outline, std::vector< int > & vlist, int s, int e, CLine & line, double tolerance )
{
	if ( e == s + 1 )
		return true;

	for ( int i = s; i <= e; i++ ) {
		if ( line.dis( outline.m_vecVertex[ vlist[ i ] ].v ) > tolerance )
			return false;
	}
	return true;
}
