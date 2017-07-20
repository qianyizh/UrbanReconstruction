#pragma once

class CMeshWriter
{
public:
	CMeshWriter(void);
	~CMeshWriter(void);

protected:
	FILE * m_pFile;

public:
	void OpenFile( char filename[] );
	void WriteHeader();
	void WriteVertex( const double v[ 3 ] );
	void WriteFace( const int i[3] );
	void WriteQuad( const int i[4], bool split = false );
	void CloseFile();
};
