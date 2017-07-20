#include "StdAfx.h"
#include "SplitGrid.h"
#include "ParamManager.h"
#include "Miscs\TimeMeter.h"

CSplitGrid::CSplitGrid(void)
{
}

CSplitGrid::~CSplitGrid(void)
{
}

void CSplitGrid::Split()
{
	CParamManager * manager = CParamManager::GetParamManager();
	CTimeMeter timer;
	timer.Start();

	fprintf_s( stderr, "==================== Pass 1, split points ====================\n" );

	Init();

	fprintf_s( stderr, "Initialize ... finished.\n" );

	fprintf_s( stderr, "Processing progress ... " );
	InitPrintProgress();

	int index;
	while ( ( index = ReadNextChunk() ) != -1 ) {

		PrintProgress(  );

		CSplitChunk * chunk = m_vecPointer[ index ];
		chunk->BuildGridIndex();

		// check neighborhood of index chunk
		m_vecState[ chunk->m_iIndex ] = CSplitChunk::SCS_Read;
		NotifySCSRead( chunk->m_iX, chunk->m_iY );

		if ( manager->m_bDebugPrintOut ) {
			PrintGridState( manager->m_pDebugPrintOutFile );
		}

	}

	m_cWriter.WriteEOF();
	m_cWriter.CloseFile();

	m_cReader.CloseFile();

	fprintf_s( stderr, " ... done!\n" );

	fprintf_s( stderr, "Total number of patches is : %d (%d)\n", ( int )m_pFixedSet->m_hashData.size() - m_pFixedSet->m_nMerged, ( int )m_pFixedSet->m_hashData.size() );

	fprintf_s( stderr, "Total processing time is " );
	timer.End();
	timer.Print();
	fprintf_s( stderr, ".\n\n" );

	m_pFixedSet->Save( manager->m_pTempSetFile );
}

void CSplitGrid::Init()
{
	CParamManager * manager = CParamManager::GetParamManager();
	m_pFixedSet = CFixedPatchSet::GetFixedPatchSet();
	m_pFixedSet->Init();

	m_cReader.OpenFile( manager->m_pInputFile );
	m_cReader.RegisterGrid( this );
	m_cReader.ReadHeader();

	m_cWriter.OpenFile( manager->m_pTempFile );
	m_cWriter.RegisterGrid( this );
	m_cWriter.WriteHeader();

	m_vecPointer.clear();
	m_vecPointer.resize( m_nSideNumber * m_nSideNumber, NULL );
	m_vecState.clear();
	m_vecState.resize( m_nSideNumber * m_nSideNumber, CSplitChunk::SCS_New );
}

