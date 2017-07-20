#pragma once

#include <vector>

#define __pi 3.1415926535897932384626
#define __arc(x) (x) / 180.0 * __pi

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

	bool m_bWallRectangle;
};
