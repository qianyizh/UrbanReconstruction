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
	::GetPrivateProfileStringA( "BPOHist", "WorkingDir", "C:\\Lidar\\", m_pWorkingDir, 1024, m_pIniName );

	// m_pWorkingDir
	::GetPrivateProfileStringA( "BPOHist", "SPBFile", "stdin", m_pSPBFile, 1024, m_pIniName );

	// m_pOutputFile
	::GetPrivateProfileStringA( "BPOHist", "Output", "stdout", m_pOutputFile, 1024, m_pIniName );

	// m_dbCenterDistance
	::GetPrivateProfileStringA( "BPOHist", "CenterDistance", "500.0", pString, MAX_STRING, m_pIniName );
	m_dbCenterDistance = atof( pString );

	// m_dbHistogramDistance
	::GetPrivateProfileStringA( "BPOHist", "HistogramDistance", "500.0", pString, MAX_STRING, m_pIniName );
	m_dbHistogramDistance = atof( pString );

	// m_nHistogramPrecision
	m_nHistogramPrecision = ::GetPrivateProfileIntA( "BPOHist", "HistogramPrecision", 100, m_pIniName );

	// m_dbHistogramSmoothSigma
	::GetPrivateProfileStringA( "BPOHist", "HistogramSmoothSigma", "1.5", pString, MAX_STRING, m_pIniName );
	m_dbHistogramSmoothSigma = atof( pString );

	// m_dbHistogramPeakTolerance
	::GetPrivateProfileStringA( "BPOHist", "HistogramPeakTolerance", "1.5", pString, MAX_STRING, m_pIniName );
	m_dbHistogramPeakTolerance = atof( pString );
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
		sprintf_s( m_pOutputFile, 1024, "%s", argv[ 3 ] );
	}
}

CParamManager * CParamManager::singletonInstance = NULL;
