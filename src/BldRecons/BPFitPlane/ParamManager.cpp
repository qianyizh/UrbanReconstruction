#include "StdAfx.h"
#include <direct.h>
#include <AtlConv.h>
#include "ParamManager.h"
#include <math.h>

CParamManager::CParamManager(void)
{
	GetNames();
	LoadConfiguration();
}

CParamManager::~CParamManager(void)
{
}

void CParamManager::GetNames()
{
	USES_CONVERSION;

	memset( m_pExeName, 0, sizeof(char) * 1024 );
	memset( m_pDirName, 0, sizeof(char) * 1024 );
	memset( m_pTrueName, 0, sizeof(char) * 1024 );
	memset( m_pIniName, 0, sizeof(char) * 1024 );

	TCHAR pFileName[MAX_PATH];
	::GetModuleFileName( NULL, pFileName, MAX_PATH );
	sprintf_s( m_pExeName, 1024, T2A( pFileName ) );

	_getcwd(m_pDirName, sizeof(m_pDirName));

	char * pTemp = strrchr( m_pExeName, '\\' );
	pTemp = strrchr( m_pExeName, '.' );
	strncat_s( m_pTrueName, 1024, m_pExeName, pTemp - m_pExeName );

	sprintf_s( m_pIniName, 1024, "%s\\config.ini", m_pDirName );
}

void CParamManager::LoadConfiguration()
{
	const int MAX_STRING = 1024;
	char pString[ MAX_STRING ];

	// m_pWorkingDir
	::GetPrivateProfileStringA( "BPFitPlane", "WorkingDir", "C:\\Lidar\\", m_pWorkingDir, 1024, m_pIniName );

	// m_cFittingParam
	::GetPrivateProfileStringA( "BPFitPlane", "SeedMaxFlat", "0.05", pString, MAX_STRING, m_pIniName );
	m_cFittingParam.m_dbSeedMaxFlat = atof( pString );

	::GetPrivateProfileStringA( "BPFitPlane", "AngleMaxDifference", "8.0", pString, MAX_STRING, m_pIniName );
	m_cFittingParam.m_dbAngleMaxDifference = atof( pString );
	m_cFittingParam.m_dbCosAngleMaxDifference = cos( __arc( m_cFittingParam.m_dbAngleMaxDifference ) );

	::GetPrivateProfileStringA( "BPFitPlane", "MaxDistance", "0.3", pString, MAX_STRING, m_pIniName );
	m_cFittingParam.m_dbMaxDistance = atof( pString );
	
	m_cFittingParam.m_nAcceptPointNumber = ::GetPrivateProfileIntA( "BPFitPlane", "AcceptPointNumber", 25, m_pIniName );

	// m_nOutlineNeighbor
	m_nOutlineNeighbor = ::GetPrivateProfileIntA( "BPFitPlane", "OutlineNeighbor", 3, m_pIniName );
}

void CParamManager::RegisterCommandLine( int argc, char *argv[] )
{
	if ( argc > 1 ) {
		sprintf_s( m_pWorkingDir, 1024, "%s", argv[ 1 ] );
	}
}

CParamManager * CParamManager::singletonInstance = NULL;
