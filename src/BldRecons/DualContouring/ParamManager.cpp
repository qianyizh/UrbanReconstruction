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
	::GetPrivateProfileStringA( "DualContouring", "WorkingDir", "C:\\Lidar\\", m_pWorkingDir, 1024, m_pIniName );

	// m_cDCParam
	::GetPrivateProfileStringA( "DualContouring", "GridLength", "1.0", pString, MAX_STRING, m_pIniName );
	m_cDCParam.m_dbGridLength = atof( pString );

	m_cDCParam.m_nAcceptNumber = ::GetPrivateProfileIntA( "DualContouring", "AcceptNumber", 4, m_pIniName );

	::GetPrivateProfileStringA( "DualContouring", "RelativeDistance", "1.0", pString, MAX_STRING, m_pIniName );
	m_cDCParam.m_dbRelativeDistance = atof( pString );

	::GetPrivateProfileStringA( "DualContouring", "RelativeZ", "0.6", pString, MAX_STRING, m_pIniName );
	m_cDCParam.m_dbRelativeZ = atof( pString );

	::GetPrivateProfileStringA( "DualContouring", "Weight", "4.0", pString, MAX_STRING, m_pIniName );
	m_cDCParam.m_dbWeight = atof( pString );

	::GetPrivateProfileStringA( "DualContouring", "ErrorTolerance", "1.5", pString, MAX_STRING, m_pIniName );
	m_cDCParam.m_dbErrorTolerance = atof( pString );

	::GetPrivateProfileStringA( "DualContouring", "SingularTolerance", "0.15", pString, MAX_STRING, m_pIniName );
	m_cDCParam.m_dbSingularTolerance = atof( pString );

	// m_bWallRectangle
	m_bWallRectangle = ( bool )::GetPrivateProfileIntA( "DualContouring", "WallRectangle", 0, m_pIniName );

	// m_bAntiNonManifold
	m_bAntiNonManifold = ( bool )::GetPrivateProfileIntA( "DualContouring", "AntiNonManifold", 1, m_pIniName );

	// m_cSnappingParam
	m_cSnappingParam.m_bEnableSnapping = ( bool )::GetPrivateProfileIntA( "DualContouring", "EnableSnapping", 0, m_pIniName );

	::GetPrivateProfileStringA( "DualContouring", "SPBFile", "", m_cSnappingParam.m_pSPBFile, 1024, m_pIniName );

	::GetPrivateProfileStringA( "DualContouring", "HistFile", "", m_cSnappingParam.m_pHistFile, 1024, m_pIniName );

	::GetPrivateProfileStringA( "DualContouring", "SnappingErrorTolerance", "1.5", pString, MAX_STRING, m_pIniName );
	m_cSnappingParam.m_dbErrorTolerance = atof( pString );

	::GetPrivateProfileStringA( "DualContouring", "SnappingMinimumLength", "7.0", pString, MAX_STRING, m_pIniName );
	m_cSnappingParam.m_dbMinimumLength = atof( pString );

}

void CParamManager::RegisterCommandLine( int argc, char *argv[] )
{
	if ( argc > 1 ) {
		sprintf_s( m_pWorkingDir, 1024, "%s", argv[ 1 ] );
	}
}

CParamManager * CParamManager::singletonInstance = NULL;
