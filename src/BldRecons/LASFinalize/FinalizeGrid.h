#pragma once

#include <vector>
#include "Grid\StreamingGrid.h"
#include "liblas\laspoint.hpp"
#include "Streaming\SPWriter.h"
#include "Streaming\SPAWriter.h"
#include "Streaming\SPBWriter.h"

typedef std::vector< CVector3D > ChunkData;

class CFinalizeGrid : public CStreamingGrid
{
public:
	CFinalizeGrid(void);
	~CFinalizeGrid(void);

public:
	std::vector< int > m_vecGridIndex;
	std::vector< int > m_vecGridNumber;
	CSPWriter m_cWriter;
	CSPAWriter m_cAWriter;
	CSPBWriter m_cBWriter;

public:
	void ComputeBoundingBox();				// first pass
	void Stream_ComputeGridIndex();			// second pass
	void Stream_WriteGrid();				// third pass

protected:
	void ComputeGridLength();
	bool CheckPoint( const liblas::LASPoint & point );
	void InitWrite();
	void FinWrite();
	void WriteChunk( ChunkData * data, int index, int number );
};
