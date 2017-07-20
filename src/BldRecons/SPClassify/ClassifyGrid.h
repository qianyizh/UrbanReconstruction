#pragma once

#include <vector>
#include "Grid\StreamingGrid.h"
#include "Streaming\SPReader.h"
#include "Streaming\SPAWriter.h"
#include "ClassifyChunk.h"

class CClassifyGrid : public CStreamingGrid
{
public:
	CClassifyGrid(void);
	~CClassifyGrid(void);

public:
	CSPReader m_cReader;
	CSPAWriter m_cWriter;

	std::vector< CClassifyChunk * > m_vecPointer;
	std::vector< CClassifyChunk::ClassifyChunkState > m_vecState;

public:
	void Classify();

private:
	void Init();
	void Fin();

	int ReadNextChunk();

	void NormalChunk( CClassifyChunk * chunk );
	void ClassifyChunk( CClassifyChunk * chunk );
	void RefineChunk( CClassifyChunk * chunk );
	void WriteChunk( CClassifyChunk * chunk );

	//void NotifyCCSRead( CClassifyChunk * chunk );
	//void NotifyCCSNormaled( CClassifyChunk * chunk );
	//void NotifyCCSClassified( CClassifyChunk * chunk );
	//void NotifyCCSRefined( CClassifyChunk * chunk );
	void NotifyCCSRead( int ix, int iy );
	void NotifyCCSNormaled( int ix, int iy );
	void NotifyCCSClassified( int ix, int iy );
	void NotifyCCSRefined( int ix, int iy );

	inline bool CheckCCSRead( int idx ) {
		return ( m_vecState[ idx ] >= CClassifyChunk::CCS_Read );
	}
	inline bool CheckCCSNormaled( int idx ) {
		return ( m_vecState[ idx ] >= CClassifyChunk::CCS_Normaled );
	}
	inline bool CheckCCSClassified( int idx ) {
		return ( m_vecState[ idx ] >= CClassifyChunk::CCS_Classified );
	}
	inline bool CheckCCSRefined( int idx ) {
		return ( m_vecState[ idx ] >= CClassifyChunk::CCS_Refined );
	}

private:
	PointDataVector m_vecPointData;
	void GetNeighbor( PointData & point, int dis, CClassifyChunk * chunk, int x, int y );

protected:
	int m_nTotalNumber;
	int m_nBuildingNumber;
	int m_nTreeNumber;
	int m_nNoiseNumber;

private:
	void PrintGridState( const char filename[] );
};
