#include "StdAfx.h"
#include "RefineGrid.h"
#include "ParamManager.h"
#include "Miscs\TimeMeter.h"

CRefineGrid::CRefineGrid(void)
{
}

CRefineGrid::~CRefineGrid(void)
{
}

void CRefineGrid::Refine()
{

	CParamManager * manager = CParamManager::GetParamManager();
	CTimeMeter timer;
	timer.Start();
	fprintf_s( stderr, "==================== Pass 2, merge patches ====================\n" );

	InitRoofPatch();

	fprintf_s( stderr, "Processing progress ... " );
	InitPrintProgress();

	int index;

	while ( ( index = ReadNextChunk() ) != -1 ) {

		PrintProgress(  );

		CRefineChunk * chunk = m_vecPointer[ index ];
		chunk->BuildGridIndex();

		MergeRoofPatch( chunk );

		delete m_vecPointer[ index ];
		m_vecPointer[ index ] = NULL;

	}

	fprintf_s( stderr, " ... done!\n" );

	FinRoofPatch();

	fprintf_s( stderr, "Total processing time is " );
	timer.End();
	timer.Print();
	fprintf_s( stderr, ".\n\n" );

	timer.Start();
	fprintf_s( stderr, "==================== Pass 3, output ====================\n" );

	InitWritePatch();

	fprintf_s( stderr, "Processing progress ... " );
	InitPrintProgress();

	while ( ( index = ReadNextChunk() ) != -1 ) {

		PrintProgress(  );

		CRefineChunk * chunk = m_vecPointer[ index ];
		chunk->BuildGridIndex();

		WritePatch( chunk );

		delete m_vecPointer[ index ];
		m_vecPointer[ index ] = NULL;

	}

	fprintf_s( stderr, " ... done!\n" );

	FinWritePatch();

	timer.End();
	timer.Print();
	fprintf_s( stderr, ".\n" );

}

int CRefineGrid::ReadNextChunk()
{
	// return the index of the finalized chunk
	// return -1 indicating a EOF has been read
	bool bNoneChunkEnd = true;
	int index;

	while ( bNoneChunkEnd ) {

		if ( m_cReader.ReadNextElement() ) {		// read chunk information

			SPBCell * cell = m_cReader.GetCell();

			switch ( cell->type ) {
			case 0:									// begin chunk
				m_vecPointer[ cell->chunk_index ] = new CRefineChunk( cell->chunk_index, cell->point_number, this );
				break;
			case 1:									// end chunk
				bNoneChunkEnd = false;
				index = cell->chunk_index;
				break;
			case -1:								// EOF
				bNoneChunkEnd = false;
				index = -1;
				break;
			}

		} else {

			SPBPoint * point = m_cReader.GetPoint();
			CVector3D v( point->pos[0], point->pos[1], point->pos[2] );
			CRefineChunk * chunk = m_vecPointer[ Index( v ) ];
			chunk->PushPoint( point );

			IncReadNumber();

		}
	}

	return index;
}

//////////////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////////////

void CRefineGrid::InitRoofPatch()
{

	CParamManager * manager = CParamManager::GetParamManager();
	m_pFixedSet = CFixedPatchSet::GetFixedPatchSet();

	m_cReader.OpenFile( manager->m_pTempFile );
	m_cReader.RegisterGrid( this );
	m_cReader.ReadHeader();

	m_vecPointer.clear();
	m_vecPointer.resize( m_nSideNumber * m_nSideNumber, NULL );

	PatchInfo max_patch;
	max_patch.num = -1;

	for ( stdext::hash_map< PatchIndex, PatchInfoEx >::iterator it = m_pFixedSet->m_hashData.begin(); it != m_pFixedSet->m_hashData.end(); it++ ) {
		if ( it->second.num > max_patch.num ) {
			max_patch.num = it->second.num;
			max_patch.base = it->second.base;
		}
	}

	fprintf_s( stderr, "Ground patch detected, label is %I64x, with %d points.\n", max_patch.base, max_patch.num );

	m_iGround = max_patch.base;

	for ( stdext::hash_map< PatchIndex, PatchInfoEx >::iterator it = m_pFixedSet->m_hashData.begin(); it != m_pFixedSet->m_hashData.end(); it++ ) {
		if ( it->second.base != max_patch.base && it->second.num >= manager->m_nGroundPatchAssignment ) {
			fprintf_s( stderr, "Large patch %I64x with %d points is merged into ground patch.\n", it->second.base, it->second.num );
			//fprintf_s( stderr, "%I64x, %d; %I64x, %d\n", max_patch.base, max_patch.num, it->second.base, it->second.num );
			m_pFixedSet->Merge( & max_patch, &( it->second ) );
		} else if ( it->second.base != max_patch.base && it->second.num >= manager->m_nLargePatchPointNumber && it->second.height <= manager->m_dbLargeGroundPatchMaxHeight ) {
			fprintf_s( stderr, "Low patch %I64x with %d points is merged into ground patch.\n", it->second.base, it->second.num, it->second.height );
			//fprintf_s( stderr, "%I64x, %d; %I64x, %d\n", max_patch.base, max_patch.num, it->second.base, it->second.num );
			m_pFixedSet->Merge( & max_patch, &( it->second ) );
		}

	}

}

