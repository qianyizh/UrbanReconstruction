#pragma once

#include <hash_set>

#include "Grid\StreamingGrid.h"
#include "Streaming\SPAReader.h"
#include "Streaming\SPBWriter.h"
#include "SplitChunk.h"
#include "FixedPatchSet.h"

class CSplitGrid : public CStreamingGrid
{
public:
	CSplitGrid(void);
	~CSplitGrid(void);

public:
	CSPAReader m_cReader;
	CSPBWriter m_cWriter;

	std::vector< CSplitChunk * > m_vecPointer;
	std::vector< CSplitChunk::SplitChunkState > m_vecState;

	CFixedPatchSet * m_pFixedSet;

public:
	void Split();

private:
	void Init();
	int ReadNextChunk();

	void SplitChunk( CSplitChunk * chunk );
	void WriteChunk( CSplitChunk * chunk );

	void NotifySCSRead( int ix, int iy );
	void NotifySCSSplitted( int ix, int iy );

	inline bool CheckSCSRead( int idx ) {
		return ( m_vecState[ idx ] >= CSplitChunk::SCS_Read );
	}
	inline bool CheckSCSSplitted( int idx ) {
		return ( m_vecState[ idx ] >= CSplitChunk::SCS_Splitted );
	}
	inline bool CheckSCSWritten( int idx ) {
		return ( m_vecState[ idx ] >= CSplitChunk::SCS_Written );
	}

protected:
	void MergeNeighbor( PatchPointData & point, int ndis, double distance, CSplitChunk * chunk, int x, int y );
	PatchInfo * FindPatch( PatchInfo * info, bool & fixed );
	void MergePatch( PatchInfo * info1, PatchInfo * info2 );
	void FixChunk( CSplitChunk * chunk, int ndis );

private:
	void PrintGridState( const char filename[] );
};
