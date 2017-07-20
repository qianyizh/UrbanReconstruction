#include "stdafx.h"
#include "MeshBoundary.h"

CMeshBoundary::CMeshBoundary(void)
{
}

CMeshBoundary::~CMeshBoundary(void)
{
}

void CMeshBoundary::Init( CMesh * mesh )
{
	m_pMesh = mesh;
	m_vecVertex.clear();
	m_vecVertex.resize( mesh->m_vecVertex.size() );
	m_vecEdge.clear();
	m_vecFace.clear();
	m_vecFace.resize( mesh->m_vecTriangle.size() );
	for ( int i = 0; i < ( int )mesh->m_vecTriangle.size(); i++ ) {
		m_vecFace[ i ].vi[ 0 ] = mesh->m_vecTriangle[ i ].i[ 0 ];
		m_vecFace[ i ].vi[ 1 ] = mesh->m_vecTriangle[ i ].i[ 1 ];
		m_vecFace[ i ].vi[ 2 ] = mesh->m_vecTriangle[ i ].i[ 2 ];
	}

	// first build up half-edge structure
	CreateHalfEdgeMesh();

	// second find boundaries
	CreateBoundary();

	// third sort boundaries
	SortBoundary();
}

void CMeshBoundary::CreateHalfEdgeMesh()
{
	// input f.vi

	// first pass, solve v.ei, e.vi, e.fi, e.next, f.ei
	// unsolved: e.twin, e.next
	for ( int i = 0; i < ( int )m_vecFace.size(); i++ ) {
		CAuxFace & f = m_vecFace[ i ];
		int base_edge_size = ( int )m_vecEdge.size();
		for ( int j = 0; j < 3; j++ ) {
			int k = (j+1)%3;
			CAuxVertex & v = m_vecVertex[ f.vi[j] ];
			// half-edge j->k
			CAuxEdge e;
			e.vi[0] = f.vi[j];
			e.vi[1] = f.vi[k];
			e.fi = i;
			e.next = k + base_edge_size;
			e.twin = -1;
			f.ei[j] = m_vecEdge.size();
			v.ei.push_back( m_vecEdge.size() );
			m_vecEdge.push_back( e );
		}
	}

	// second pass, solve e.twin
	for ( int i = 0; i < ( int )m_vecVertex.size(); i++ ) {
		CAuxVertex & v = m_vecVertex[ i ];
		for ( int j = 0; j < ( int )v.ei.size(); j++ ) {
			CAuxEdge & e = m_vecEdge[ v.ei[j] ];
			if ( e.twin == -1 ) {
				CAuxVertex & v1 = m_vecVertex[ e.vi[1] ];
				for ( int k = 0; k < ( int )v1.ei.size(); k++ ) {
					CAuxEdge & e1 = m_vecEdge[ v1.ei[k] ];
					if ( e.vi[0] == e1.vi[1] && e.vi[1] == e1.vi[0] ) {
						// find a twin pair
						e.twin = v1.ei[k];
						e1.twin = v.ei[j];
						break;
					}
				}
			}
		}
	}

}

void CMeshBoundary::CreateBoundary()
{
	std::vector< int > boundary_ei;
	std::vector< int > boundary_ei_back_pointer( m_vecEdge.size(), -1 );
	for ( int i = 0; i < ( int )m_vecEdge.size(); i++ ) {
		if ( m_vecEdge[ i ].twin == -1 ) {
			boundary_ei_back_pointer[ i ] = boundary_ei.size();
			boundary_ei.push_back( i );
		}
	}

	std::vector< bool > edge_used( boundary_ei.size(), false );
	int used_num = 0;

	while ( used_num < ( int )boundary_ei.size() ) {
		m_vecBoundary.resize( m_vecBoundary.size() + 1 );
		CAuxBoundary & bdr = m_vecBoundary.back();

		int seed_edge = -1;
		for ( int i = 0; i < ( int )boundary_ei.size(); i++ ) {
			if ( edge_used[ i ] == false ) {
				seed_edge = boundary_ei[ i ];
				break;
			}
		}

		// go one step
		bdr.ei.push_back( seed_edge );
		bdr.vi.push_back( m_vecEdge[ seed_edge ].vi[ 0 ] );
		bdr.height = m_pMesh->m_vecVertex[ m_vecEdge[ seed_edge ].vi[ 0 ] ].v[ 2 ];
		edge_used[ boundary_ei_back_pointer[ seed_edge ] ] = true;
		used_num++;
		int ei = GetNextEdge( seed_edge );

		while ( ei != seed_edge ) {
			bdr.ei.push_back( ei );
			bdr.vi.push_back( m_vecEdge[ ei ].vi[ 0 ] );
			bdr.height += m_pMesh->m_vecVertex[ m_vecEdge[ ei ].vi[ 0 ] ].v[ 2 ];
			edge_used[ boundary_ei_back_pointer[ ei ] ] = true;
			used_num++;
			ei = GetNextEdge( ei );
		}

		bdr.height /= ( double )bdr.ei.size();
	}
}

int CMeshBoundary::GetNextEdge( int ei )
{
	if ( m_vecEdge[ ei ].twin != -1 ) {
		return -1;
	} else {
		int next_ei = m_vecEdge[ ei ].next;
		while ( m_vecEdge[ next_ei ].twin != -1 )
			next_ei = m_vecEdge[ m_vecEdge[ next_ei ].twin ].next;
		return next_ei;
	}
}

void CMeshBoundary::SortBoundary()
{
	m_vecBoundarySeq.clear();
	m_vecBoundarySeq.resize( m_vecBoundary.size() );
	for ( int i = 0; i < ( int )m_vecBoundary.size(); i++ )
		m_vecBoundarySeq[ i ] = i;

	for ( int i = 0; i < ( int )m_vecBoundary.size(); i++ )
		for ( int j = ( int )m_vecBoundary.size() - 1; j > i; j-- ) {
			if ( m_vecBoundary[ m_vecBoundarySeq[ j - 1 ] ].height < m_vecBoundary[ m_vecBoundarySeq[ j ] ].height ) {
				int t = m_vecBoundarySeq[ j - 1 ];
				m_vecBoundarySeq[ j - 1 ] = m_vecBoundarySeq[ j ];
				m_vecBoundarySeq[ j ] = t;
			}
		}
}
