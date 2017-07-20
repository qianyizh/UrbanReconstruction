#pragma once

#include <hash_set>

#include "Grid\StreamingGrid.h"
#include "Streaming\SPBReader.h"
#include "Streaming\SPBWriter.h"
#include "RefineChunk.h"
#include "FixedPatchSet.h"

class CRefineGrid : public CStreamingGrid
{
public:
	CRefineGrid(void);
	~CRefineGrid(void);

public:
	CSPBReader m_cReader;
	CSPBWriter m_cWriter;

	std::vector< CRefineChunk * > m_vecPointer;

	CFixedPatchSet * m_pFixedSet;

	PatchIndex m_iGround;

public:
	void Refine();

private:
	int ReadNextChunk();

	void InitRoofPatch();
	void MergeRoofPatch( CRefineChunk * chunk );
	void FinRoofPatch();

	void InitWritePatch();
	void WritePatch( CRefineChunk * chunk );
	void FinWritePatch();

protected:
	int m_nTotalNumber;
	int m_nBuildingNumber;
	int m_nTreeNumber;
	int m_nNoiseNumber;
	int m_nGroundNumber;
};

/*
	CFixedPatchSet m_cLargePatchSet;
	void MergeLargeGround();
	void MergeSmallGround();

	void InitLargeGround();
	void FinLargeGround();
*/
