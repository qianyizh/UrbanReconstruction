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
	::GetPrivateProfileStringA( "SPB2OBJ", "Input", "stdin", m_pInputFile, 1024, m_pIniName );
	
	// m_pOutputFile
	::GetPrivateProfileStringA( "SPB2OBJ", "Output", "stdout", m_pOutputFile, 1024, m_pIniName );

	// m_bClip, m_dbClip
	::GetPrivateProfileStringA( "SPB2OBJ", "Clip", "", pString, MAX_STRING, m_pIniName );
	if ( sscanf_s( pString, "%lf,%lf,%lf,%lf", &m_dbClip[0][0], &m_dbClip[1][0], & m_dbClip[0][1], &m_dbClip[1][1] ) < 4 ) {
		m_bClip = false;
	} else {
		m_bClip = true;
	}

	// m_nSampleGrid
	m_nSampleGrid = ::GetPrivateProfileIntA( "SPB2OBJ", "SampleGrid", 1, m_pIniName );
}

void CParamManager::RegisterCommandLine( int argc, char *argv[] )
{
	if ( argc > 1 ) {
		sprintf_s( m_pInputFile, 1024, "%s", argv[ 1 ] );
	}
	if ( argc > 2 ) {
		sprintf_s( m_pOutputFile, 1024, "%s", argv[ 2 ] );
	}
}

CParamManager * CParamManager::singletonInstance = NULL;