void CRefineGrid::MergeRoofPatch( CRefineChunk * chunk )
{
	for ( int x = 0; x < m_nUnitNumber[ 0 ]; x++ )
		for ( int y = 0; y < m_nUnitNumber[ 1 ]; y++ ) {

			PatchPointDataVector & cell_vector = chunk->m_vecGridIndex[ chunk->Index( x, y ) ];
			PatchInfo roof;
			roof.base = -1;
			roof.num = 0;

			for ( int i = 0; i < ( int )cell_vector.size(); i++ ) {

				PatchPointData & point = *( cell_vector[ i ] );

				if ( point.type == PT_Building ) {

					PatchInfo * patch = m_pFixedSet->Find( & point.patch );

					if ( patch->base != m_iGround ) {
						if ( roof.base == -1 ) {
							roof.base = patch->base;
						} else {
							m_pFixedSet->Merge( & roof, & point.patch );
						}
					}

				}

			}

		}
}

void CRefineGrid::FinRoofPatch()
{

	m_cReader.CloseFile();

	CParamManager * manager = CParamManager::GetParamManager();
	int nLargePatch = 0;

	for ( stdext::hash_map< PatchIndex, PatchInfoEx >::iterator it = m_pFixedSet->m_hashData.begin(); it != m_pFixedSet->m_hashData.end(); it++ ) {
		if ( it->second.num >= manager->m_nLargePatchPointNumber ) {

			nLargePatch ++;

		}
	}

	fprintf_s( stderr, "Total %d roof patch detected.\n", nLargePatch );
}

void CRefineGrid::InitWritePatch()
{

	CParamManager * manager = CParamManager::GetParamManager();
	m_pFixedSet = CFixedPatchSet::GetFixedPatchSet();

	m_cReader.OpenFile( manager->m_pTempFile );
	m_cReader.RegisterGrid( this );
	m_cReader.ReadHeader();

	m_cWriter.OpenFile( manager->m_pOutputFile );
	m_cWriter.RegisterGrid( this );
	m_cWriter.WriteHeader();

	m_vecPointer.clear();
	m_vecPointer.resize( m_nSideNumber * m_nSideNumber, NULL );

	m_nTotalNumber = m_nNoiseNumber = m_nBuildingNumber = m_nTreeNumber = m_nGroundNumber = 0;

}

void CRefineGrid::WritePatch( CRefineChunk * chunk )
{
	CParamManager * manager = CParamManager::GetParamManager();

	m_cWriter.WriteBeginCell( chunk->m_iIndex, ( int )chunk->m_vecPatchPointData.size() );

	for ( int i = 0; i < ( int )( chunk->m_vecPatchPointData.size() ); i++ ) {
		PatchPointData & point = chunk->m_vecPatchPointData[ i ];

		m_nTotalNumber++;

		if ( point.type == PT_Noise ) {
			m_nNoiseNumber++;
		} else if ( point.type == PT_Tree ) {
			m_nTreeNumber++;
		} else {
			PatchInfo * info = m_pFixedSet->Find( & point.patch );
			if ( info->base == m_iGround ) {
				point.type = PT_Ground;
				point.patch.base = -1;
				point.patch.num = 0;
				m_nGroundNumber++;
			} else if ( info->num < manager->m_nLargePatchPointNumber ) {
				point.type = PT_Noise;
				point.patch.base = -1;
				point.patch.num = 0;
				m_nNoiseNumber++;
			} else {
				m_nBuildingNumber++;
			}
		}

		m_cWriter.WritePoint( point );
	}

	m_cWriter.WriteFinalizeCell( chunk->m_iIndex, ( int )chunk->m_vecPatchPointData.size() );
}

void CRefineGrid::FinWritePatch()
{

	m_cWriter.WriteEOF();
	m_cWriter.CloseFile();

	m_cReader.CloseFile();

	fprintf_s( stderr, "Summary :\n\tBuilding points - %d\n\tGround points - %d\n\tTree points - %d\n\tNoise points - %d\nTotally %d points written in ", m_nBuildingNumber, m_nGroundNumber, m_nTreeNumber, m_nNoiseNumber, m_nTotalNumber );

}

//////////////////////////////////////////////////////////////////////////
// auxiliary functions
//////////////////////////////////////////////////////////////////////////