int CSplitGrid::ReadNextChunk()
{
	// return the index of the finalized chunk
	// return -1 indicating a EOF has been read
	bool bNoneChunkEnd = true;
	int index;

	while ( bNoneChunkEnd ) {

		if ( m_cReader.ReadNextElement() ) {		// read chunk information

			SPACell * cell = m_cReader.GetCell();

			switch ( cell->type ) {
			case 0:									// begin chunk
				m_vecPointer[ cell->chunk_index ] = new CSplitChunk( cell->chunk_index, cell->point_number, this );
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

			SPAPoint * point = m_cReader.GetPoint();
			CVector3D v( point->pos[0], point->pos[1], point->pos[2] );
			CSplitChunk * chunk = m_vecPointer[ Index( v ) ];
			chunk->PushPoint( point );

			IncReadNumber();

		}
	}

	return index;
}

//////////////////////////////////////////////////////////////////////////
// main functions
//////////////////////////////////////////////////////////////////////////

void CSplitGrid::NotifySCSRead( int ix, int iy )
{
	for ( int x = MinBound( ix ); x <= MaxBound( ix ); x++ )
		for ( int y = MinBound( iy ); y <= MaxBound( iy ); y++ ) {

			bool bAllRead = true;

			for ( int xx = MinBound( x ); xx <= MaxBound( x ) && bAllRead; xx++ )
				for ( int yy = MinBound( y ); yy <= MaxBound( y ) && bAllRead; yy++ ) {
					if ( InBound( xx, yy ) && CheckSCSRead( Index( xx, yy ) ) == false ) {
						bAllRead = false;
					}
				}

			if ( bAllRead ) {
				SplitChunk( m_vecPointer[ Index( x, y ) ] );
			}

		}
}

void CSplitGrid::NotifySCSSplitted( int ix, int iy )
{
	for ( int x = MinBound( ix ); x <= MaxBound( ix ); x++ )
		for ( int y = MinBound( iy ); y <= MaxBound( iy ); y++ ) {

			bool bAllRefined = true;

			for ( int xx = MinBound( x ); xx <= MaxBound( x ) && bAllRefined; xx++ )
				for ( int yy = MinBound( y ); yy <= MaxBound( y ) && bAllRefined; yy++ ) {
					if ( InBound( xx, yy ) && CheckSCSSplitted( Index( xx, yy ) ) == false ) {
						bAllRefined = false;
					}
				}

			if ( bAllRefined ) {
				WriteChunk( m_vecPointer[ Index( x, y ) ] );
				delete m_vecPointer[ Index( x, y ) ];
				m_vecPointer[ Index( x, y ) ] = NULL;
			}
		}
}

void CSplitGrid::SplitChunk( CSplitChunk * chunk )
{
	CParamManager * manager = CParamManager::GetParamManager();

	for ( int x = 0; x < m_nUnitNumber[ 0 ]; x++ )
		for ( int y = 0; y < m_nUnitNumber[ 1 ]; y++ ) {
			PatchPointDataVector & cell_vector = chunk->m_vecGridIndex[ chunk->Index( x, y ) ];
			for ( int i = 0; i < ( int )cell_vector.size(); i++ ) {
				PatchPointData & point = *( cell_vector[ i ] );
				if ( point.type == PT_Building ) {
					MergeNeighbor( point, 1, manager->m_dbNeighborDistance, chunk, x, y );
				}
			}
		}

	FixChunk( chunk, 1 );

	m_vecState[ chunk->m_iIndex ] = chunk->SCS_Splitted;
	NotifySCSSplitted( chunk->m_iX, chunk->m_iY );
}

void CSplitGrid::MergeNeighbor( PatchPointData & point, int ndis, double distance, CSplitChunk * chunk, int x, int y )
{
	double distance2 = distance * distance;

	for ( int xx = x - ndis; xx <= x + ndis; xx++ )
		for ( int yy = y - ndis; yy <= y + ndis; yy++ ) {
			int i = xx;
			int j = yy;
			CSplitChunk * data_chunk = chunk;
			while ( i < 0 && data_chunk != NULL ) {
				i += m_nUnitNumber[ 0 ];
				data_chunk = InBound( data_chunk->m_iX - 1, data_chunk->m_iY ) ? m_vecPointer[ Index( data_chunk->m_iX - 1, data_chunk->m_iY ) ] : NULL;
			}
			while ( i > m_nUnitNumber[ 0 ] - 1 && data_chunk != NULL ) {
				i -= m_nUnitNumber[ 0 ];
				data_chunk = InBound( data_chunk->m_iX + 1, data_chunk->m_iY ) ? m_vecPointer[ Index( data_chunk->m_iX + 1, data_chunk->m_iY ) ] : NULL;
			}
			while ( j < 0 && data_chunk != NULL ) {
				j += m_nUnitNumber[ 1 ];
				data_chunk = InBound( data_chunk->m_iX, data_chunk->m_iY - 1 ) ? m_vecPointer[ Index( data_chunk->m_iX, data_chunk->m_iY - 1 ) ] : NULL;
			}
			while ( j > m_nUnitNumber[ 1 ] - 1 && data_chunk != NULL ) {
				j -= m_nUnitNumber[ 1 ];
				data_chunk = InBound( data_chunk->m_iX, data_chunk->m_iY + 1 ) ? m_vecPointer[ Index( data_chunk->m_iX, data_chunk->m_iY + 1 ) ] : NULL;
			}

			if ( data_chunk != NULL ) {
				PatchPointDataVector & cell_vector = data_chunk->m_vecGridIndex[ data_chunk->Index( i, j ) ];
				for ( int k = 0; k < ( int )cell_vector.size(); k++ ) {
					PatchPointData & neighbor = *( cell_vector[ k ] );
					if ( neighbor.type == PT_Building ) {
						CVector3D diff = point.v - neighbor.v;
						if ( diff.length2() < distance2 ) {

							MergePatch( & point.patch, & neighbor.patch );

						}
					}
				}
			}
		}
}

PatchInfo * CSplitGrid::FindPatch( PatchInfo * info, bool & fixed )
{
	fixed = false;

	PatchIndex patchfirst = -1;
	PatchInfo * patchsecond = info;

	while ( patchfirst != patchsecond->base ) {
		patchfirst = patchsecond->base;
		int global_patch = GLOBAL_INDEX( patchsecond->base );
		int local_patch = LOCAL_INDEX( patchsecond->base );
		//if ( CheckSCSWritten( global_patch ) ) {
		if ( CheckSCSSplitted( global_patch ) ) {
			// fixed!
			patchsecond = m_pFixedSet->Find( patchsecond );
			break;
		} else {
			patchsecond = & m_vecPointer[ global_patch ]->m_vecPatchPointData[ local_patch ].patch;
		}
	}

	patchfirst = -1;
	PatchInfo * temppatch = info;

	while ( patchfirst != temppatch->base ) {
		patchfirst = temppatch->base;
		int global_patch = GLOBAL_INDEX( temppatch->base );
		int local_patch = LOCAL_INDEX( temppatch->base );
		//if ( CheckSCSWritten( global_patch ) ) {
		if ( CheckSCSSplitted( global_patch ) ) {
			fixed = true;
			break;
		} else {
			temppatch->base = patchsecond->base;
			temppatch = & m_vecPointer[ global_patch ]->m_vecPatchPointData[ local_patch ].patch;
		}
	}

	return patchsecond;
}

void CSplitGrid::MergePatch( PatchInfo * info1, PatchInfo * info2 )
{
	bool fixed1, fixed2;
	PatchInfo * root1 = FindPatch( info1, fixed1 );
	PatchInfo * root2 = FindPatch( info2, fixed2 );

	if ( info1->base == info2->base )
		return;

	if ( fixed1 && fixed2 ) {

		m_pFixedSet->Merge( info1, info2 );

	} else if ( !fixed1 && !fixed2 ) {

		if ( root1->num >= root2->num ) {
			root2->base = root1->base;
			root1->num += root2->num;
			root2->num = 0;
			FindPatch( info2, fixed2 );
		} else {
			root1->base = root2->base;
			root2->num += root1->num;
			root1->num = 0;
			FindPatch( info1, fixed1 );
		}

	} else if ( fixed1 && !fixed2 ) {
		
		root2->base = root1->base;
		root1->num += root2->num;
		root2->num = 0;
		FindPatch( info2, fixed2 );

	} else if ( !fixed1 && fixed2 ) {

		root1->base = root2->base;
		root2->num += root1->num;
		root1->num = 0;
		FindPatch( info1, fixed1 );

	}
}

void CSplitGrid::FixChunk( CSplitChunk * chunk, int ndis )
{
	// clean up all the possible dangling pointers
	for ( int xx = 0 - ndis; xx < m_nUnitNumber[ 0 ] + ndis; xx++ )
		for ( int yy = 0 - ndis; yy < m_nUnitNumber[ 1 ] + ndis; yy++ ) {
			int i = xx;
			int j = yy;
			CSplitChunk * data_chunk = chunk;
			while ( i < 0 && data_chunk != NULL ) {
				i += m_nUnitNumber[ 0 ];
				data_chunk = InBound( data_chunk->m_iX - 1, data_chunk->m_iY ) ? m_vecPointer[ Index( data_chunk->m_iX - 1, data_chunk->m_iY ) ] : NULL;
			}
			while ( i > m_nUnitNumber[ 0 ] - 1 && data_chunk != NULL ) {
				i -= m_nUnitNumber[ 0 ];
				data_chunk = InBound( data_chunk->m_iX + 1, data_chunk->m_iY ) ? m_vecPointer[ Index( data_chunk->m_iX + 1, data_chunk->m_iY ) ] : NULL;
			}
			while ( j < 0 && data_chunk != NULL ) {
				j += m_nUnitNumber[ 1 ];
				data_chunk = InBound( data_chunk->m_iX, data_chunk->m_iY - 1 ) ? m_vecPointer[ Index( data_chunk->m_iX, data_chunk->m_iY - 1 ) ] : NULL;
			}
			while ( j > m_nUnitNumber[ 1 ] - 1 && data_chunk != NULL ) {
				j -= m_nUnitNumber[ 1 ];
				data_chunk = InBound( data_chunk->m_iX, data_chunk->m_iY + 1 ) ? m_vecPointer[ Index( data_chunk->m_iX, data_chunk->m_iY + 1 ) ] : NULL;
			}

			if ( data_chunk != NULL ) {
				PatchPointDataVector & cell_vector = data_chunk->m_vecGridIndex[ data_chunk->Index( i, j ) ];
				for ( int k = 0; k < ( int )cell_vector.size(); k++ ) {
					PatchPointData & point = *( cell_vector[ k ] );
					bool temp;
					FindPatch( & point.patch, temp );
				}
			}
		}

	// then push all the new patches into the FixedSet
	for ( int i = 0; i < ( int )( chunk->m_vecPatchPointData.size() ); i++ ) {
		if ( chunk->m_vecPatchPointData[ i ].patch.num > 0 ) {
			m_pFixedSet->PushEx( PATCH_INDEX( chunk->m_iIndex, i ), chunk->m_vecPatchPointData[ i ].patch.num, chunk->m_vecPatchPointData[ i ].v[ 2 ] );
		}
	}

}


void CSplitGrid::WriteChunk( CSplitChunk *chunk )
{
	m_cWriter.WriteBeginCell( chunk->m_iIndex, ( int )chunk->m_vecPatchPointData.size() );

	for ( int i = 0; i < ( int )( chunk->m_vecPatchPointData.size() ); i++ ) {

		m_cWriter.WritePoint( chunk->m_vecPatchPointData[ i ] );

	}

	m_cWriter.WriteFinalizeCell( chunk->m_iIndex, ( int )chunk->m_vecPatchPointData.size() );

	m_vecState[ chunk->m_iIndex ] = chunk->SCS_Written;
}

/*
void CSplitGrid::RecheckAllSplitted()
{
	for ( int i = 0; i < m_nSideNumber * m_nSideNumber; i++ ) {
		if ( m_vecState[ i ] == CSplitChunk::SCS_Splitted || m_vecState[ i ] == CSplitChunk::SCS_Read ) {
			CSplitChunk * chunk = m_vecPointer[ i ];
			for ( int j = 0; j < ( int )chunk->m_vecPatchPointData.size(); j++ ) {
				bool temp;
				FindPatch( & chunk->m_vecPatchPointData[ j ].patch, temp );
			}
		}
	}
}

void CSplitGrid::RecheckAllDirty()
{
	for ( stdext::hash_set< int >::iterator it = m_hashsetDirtyChunk.begin(); it != m_hashsetDirtyChunk.end(); it++ ) {
		CSplitChunk * chunk = m_vecPointer[ *it ];
		for ( int j = 0; j < ( int )chunk->m_vecPatchPointData.size(); j++ ) {
			bool temp;
			FindPatch( & chunk->m_vecPatchPointData[ j ].patch, temp );
		}
	}
}
*/

//////////////////////////////////////////////////////////////////////////
// auxiliary functions
//////////////////////////////////////////////////////////////////////////
void CSplitGrid::PrintGridState( const char filename[] )
{
	FILE * file;
	fopen_s( &file, filename, "a" );

	for ( int i = 0; i < m_nSideNumber; i++ ) {
		for ( int j = 0; j < m_nSideNumber; j++ ) {
			int idx = Index( i, j );
			fprintf_s( file, "%d ", m_vecState[ idx ] );
		}
		fprintf_s( file, "\n" );
	}

	fclose( file );
}
