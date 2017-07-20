#include "StdAfx.h"
#include "Outline.h"
#include "Miscs\NumericalSolver.h"

#include <list>

COutline::COutline(void)
{
}

COutline::~COutline(void)
{
}

void COutline::Add( int v0, int v1 )
{
	m_vecVertex[ v0 ].next = v1;
	m_vecVertex[ v1 ].prev = v0;
}

void COutline::ComputeDirection( int nNeighbor )
{
	double e[2][2], v[2][2], d[2];

	std::vector< int > vertex_index( nNeighbor * 2 );

	for ( int i = 0; i < ( int )m_vecVertex.size(); i++ ) {
		COutlineVertex & vertex = m_vecVertex[ i ];
		vertex.d = CVector3D( 0.0, 0.0, 0.0 );

		// prev
		int vi = i;
		for ( int k = 0; k < nNeighbor; k++ ) {
			vi = m_vecVertex[ vi ].prev;
			vertex_index[ k ] = vi;
		}

		// next
		vi = i;
		for ( int k = 0; k < nNeighbor; k++ ) {
			vi = m_vecVertex[ vi ].next;
			vertex_index[ k + nNeighbor ] = vi;
		}

		// covariance
		e[0][0] = e[0][1] = e[1][0] = e[1][1] = 0.0;

		for ( int k = 0; k < nNeighbor * 2; k++ ) {
			CVector3D diff = vertex.v - m_vecVertex[ vertex_index[ k ] ].v;
			e[ 0 ][ 0 ] += diff[ 0 ] * diff[ 0 ];
			e[ 0 ][ 1 ] += diff[ 0 ] * diff[ 1 ];
			e[ 1 ][ 1 ] += diff[ 1 ] * diff[ 1 ];
		}

		e[ 0 ][ 0 ] /= ( nNeighbor * 2.0 );
		e[ 0 ][ 1 ] /= ( nNeighbor * 2.0 );
		e[ 1 ][ 1 ] /= ( nNeighbor * 2.0 );
		e[ 1 ][ 0 ] = e[ 0 ][ 1 ];

		CNumericalSolver::SolveEigenVectors2( e, v, d );
		vertex.d = CVector3D( v[0][0], v[1][0], 0.0	);
	}
}

void COutline::Save( FILE * pFile )
{
	int num = m_vecVertex.size();
	fwrite( &  m_cPlane, sizeof( CPlane ), 1, pFile );
	fwrite( & num, sizeof( int ), 1, pFile );

	for ( int i = 0; i < num; i++ ) {
		fwrite( & m_vecVertex[ i ], sizeof( COutlineVertex ), 1, pFile );
	}
}

void COutline::Load( FILE * pFile )
{
	int num;
	fread( & m_cPlane, sizeof( CPlane ), 1, pFile );
	fread( & num, sizeof( int ), 1, pFile );
	m_vecVertex.clear();
	m_vecVertex.resize( num );

	for ( int i = 0; i < num; i++ ) {
		fread( & m_vecVertex[ i ], sizeof( COutlineVertex ), 1, pFile );
	}
}

//////////////////////////////////////////////////////////////////////////
// turn a polygon into a triangle patch, interesting part...
//////////////////////////////////////////////////////////////////////////

