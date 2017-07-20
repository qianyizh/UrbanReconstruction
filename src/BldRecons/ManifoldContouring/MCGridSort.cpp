#include "StdAfx.h"
#include "MCGrid.h"

//////////////////////////////////////////////////////////////////////////
// sort functions
//////////////////////////////////////////////////////////////////////////

int compare( const void *arg1, const void *arg2 ) {
	CMCGrid::DistanceSorting * n1 = ( CMCGrid::DistanceSorting * ) arg1;
	CMCGrid::DistanceSorting * n2 = ( CMCGrid::DistanceSorting * ) arg2;
	if ( n1->distance < n2->distance )
		return -1;
	else
		return 1;
}

void CMCGrid::SortDistance2D( CVector3DPointer_Vector & points, CVector3D & ref_v, std::vector< int > & cluster, int cluster_index, DistanceSorting_Vector & sorting_result )
{
	for ( int i = 0; i < ( int )cluster.size(); i++ ) {
		if ( cluster[ i ] == cluster_index ) {
			sorting_result.push_back( DistanceSorting( i, ( * points[ i ] - ref_v ).XY().length() ) );
		}
	}
	qsort( &( sorting_result[0] ), sorting_result.size(), sizeof( DistanceSorting ), compare );
}

void CMCGrid::QSort( DistanceSorting_Vector & sorting_result )
{
	qsort( &( sorting_result[0] ), sorting_result.size(), sizeof( DistanceSorting ), compare );
}
