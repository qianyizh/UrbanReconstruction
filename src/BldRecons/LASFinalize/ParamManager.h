#pragma once

#include <vector>

struct FileNameString {
	char name[ 1024 ];
};

class CParamManager
{
public:
	CParamManager(void);
	~CParamManager(void);

protected:
	static CParamManager * singletonInstance;

public:
	static CParamManager * GetParamManager( void )
	{
		if ( singletonInstance == NULL )
		{
			singletonInstance = new CParamManager( );
		}
		return singletonInstance;
	}

	void RegisterCommandLine( int argc, char * argv[] );

private:
	void GetNames();
	void LoadConfiguration();
	void LoadFileList();

public:
	enum FinalizerOutputFormat { FOF_SP, FOF_SPA, FOF_SPB };
	FinalizerOutputFormat m_cOutputFormat;

public:
	char m_pExeName[ 1024 ];
	char m_pTrueName[ 1024 ];
	char m_pDirName[ 1024 ];
	char m_pIniName[ 1024 ];

	char m_pFileList[ 1024 ];
	char m_pOutputFile[ 1024 ];
	int m_nFiles;
	std::vector< FileNameString > m_vecInputFiles;

	int m_nCellDepth;
	double m_dbGridLength;
	double m_dbScale;
};