void COutline::GenerateRoofTriangle()
{
	m_vecRoofTriangle.clear();
	int num = ( int )m_vecVertex.size();
	std::vector< bool > touched( num, false );
	int touched_num = 0;
	std::list< COutlineVertex > vertex_loop;

	while ( touched_num < num ) {
		int idx;
		for ( idx = 0; idx < num; idx++ ) {
			if ( touched[ idx ] == false )
				break;
		}

		double angle_sum = 0.0;
		const double pi = 3.1415926535897932384626;
		vertex_loop.clear();
		while ( touched[ idx ] == false ) {
			touched[ idx ] = true;
			vertex_loop.push_back( m_vecVertex[ idx ] );
			vertex_loop.back().prev = vertex_loop.back().next = idx;
			idx = m_vecVertex[ idx ].next;
		}
		touched_num += ( int )vertex_loop.size();

		bool changed = true;
		std::list< COutlineVertex >::iterator it, itprev, itnext;

		while ( changed && vertex_loop.size() >= 3 ) {

			changed = false;

			for ( it = vertex_loop.begin(); it != vertex_loop.end(); it++ ) {

				if ( it == vertex_loop.begin() ) {
					itprev = vertex_loop.end();
				} else {
					itprev = it;
				}
				itprev--;

				itnext = it;
				itnext++;
				if ( itnext == vertex_loop.end() ) {
					itnext = vertex_loop.begin();
				}

				// check triangle itprev, it, itnext
				CVector3D & v = it->v;
				CVector3D & vprev = itprev->v;
				CVector3D & vnext = itnext->v;

				CVector3D vvprev = vprev - v;
				CVector3D vvnext = vnext - v;

				if ( vvprev[ 0 ] * vvprev[ 0 ] + vvprev[ 1 ] * vvprev[ 1 ] < DOUBLE_TOLERANCE ) {
					m_vecRoofTriangle.push_back( COutlineTriangle( itprev->prev, it->prev, itnext->prev ) );
					vertex_loop.erase( it );
					changed = true;
					break;
				}
				if ( vvnext[ 0 ] * vvnext[ 0 ] + vvnext[ 1 ] * vvnext[ 1 ] < DOUBLE_TOLERANCE ) {
					m_vecRoofTriangle.push_back( COutlineTriangle( itprev->prev, it->prev, itnext->prev ) );
					vertex_loop.erase( it );
					changed = true;
					break;
				}
				if ( vvprev[ 0 ] * vvnext[ 1 ] - vvnext[ 0 ] * vvprev[ 1 ] > 0 )
					continue;

				bool pass_test = true;
				double testx[ 3 ] = { vprev[ 0 ], v[ 0 ], vnext[ 0 ] };
				double testy[ 3 ] = { vprev[ 1 ], v[ 1 ], vnext[ 1 ] };
				for ( std::list< COutlineVertex >::iterator itt = vertex_loop.begin(); itt != vertex_loop.end(); itt++ ) {
					if ( itt->prev != itprev->prev && itt->prev != it->prev && itt->prev != itnext->prev && InPolygon( itt->v[0], itt->v[1], 3, testx, testy ) ) {
						pass_test = false;
						break;
					}
				}

				if ( pass_test ) {
					m_vecRoofTriangle.push_back( COutlineTriangle( itprev->prev, it->prev, itnext->prev ) );
					vertex_loop.erase( it );
					changed = true;
					break;
				}
			}
		}
	}
}

bool COutline::InPolygon( double x, double y, int n, const double * px, const double * py )
{
	int i, j = n - 1;
	bool test_result = false;
	for( i = 0; i < n; i++)
	{
		if ( py[i] < y && py[j] >= y || py[j] < y && py[i] >= y )
			if ( px[i] + ( y - py[i] ) / ( py[j] - py[i] ) * ( px[j] - px[i] ) < x )
				test_result = !test_result;
		j = i;
	}
	return test_result;
}

void COutline::WriteOBJVertex( FILE *pFile, double ground_z )
{
	for ( int i = 0; i < ( int )m_vecVertex.size(); i++ ) {
		COutlineVertex & v = m_vecVertex[ i ];
		fprintf_s( pFile, "v %.8f %.8f %.8f\n", v.v[ 0 ], v.v[ 1 ], v.v[ 2 ] );
	}
	for ( int i = 0; i < ( int )m_vecVertex.size(); i++ ) {
		COutlineVertex & v = m_vecVertex[ i ];
		fprintf_s( pFile, "v %.8f %.8f %.8f\n", v.v[ 0 ], v.v[ 1 ], ground_z );
	}
}

void COutline::WriteOBJFace( FILE * pFile, int offset, bool wall_rectangle )
{
	int num = ( int )m_vecVertex.size();

	for ( int i = 0; i < ( int )m_vecRoofTriangle.size(); i++ ) {
		COutlineTriangle & t = m_vecRoofTriangle[ i ];
		fprintf_s( pFile, "f %d %d %d\n", t.vi[ 0 ] + offset, t.vi[ 1 ] + offset, t.vi[ 2 ] + offset );
	}

	//std::vector< bool > touched( num, false );
	//int touched_num = 0;

	//while ( touched_num < num ) {
	//	int idx;
	//	for ( idx = 0; idx < num; idx++ ) {
	//		if ( touched[ idx ] == false )
	//			break;
	//	}

	//	fprintf_s( pFile, "f" );
	//	while ( touched[ idx ] == false ) {
	//		touched[ idx ] = true;
	//		fprintf_s( pFile, " %d", idx + offset );
	//		touched_num++;
	//		idx = m_vecVertex[ idx ].next;
	//	}
	//	fprintf_s( pFile, "\n" );

	//}

	for ( int i = 0; i < ( int )m_vecVertex.size(); i++ ) {
		COutlineVertex & v = m_vecVertex[ i ];
		if ( wall_rectangle ) {
			fprintf_s( pFile, "f %d %d %d %d\n", i + offset, v.prev + offset, v.prev + num + offset, i + num + offset );
		} else {
			fprintf_s( pFile, "f %d %d %d\n", i + offset, v.prev + offset, i + num + offset );
			fprintf_s( pFile, "f %d %d %d\n", v.prev + offset, v.prev + num + offset, i + num + offset );
		}
	}
}