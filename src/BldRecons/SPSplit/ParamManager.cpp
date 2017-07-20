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

	// m_pInputList
	::GetPrivateProfileStringA( "SPSplit", "Input", "stdin", m_pInputFile, 1024, m_pIniName );
	
	// m_pTempFile
	::GetPrivateProfileStringA( "SPSplit", "Temp", "C:\\lidar.tmp", m_pTempFile, 1024, m_pIniName );

	// m_pTempSetFile
	::GetPrivateProfileStringA( "SPSplit", "TempSet", "C:\\lidar.set", m_pTempSetFile, 1024, m_pIniName );

	// m_pOutputFile
	::GetPrivateProfileStringA( "SPSplit", "Output", "stdout", m_pOutputFile, 1024, m_pIniName );

	// m_dbNeighborDistance
	::GetPrivateProfileStringA( "SPSplit", "NeighborDistance", "1.0", pString, 1024, m_pIniName );
	m_dbNeighborDistance = atof( pString );

	// m_nGroundPatchAssignment
	m_nGroundPatchAssignment = ::GetPrivateProfileIntA( "SPSplit", "GroundPatchAssignment", 1000000, m_pIniName );

	// m_nLargePatchPointNumber
	m_nLargePatchPointNumber = ::GetPrivateProfileIntA( "SPSplit", "LargePatchPointNum", 1000, m_pIniName );

	// m_dbLargeGroundPatchHeightDifference
	::GetPrivateProfileStringA( "SPSplit", "LargeGroundPatchHeightDifference", "1.0", pString, 1024, m_pIniName );
	m_dbLargeGroundPatchHeightDifference = atof( pString );

	// m_dbSmallGroundPatchHeightDifference
	::GetPrivateProfileStringA( "SPSplit", "SmallGroundPatchHeightDifference", "3.0", pString, 1024, m_pIniName );
	m_dbSmallGroundPatchHeightDifference = atof( pString );

	// m_dbLargeGroundPatchMaxHeight
	::GetPrivateProfileStringA( "SPSplit", "GroundPatchMaxHeight", "-10000.0", pString, 1024, m_pIniName );
	m_dbLargeGroundPatchMaxHeight = atof( pString );

	// m_pDebugPrintOutFile
	::GetPrivateProfileStringA( "SPSplit", "DebugPrintOut", "", m_pDebugPrintOutFile, 1024, m_pIniName );
	m_bDebugPrintOut = ( strlen( m_pDebugPrintOutFile ) > 0 );
}

void CParamManager::RegisterCommandLine( int argc, char *argv[] )
{
	if ( argc > 1 ) {
		sprintf_s( m_pInputFile, 1024, "%s", argv[ 1 ] );
	}
	if ( argc > 2 ) {
		sprintf_s( m_pTempFile, 1024, "%s", argv[ 2 ] );
	}
	if ( argc > 3 ) {
		sprintf_s( m_pOutputFile, 1024, "%s", argv[ 3 ] );
	}
}

CParamManager * CParamManager::singletonInstance = NULL;
