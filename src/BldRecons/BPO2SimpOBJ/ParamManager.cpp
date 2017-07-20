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

	const char pStr_LeastSquare[] = "LeastSquare";

	// m_pWorkingDir
	::GetPrivateProfileStringA( "BPO2SimpOBJ", "WorkingDir", "C:\\Lidar\\", m_pWorkingDir, 1024, m_pIniName );

	// m_pSPBFile
	::GetPrivateProfileStringA( "BPO2SimpOBJ", "SPBFile", "stdin", m_pSPBFile, 1024, m_pIniName );

	// m_pHistFile
	::GetPrivateProfileStringA( "BPO2SimpOBJ", "HistFile", "stdout", m_pHistFile, 1024, m_pIniName );

	// m_nSegmentMinimumPointNumber
	m_nSegmentMinimumPointNumber = ::GetPrivateProfileIntA( "BPO2SimpOBJ", "SegmentMinimumPointNumber", 7, m_pIniName );

	// m_bLineSnapping
	m_bLineSnapping = ( bool )::GetPrivateProfileIntA( "BPO2SimpOBJ", "LineSnapping", 0, m_pIniName );

	// m_bWallRectangle
	m_bWallRectangle = ( bool )::GetPrivateProfileIntA( "BPO2SimpOBJ", "WallRectangle", 0, m_pIniName );

	// m_dbLineErrorTolerance
	::GetPrivateProfileStringA( "BPO2SimpOBJ", "LineErrorTolerance", "1.5", pString, MAX_STRING, m_pIniName );
	m_dbLineErrorTolerance = atof( pString );

	// m_dbCornerErrorTolerance
	::GetPrivateProfileStringA( "BPO2SimpOBJ", "CornerErrorTolerance", "5.0", pString, MAX_STRING, m_pIniName );
	m_dbCornerErrorTolerance = atof( pString );

	// m_dbCornerAngleTolerance
	::GetPrivateProfileStringA( "BPO2SimpOBJ", "CornerAngleTolerance", "45.0", pString, MAX_STRING, m_pIniName );
	m_dbCornerAngleTolerance = atof( pString );
	m_dbCosCornerAngleTolerance = cos( __arc( m_dbCornerAngleTolerance ) );

	// m_nFittingMode
	::GetPrivateProfileStringA( "BPO2SimpOBJ", "FittingMode", "DirectConnect", pString, MAX_STRING, m_pIniName );
	if ( _stricmp( pString, pStr_LeastSquare ) == 0 ) {
		m_nFittingMode = LFM_LeastSquare;
	} else {
		m_nFittingMode = LFM_DirectConnect;
	}
}

void CParamManager::RegisterCommandLine( int argc, char *argv[] )
{
	if ( argc > 1 ) {
		sprintf_s( m_pWorkingDir, 1024, "%s", argv[ 1 ] );
	}
	if ( argc > 2 ) {
		sprintf_s( m_pSPBFile, 1024, "%s", argv[ 2 ] );
	}
	if ( argc > 3 ) {
		sprintf_s( m_pHistFile, 1024, "%s", argv[ 3 ] );
	}
}

CParamManager * CParamManager::singletonInstance = NULL;
