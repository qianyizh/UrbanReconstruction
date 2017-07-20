#pragma once

#include <vector>

#define __pi 3.1415926535897932384626
#define __arc(x) (x) / 180.0 * __pi

struct ClassifyParam {
	std::vector< double > w;
	double c;
	double c_refine;
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
	void LoadParamFile();

public:
	char m_pExeName[ 1024 ];
	char m_pTrueName[ 1024 ];
	char m_pDirName[ 1024 ];
	char m_pIniName[ 1024 ];

	char m_pInputFile[ 1024 ];
	char m_pOutputFile[ 1024 ];
	char m_pParamFile[ 1024 ];

	int m_nFeatureNum;
	ClassifyParam m_cParam;

	int m_nNeighborRequirement;
	double m_dbVerticalDegreeRequirement;
	double m_dbCosVerticalDegree;

	bool m_bDebugPrintOut;
	char m_pDebugPrintOutFile[ 1024 ];
};