/*

struct PatchSum {
	PatchIndex patch;
	int num;
	double z;
};

void CRefineGrid::MergeLargeGround()
{

	CParamManager * manager = CParamManager::GetParamManager();
	CTimeMeter timer;
	timer.Start();

	fprintf_s( stderr, "==================== Pass 2, merge large ground patches ====================\n" );

	InitLargeGround();

	fprintf_s( stderr, "Processing progress ... " );
	InitPrintProgress();

	int index;
	std::vector< PatchSum > patchsum;

	while ( ( index = ReadNextChunk() ) != -1 ) {

		PrintProgress(  );

		CRefineChunk * chunk = m_vecPointer[ index ];
		chunk->BuildGridIndex();

		for ( int x = 0; x < m_nUnitNumber[ 0 ]; x++ )
			for ( int y = 0; y < m_nUnitNumber[ 1 ]; y++ ) {

				PatchPointDataVector & cell_vector = chunk->m_vecGridIndex[ chunk->Index( x, y ) ];
				patchsum.clear();

				for ( int i = 0; i < ( int )cell_vector.size(); i++ ) {

					PatchPointData & point = *( cell_vector[ i ] );

					if ( point.type == PT_Building ) {

						PatchInfo * info = m_pFixedSet->Find( & point.patch );

						if ( info->num >= manager->m_nLargePatchPointNumber ) {

							int k = -1;
							for ( int j = 0; j < ( int )patchsum.size(); j++ )
								if ( patchsum[ j ].patch == info->base ) {
									k = j;
									break;
								}

							if ( k == -1 ) {
								PatchSum temp;
								temp.num = 1;
								temp.patch = info->base;
								temp.z = point.v[ 2 ];
								patchsum.push_back( temp );
							} else {
								patchsum[ k ].num++;
								patchsum[ k ].z += point.v[ 2 ];
							}

						}

					}

				}

				for ( int i = 0; i < ( int )patchsum.size(); i++ ) {
					patchsum[ i ].z /= ( double )patchsum[ i ].num;
				}

				PatchInfo tempi, tempj;
				tempi.num = tempj.num = 0;
				for ( int i = 0; i < ( int )patchsum.size(); i++ )
					for ( int j = i + 1; j < ( int )patchsum.size(); j++ )
						if ( abs( patchsum[ i ].z - patchsum[ j ].z ) <= manager->m_dbLargeGroundPatchHeightDifference ) {
							tempi.base = patchsum[ i ].patch;
							tempj.base = patchsum[ j ].patch;
							m_cLargePatchSet.Merge( & tempi, & tempj );
						}

			}

		delete m_vecPointer[ index ];
		m_vecPointer[ index ] = NULL;

	}

	fprintf_s( stderr, " ... done!\n" );

	FinLargeGround();

	fprintf_s( stderr, "Total processing time is " );
	timer.End();
	timer.Print();
	fprintf_s( stderr, ".\n" );

}

void CRefineGrid::InitLargeGround()
{

	CParamManager * manager = CParamManager::GetParamManager();
	m_pFixedSet = CFixedPatchSet::GetFixedPatchSet();

	m_cReader.OpenFile( manager->m_pTempFile );
	m_cReader.RegisterGrid( this );
	m_cReader.ReadHeader();

	m_vecPointer.clear();
	m_vecPointer.resize( m_nSideNumber * m_nSideNumber, NULL );

	m_cLargePatchSet.Init();

	for ( stdext::hash_map< PatchIndex, PatchInfo >::iterator it = m_pFixedSet->m_hashData.begin(); it != m_pFixedSet->m_hashData.end(); it++ ) {
		if ( it->second.num >= manager->m_nLargePatchPointNumber ) {

			// large patch detected!
			m_cLargePatchSet.Push( it->second.base, it->second.num );

		}
	}

	fprintf_s( stderr, "Total number of large patches is %d\n", ( int )m_cLargePatchSet.m_hashData.size() );

}

void CRefineGrid::FinLargeGround()
{
	CParamManager * manager = CParamManager::GetParamManager();
	m_cReader.CloseFile();

	PatchInfo max_patch;
	max_patch.num = -1;

	for ( stdext::hash_map< PatchIndex, PatchInfo >::iterator it = m_pFixedSet->m_hashData.begin(); it != m_pFixedSet->m_hashData.end(); it++ ) {
		if ( it->second.num > max_patch.num ) {
			max_patch.num = it->second.num;
			max_patch.base = it->second.base;
		}
	}

	PatchInfo * root = m_cLargePatchSet.Find( & max_patch );
	PatchInfo query;
	query.num = 0;

	for ( stdext::hash_map< PatchIndex, PatchInfo >::iterator it = m_cLargePatchSet.m_hashData.begin(); it != m_cLargePatchSet.m_hashData.end(); it++ ) {
		if ( it->second.num >= manager->m_nLargePatchPointNumber ) {

			query.base = it->second.base;
			PatchInfo * temp = m_cLargePatchSet.Find( & query );
			if ( temp->base == root->base ) {

				fprintf_s( stderr, "Ground patch detected, label is %I64x, with %d points.\n", it->second.base, it->second.num );
				it->second.num = -1;

			}

		}

	}
}

*/
