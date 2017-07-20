#pragma once

#include <hash_map>

#include "Grid\StreamingGrid.h"
#include "Streaming\SPBReader.h"
#include "Miscs\BPWriter.h"
#include "ConvertChunk.h"

struct PatchWriterInfo {
	int begin_cell;
	int final_cell;
	int number;
	double ground_z;
};

class CConvertGrid : public CStreamingGrid
{
public:
	CConvertGrid(void);
	~CConvertGrid(void);

public:
	CSPBReader m_cReader;
	stdext::hash_map< PatchIndex, PatchWriterInfo > m_hashWriterInfo;
	stdext::hash_map< PatchIndex, CBPWriter > m_hashWriter;
	stdext::hash_map< int, std::vector< PatchIndex > > m_hashFinalizedPatch;

	std::vector< CConvertChunk * > m_vecPointer;

public:
	void Convert();

private:
	int ReadNextChunk();

	void InitCount();
	void CountChunk( CConvertChunk * chunk );
	void FinCount();

	void InitWrite();
	void WriteChunk( CConvertChunk * chunk );
	void FinWrite();

	bool CheckPoint( PatchPointData & point );
	double MinGroundZ( CConvertChunk * chunk );
};
