#include "StdAfx.h"
#include "FeatureCalculator.h"
#include "ParamManager.h"
#include "Miscs\NumericalSolver.h"

CFeatureCalculator::CFeatureCalculator(void)
{
}

CFeatureCalculator::~CFeatureCalculator(void)
{
}

void CFeatureCalculator::ComputeCoVariance( PointData & point, PointDataVector & data )
{
	CParamManager * manager = CParamManager::GetParamManager();
	if ( point.type == PT_Noise || ( int )data.size() < manager->m_nNeighborRequirement ) {
		point.type = PT_Noise;
		return;
	}

	CVector3D center( 0.0, 0.0, 0.0 );

	for ( int i = 0; i < ( int )data.size(); i++ ) {
		center += ( *( data[ i ] ) ).v;
	}
	center /= ( double )data.size();

	double e[3][3];
	double v[3][3];
	double d[3];

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ )
			e[i][j] = 0.0;
	
	for ( int i = 0; i < ( int )data.size(); i++ ) {
		CVector3D diff = ( * ( data[ i ] ) ).v - center;
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
		point.n = CVector3D( v[0][2], v[1][2], v[2][2] );
	} else {
		point.n = CVector3D( -v[0][2], -v[1][2], -v[2][2] );
	}

	point.feature[ 0 ] = ( point.v - center ).length();
	point.feature[ 1 ] = 1.0 - fabs( point.n[ 2 ] );
	point.feature[ 2 ] = d[ 2 ] * 3.0 / ( d[ 0 ] + d[ 1 ] + d[ 2 ] );

	if ( point.n[ 2 ] < manager->m_dbCosVerticalDegree )
		point.type = PT_Noise;
}

void CFeatureCalculator::ComputeNormalCoVariance( PointData & point, PointDataVector & data )
{
	CParamManager * manager = CParamManager::GetParamManager();
	if ( point.type == PT_Noise || ( int )data.size() < manager->m_nNeighborRequirement ) {
		point.type = PT_Noise;
		return;
	}

	double e[3][3];
	double v[3][3];
	double d[3];

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ )
			e[i][j] = 0.0;

	int valid_point = 0;

	for ( int i = 0; i < ( int )data.size(); i++ ) {
		if ( ( * ( data[ i ] ) ).type != PT_Noise ) {
			valid_point ++;

			CVector3D & diff = ( * ( data[ i ] ) ).n;
			e[ 0 ][ 0 ] += diff[ 0 ] * diff[ 0 ];
			e[ 0 ][ 1 ] += diff[ 0 ] * diff[ 1 ];
			e[ 0 ][ 2 ] += diff[ 0 ] * diff[ 2 ];
			e[ 1 ][ 1 ] += diff[ 1 ] * diff[ 1 ];
			e[ 1 ][ 2 ] += diff[ 1 ] * diff[ 2 ];
			e[ 2 ][ 2 ] += diff[ 2 ] * diff[ 2 ];
		}
	}

	if ( valid_point < manager->m_nNeighborRequirement ) {
		point.type = PT_Noise;
		return;
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

	point.feature[ 3 ] = d[ 1 ];
	point.feature[ 4 ] = d[ 2 ];

	double t = 0.0;
	for ( int i = 0; i < manager->m_nFeatureNum; i++ )
		t += manager->m_cParam.w[ i ] * point.feature[ i ];
	if ( t < manager->m_cParam.c )
		point.type = PT_Tree;
	else
		point.type = PT_Building;
}

void CFeatureCalculator::RefineClassification( PointData & point, PointDataVector & data )
{
	CParamManager * manager = CParamManager::GetParamManager();
	if ( point.type == PT_Noise || ( int )data.size() < manager->m_nNeighborRequirement ) {
		point.type = PT_Noise;
		return;
	}

	int building_point = 0;
	for ( int i = 0; i < ( int )data.size(); i++ ) {
		if ( ( * ( data[ i ] ) ).type == PT_Building ) {
			building_point ++;
		}
	}

	point.buildingness = ( double )building_point / ( double )data.size();
}
