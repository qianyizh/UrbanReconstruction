#include "StdAfx.h"
#include "BPGrid.h"
#include "ParamManager.h"
#include "Miscs\NumericalSolver.h"
#include "Miscs\BPOWriter.h"

CBPGrid::CBPGrid( CBPCloud * pCloud ) :
	m_pCloud( pCloud )
{
}

CBPGrid::~CBPGrid(void)
{
}

void CBPGrid::BuildGridIndex()
{
	m_vecGridIndex.clear();
	m_vecGridIndex.resize( m_pCloud->m_nUnitNumber[ 0 ] * m_pCloud->m_nUnitNumber[ 1 ] );

	for ( int i = 0; i < ( int )m_pCloud->m_vecPoint.size(); i++ ) {
		m_vecGridIndex[ Index( m_pCloud->m_vecPoint[ i ].v ) ].push_back( & ( m_pCloud->m_vecPoint[ i ] ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// plane fitting functions
//////////////////////////////////////////////////////////////////////////

void CBPGrid::PlaneFitting_RegionGrow()
{
	PlaneFittingParam & param = CParamManager::GetParamManager()->m_cFittingParam;
	PatchPointDataVector data_nc;
	PatchPointDataVector data_dc;
	m_vecPlane.clear();

	for ( int i = 0; i < ( int )m_pCloud->m_vecPoint.size(); i++ ) {

		PatchPointData & seed = m_pCloud->m_vecPoint[ i ];

		if ( seed.patch.base != -1 || seed.flatness > param.m_dbSeedMaxFlat )
			continue;

		CPlane plane;
		bool accept = true;

		plane.pos = seed.v;
		plane.norm = seed.n;
		plane.index = ( int )m_vecPlane.size();

		for ( int k = 0; k < param.m_nPlaneFitting_RegionGrow_NormalCheck_IterationStep && accept; k++ ) {
			PlaneFitting_RegionGrow_NormalCheck( plane, data_nc, seed );
			RollBack( data_nc );
			if ( accept = ( ( int )data_nc.size() >= param.m_nAcceptPointNumber ) )
				plane = FitPlane( data_nc );
		}

		if ( accept ) {
			PlaneFitting_RegionGrow_DoubleCheck( plane, data_nc, data_dc, seed );

			if ( ( int )data_nc.size() + ( int )data_dc.size() >= param.m_nAcceptPointNumber ) {
				m_vecPlane.push_back( plane );
			} else {
				RollBack( data_nc );
				RollBack( data_dc );
			}
		}
	}
}

void CBPGrid::PlaneFitting_RegionGrow_NormalCheck( CPlane & plane, PatchPointDataVector & data_nc, PatchPointData & seed )
{

	data_nc.clear();
	data_nc.push_back( & seed );

	double distance2 = m_pCloud->m_dbGridLength * m_pCloud->m_dbGridLength;
	PlaneFittingParam & param = CParamManager::GetParamManager()->m_cFittingParam;

	for ( int i = 0; i < ( int )data_nc.size(); i++ ) {

		PatchPointData * current = data_nc[ i ];
		int ix = IndexX( current->v );
		int iy = IndexY( current->v );

		for ( int x = ix - 1; x <= ix + 1; x++ )
			for ( int y = iy - 1; y <= iy + 1; y++ ) {

				int idx = Index( x, y );

				for ( int i = 0; i < ( int )m_vecGridIndex[ idx ].size(); i++ ) {
					
					PatchPointData * pointer = m_vecGridIndex[ idx ][ i ];

					if ( pointer->patch.base == -1 ) {

						CVector3D diff = current->v - pointer->v;
						if ( diff.length2() < distance2 && pointer->n * plane.norm > param.m_dbCosAngleMaxDifference ) {

							data_nc.push_back( pointer );
							pointer->patch.base = plane.index;

						}

					}

				}

			}

	}

}

void CBPGrid::PlaneFitting_RegionGrow_DoubleCheck( CPlane & plane, PatchPointDataVector & data_nc, PatchPointDataVector & data_dc, PatchPointData & seed )
{

	data_nc.clear();
	data_nc.push_back( & seed );

	data_dc.clear();

	double distance2 = m_pCloud->m_dbGridLength * m_pCloud->m_dbGridLength;
	PlaneFittingParam & param = CParamManager::GetParamManager()->m_cFittingParam;

	for ( int i = 0; i < ( int )data_nc.size(); i++ ) {

		PatchPointData * current = data_nc[ i ];
		int ix = IndexX( current->v );
		int iy = IndexY( current->v );

		for ( int x = ix - 1; x <= ix + 1; x++ )
			for ( int y = iy - 1; y <= iy + 1; y++ ) {

				int idx = Index( x, y );

				for ( int i = 0; i < ( int )m_vecGridIndex[ idx ].size(); i++ ) {

					PatchPointData * pointer = m_vecGridIndex[ idx ][ i ];

					if ( pointer->patch.base == -1 ) {

						CVector3D diff = current->v - pointer->v;
						if ( diff.length2() < distance2 ) {

							if ( pointer->n * plane.norm > param.m_dbCosAngleMaxDifference ) {
								data_nc.push_back( pointer );
								pointer->patch.base = plane.index;
							} else if ( abs( plane.dis( pointer->v ) ) < param.m_dbMaxDistance ) {
								data_dc.push_back( pointer );
								pointer->patch.base = plane.index;
							}

						}

					}

				}

			}

	}

}

CPlane CBPGrid::FitPlane( PatchPointDataVector & data )
{
	CPlane plane;
	plane.pos = CVector3D( 0.0, 0.0, 0.0 );

	for ( int i = 0; i < ( int )data.size(); i++ ) {
		plane.pos += ( *( data[ i ] ) ).v;
	}
	plane.pos /= ( double )data.size();

	double e[3][3];
	double v[3][3];
	double d[3];

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ )
			e[i][j] = 0.0;

	for ( int i = 0; i < ( int )data.size(); i++ ) {
		CVector3D diff = ( * ( data[ i ] ) ).v - plane.pos;
		e[ 0 ][ 0 ] += diff[ 0 ] * diff[ 0 ];
		e[ 0 ][ 1 ] += diff[ 0 ] * diff[ 1 ];
		e[ 0 ][ 2 ] += diff[ 0 ] * diff[ 2 ];
		e[ 1 ][ 1 ] += diff[ 1 ] * diff[ 1 ];
		e[ 1 ][ 2 ] += diff[ 1 ] * diff[ 2 ];
		e[ 2 ][ 2 ] += diff[ 2 ] * diff[ 2 ];
	}

	e[ 0 ][ 0 ] /= ( double )data.size();
	e[ 0 ][ 1 ] /= ( double )data.size();
	e[ 0 ][ 2 ] /= ( double )data.size();
	e[ 1 ][ 1 ] /= ( double )data.size();
	e[ 1 ][ 2 ] /= ( double )data.size();
	e[ 2 ][ 2 ] /= ( double )data.size();
	e[ 1 ][ 0 ] = e[ 0 ][ 1 ];
	e[ 2 ][ 0 ] = e[ 0 ][ 2 ];
	e[ 2 ][ 1 ] = e[ 1 ][ 2 ];

	CNumericalSolver::SolveEigenVectors3( e, v, d );

	if ( v[2][2] >= 0.0 ) {
		plane.norm = CVector3D( v[0][2], v[1][2], v[2][2] );
	} else {
		plane.norm = CVector3D( -v[0][2], -v[1][2], -v[2][2] );
	}

	plane.index = ( int )m_vecPlane.size();

	return plane;
}

void CBPGrid::RollBack( PatchPointDataVector & data )
{
	for ( int i = 0; i < ( int )data.size(); i++ ) {
		( * data[ i ] ).patch.base = -1;
	}
}

//////////////////////////////////////////////////////////////////////////
// generate outline functions
//////////////////////////////////////////////////////////////////////////

void CBPGrid::GenerateOutline()
{
	InitOutline();

	for ( int i = 0; i < ( int )m_vecPlane.size(); i++ ) {
		GenerateSingleOutline( i );
		m_vecOutline[ i ].ComputeDirection( CParamManager::GetParamManager()->m_nOutlineNeighbor );
	}
}

void CBPGrid::InitOutline()
{
	m_vecOutline.clear();
	m_vecOutline.resize( m_vecPlane.size() );

	for ( int i = 0; i < ( int )m_vecOutline.size(); i++ ) {
		m_vecOutline[ i ].m_cPlane = m_vecPlane[ i ];
	}

	m_vecGridPatch.clear();
	m_vecGridPatch.resize( m_pCloud->m_nUnitNumber[ 0 ] * m_pCloud->m_nUnitNumber[ 1 ] );

	for ( int i = 0; i < ( int )m_pCloud->m_vecPoint.size(); i++ ) {
		if ( m_pCloud->m_vecPoint[ i ].patch.base != -1 ) {
			PushPatchVector( Index( m_pCloud->m_vecPoint[ i ].v ), ( int )m_pCloud->m_vecPoint[ i ].patch.base );
		}
	}
}

void CBPGrid::GenerateSingleOutline( int plane_index )
{
	COutline & outline = m_vecOutline[ plane_index ];

	outline.m_vecVertex.clear();
	std::vector< int > vertex_index( m_pCloud->m_nUnitNumber[ 0 ] * m_pCloud->m_nUnitNumber[ 1 ] * 2, -1 );				// 0 - diff in x; 1 - diff in y

	// first pass, generate vertex

	for ( int x = 0; x < m_pCloud->m_nUnitNumber[ 0 ] - 1; x++ )
		for ( int y = 1; y < m_pCloud->m_nUnitNumber[ 1 ] - 1; y++ ) {
			int ivi = Index( x, y ) * 2 + 0;
			int ii[ 2 ] = { Index( x, y ), Index( x + 1, y ) };
			bool has_patch[ 2 ] = { CheckPatchVector( ii[ 0 ], plane_index ), CheckPatchVector( ii[ 1 ], plane_index ) };

			if ( has_patch[ 0 ] && !has_patch[ 1 ] ) {
				double minmax = -1e300;
				PatchPointData * pointer = NULL;
				PatchPointDataVector & data = m_vecGridIndex[ ii[ 0 ] ];
				for ( int i = 0; i < ( int )data.size(); i++ ) {
					if ( data[ i ]->patch.base == plane_index && data[ i ]->v[ 0 ] > minmax ) {
						pointer = data[ i ];
						minmax = data[ i ]->v[ 0 ];
					}
				}
				vertex_index[ ivi ] = ( int )outline.m_vecVertex.size();
				outline.m_vecVertex.resize( outline.m_vecVertex.size() + 1 );
				outline.m_vecVertex.back().v = m_vecPlane[ plane_index ].project( pointer->v );
			}

			if ( !has_patch[ 0 ] && has_patch[ 1 ] ) {
				double minmax = 1e300;
				PatchPointData * pointer = NULL;
				PatchPointDataVector & data = m_vecGridIndex[ ii[ 1 ] ];
				for ( int i = 0; i < ( int )data.size(); i++ ) {
					if ( data[ i ]->patch.base == plane_index && data[ i ]->v[ 0 ] < minmax ) {
						pointer = data[ i ];
						minmax = data[ i ]->v[ 0 ];
					}
				}
				vertex_index[ ivi ] = ( int )outline.m_vecVertex.size();
				outline.m_vecVertex.resize( outline.m_vecVertex.size() + 1 );
				outline.m_vecVertex.back().v = m_vecPlane[ plane_index ].project( pointer->v );
			}
		}

	for ( int y = 0; y < m_pCloud->m_nUnitNumber[ 1 ] - 1; y++ )
		for ( int x = 1; x < m_pCloud->m_nUnitNumber[ 0 ] - 1; x++ ) {
			int ivi = Index( x, y ) * 2 + 1;
			int ii[ 2 ] = { Index( x, y ), Index( x, y + 1 ) };
			bool has_patch[ 2 ] = { CheckPatchVector( ii[ 0 ], plane_index ), CheckPatchVector( ii[ 1 ], plane_index ) };

			if ( has_patch[ 0 ] && !has_patch[ 1 ] ) {
				double minmax = -1e300;
				PatchPointData * pointer = NULL;
				PatchPointDataVector & data = m_vecGridIndex[ ii[ 0 ] ];
				for ( int i = 0; i < ( int )data.size(); i++ ) {
					if ( data[ i ]->patch.base == plane_index && data[ i ]->v[ 1 ] > minmax ) {
						pointer = data[ i ];
						minmax = data[ i ]->v[ 1 ];
					}
				}
				vertex_index[ ivi ] = ( int )outline.m_vecVertex.size();
				outline.m_vecVertex.resize( outline.m_vecVertex.size() + 1 );
				outline.m_vecVertex.back().v = m_vecPlane[ plane_index ].project( pointer->v );
			}

			if ( !has_patch[ 0 ] && has_patch[ 1 ] ) {
				double minmax = 1e300;
				PatchPointData * pointer = NULL;
				PatchPointDataVector & data = m_vecGridIndex[ ii[ 1 ] ];
				for ( int i = 0; i < ( int )data.size(); i++ ) {
					if ( data[ i ]->patch.base == plane_index && data[ i ]->v[ 1 ] < minmax ) {
						pointer = data[ i ];
						minmax = data[ i ]->v[ 1 ];
					}
				}
				vertex_index[ ivi ] = ( int )outline.m_vecVertex.size();
				outline.m_vecVertex.resize( outline.m_vecVertex.size() + 1 );
				outline.m_vecVertex.back().v = m_vecPlane[ plane_index ].project( pointer->v );
			}
		}

	// second pass, generate line
	for ( int x = 0; x < m_pCloud->m_nUnitNumber[ 0 ] - 1; x++ ) {
		for ( int y = 0; y < m_pCloud->m_nUnitNumber[ 1 ] - 1; y++ ) {
			int ii[ 4 ] = { Index( x, y ), Index( x + 1, y ), Index( x + 1, y + 1 ), Index( x, y + 1 ) };
			int ivi[ 4 ] = { Index( x, y ) * 2 + 0, Index( x + 1, y ) * 2 + 1, Index( x, y + 1 ) * 2 + 0, Index( x, y ) * 2 + 1 }; 
			if ( vertex_index[ ivi[ 0 ] ] == -1 && vertex_index[ ivi[ 1 ] ] == -1 && vertex_index[ ivi[ 2 ] ] == -1 && vertex_index[ ivi[ 3 ] ] == -1 )
				continue;
			bool black[ 4 ] = { CheckPatchVector( ii[ 0 ], plane_index ), CheckPatchVector( ii[ 1 ], plane_index ), CheckPatchVector( ii[ 2 ], plane_index ), CheckPatchVector( ii[ 3 ], plane_index ) };

			if ( black[ 0 ] && black[ 2 ] && !black[ 1 ] && !black[ 3 ] ) {
				outline.Add( vertex_index[ ivi[ 0 ] ], vertex_index[ ivi[ 3 ] ] );
				outline.Add( vertex_index[ ivi[ 2 ] ], vertex_index[ ivi[ 1 ] ] );
			} else if ( black[ 1 ] && black[ 3 ] && !black[ 0 ] && !black[ 2 ] ) {
				outline.Add( vertex_index[ ivi[ 1 ] ], vertex_index[ ivi[ 0 ] ] );
				outline.Add( vertex_index[ ivi[ 3 ] ], vertex_index[ ivi[ 2 ] ] );
			} else {
				int dir = 1;
				int tt[ 2 ] = { -1, -1 };
				int itt = 0;
				for ( int k = 0; k < 4; k++ ) {
					if ( vertex_index[ ivi[ k ] ] != -1 ) {
						if ( itt == 0 && black[ k ] )
							dir = 0;
						else if ( itt == 0 && !black[ k ] )
							dir = 1;
						tt[ itt ] = vertex_index[ ivi[ k ] ];
						itt++;
					}
				}
				outline.Add( tt[ dir ], tt[ 1 - dir ] );
			}
		}
	}
}

void CBPGrid::WriteOutline( char filename[] )
{
	CBPOWriter writer;
	writer.OpenFile( filename );

	writer.WriteHeader( m_pCloud->m_n64Patch, ( int )m_vecOutline.size(), m_pCloud->m_dbGroundZ, m_pCloud->m_dbGridLength );

	for ( int i = 0; i < ( int )m_vecOutline.size(); i++ ) {
		writer.WriteOutline( m_vecOutline[ i ] );
	}

	writer.CloseFile();
}

//////////////////////////////////////////////////////////////////////////
// auxiliary functions
//////////////////////////////////////////////////////////////////////////

int CBPGrid::Index( int x, int y )
{
	return ( x * m_pCloud->m_nUnitNumber[ 1 ] + y );
}

int CBPGrid::IndexX( const CVector3D & v )
{
	CVector3D diff = v - m_pCloud->m_cBoundingBox.m_vMin;
	int x = ( int )( diff[0] / m_pCloud->m_dbGridLength ) + 1 ;
	RegularizeX( x );
	return x;
}

int CBPGrid::IndexY( const CVector3D & v )
{
	CVector3D diff = v - m_pCloud->m_cBoundingBox.m_vMin;
	int y = ( int )( diff[1] / m_pCloud->m_dbGridLength ) + 1;
	RegularizeY( y );
	return y;
}

int CBPGrid::Index( const CVector3D & v )
{
	CVector3D diff = v - m_pCloud->m_cBoundingBox.m_vMin;
	int x = ( int )( diff[0] / m_pCloud->m_dbGridLength ) + 1 ;
	RegularizeX( x );
	int y = ( int )( diff[1] / m_pCloud->m_dbGridLength ) + 1;
	RegularizeY( y );
	return Index( x, y );
}

void CBPGrid::RegularizeX( int & x )
{
	if ( x < 1 )
		x = 1;
	if ( x >= m_pCloud->m_nUnitNumber[ 0 ] - 1 )
		x = m_pCloud->m_nUnitNumber[ 0 ] - 2;
}

void CBPGrid::RegularizeY( int & y )
{
	if ( y < 1 )
		y = 1;
	if ( y >= m_pCloud->m_nUnitNumber[ 1 ] - 1 )
		y = m_pCloud->m_nUnitNumber[ 1 ] - 2;
}

bool CBPGrid::CheckPatchVector( int index, int patch ) {
	PatchVector & pv = m_vecGridPatch[ index ];
	for ( int i = 0; i < ( int )pv.size(); i++ ) {
		if ( patch == pv[ i ] ) {
			return true;
		}
	}
	return false;
}

void CBPGrid::PushPatchVector( int index, int patch ) {
	if ( CheckPatchVector( index, patch ) == false ) {
		m_vecGridPatch[ index ].push_back( patch );
	}
}
