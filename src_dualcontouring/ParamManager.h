#pragma once

#include <vector>

#define __pi 3.1415926535897932384626

struct DualContouringParam {
	double m_dbGridLength;
	int m_nAcceptNumber;
	double m_dbRelativeDistance;
	double m_dbRelativeZ;
	double m_dbWeight;
	double m_dbErrorTolerance;
	double m_dbSingularTolerance;
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

public:
	char m_pExeName[ 1024 ];
	char m_pTrueName[ 1024 ];
	char m_pDirName[ 1024 ];
	char m_pIniName[ 1024 ];

	char m_pWorkingDir[ 1024 ];

	DualContouringParam m_cDCParam;
};
