#pragma once

#include <hash_set>

#include "Grid\StreamingGrid.h"
#include "Streaming\SPBReader.h"
#include "Miscs\MeshWriter.h"
#include "MeshChunk.h"

class CMeshGrid : public CStreamingGrid
{
public:
	CMeshGrid(void);
	~CMeshGrid(void);

public:
	CSPBReader m_cReader;
	CMeshWriter m_cWriter;

	std::vector< CMeshChunk * > m_vecPointer;

	std::vector< double > m_vecHeight;

public:
	void Mesh();

private:
	void Init();
	void ComputeGroundHeight( CMeshChunk * chunk );
	void Fin();
	void Interpolate();
	void WriteMesh( int sample_grid );

private:
	int ReadNextChunk();

protected:
	int m_nVertexNumber;
	int m_nFaceNumber;

protected:
	int m_nClip[ 2 ][ 2 ];
	void InitClip();
	int ClipIndex( int x, int y );
};
