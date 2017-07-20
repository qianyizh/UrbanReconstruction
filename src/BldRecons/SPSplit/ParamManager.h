#pragma once

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

public:
	char m_pExeName[ 1024 ];
	char m_pTrueName[ 1024 ];
	char m_pDirName[ 1024 ];
	char m_pIniName[ 1024 ];

	char m_pInputFile[ 1024 ];
	char m_pTempFile[ 1024 ];
	char m_pTempSetFile[ 1024 ];
	char m_pOutputFile[ 1024 ];
	
	double m_dbNeighborDistance;
	int m_nGroundPatchAssignment;
	int m_nLargePatchPointNumber;
	double m_dbLargeGroundPatchHeightDifference;
	double m_dbSmallGroundPatchHeightDifference;
	double m_dbLargeGroundPatchMaxHeight;

	bool m_bDebugPrintOut;
	char m_pDebugPrintOutFile[ 1024 ];
};
