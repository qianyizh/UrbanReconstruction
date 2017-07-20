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

	char * pTemp = strrchr( m_pExeName, '\\' );
	_getcwd(m_pDirName, sizeof(m_pDirName));

	pTemp = strrchr( m_pExeName, '.' );
	strncat_s( m_pTrueName, 1024, m_pExeName, pTemp - m_pExeName );

	sprintf_s( m_pIniName, 1024, "%s\\config.ini", m_pDirName );
}

void CParamManager::LoadConfiguration()
{
	const int MAX_STRING = 1024;
	char pString[ MAX_STRING ];

	// m_pInputList
	::GetPrivateProfileStringA( "SPClassify", "Input", "stdin", m_pInputFile, 1024, m_pIniName );
	
	// m_pOutputFile
	::GetPrivateProfileStringA( "SPClassify", "Output", "stdout", m_pOutputFile, 1024, m_pIniName );

	// m_pParamFile
	::GetPrivateProfileStringA( "SPClassify", "ParamFile", "", m_pParamFile, 1024, m_pIniName );

	// m_nFeatureNum
	m_nFeatureNum = ::GetPrivateProfileIntA( "SPClassify", "FeatureNum", 5, m_pIniName );

	// m_nNeighborRequirement
	m_nNeighborRequirement = ::GetPrivateProfileIntA( "SPClassify", "NeighborRequirement", 10, m_pIniName );

	// m_dbVerticalDegreeRequirement
	::GetPrivateProfileStringA( "SPClassify", "VerticalDegreeRequirement", "80.0", pString, MAX_STRING, m_pIniName );
	m_dbVerticalDegreeRequirement = atof( pString );
	m_dbCosVerticalDegree = cos( __arc( m_dbVerticalDegreeRequirement ) );

	// m_pDebugPrintOutFile
	::GetPrivateProfileStringA( "SPClassify", "DebugPrintOut", "", m_pDebugPrintOutFile, 1024, m_pIniName );
	m_bDebugPrintOut = ( strlen( m_pDebugPrintOutFile ) > 0 );
}

void CParamManager::RegisterCommandLine( int argc, char *argv[] )
{
	if ( argc > 1 ) {
		sprintf_s( m_pParamFile, 1024, "%s", argv[ 1 ] );
	}
	if ( argc > 2 ) {
		sprintf_s( m_pInputFile, 1024, "%s", argv[ 2 ] );
	}
	if ( argc > 3 ) {
		sprintf_s( m_pOutputFile, 1024, "%s", argv[ 3 ] );
	}
	LoadParamFile();
}

void CParamManager::LoadParamFile()
{
	m_cParam.w.resize( m_nFeatureNum, 0.0 );
	FILE * file;
	fopen_s( &file, m_pParamFile, "r" );
	
	for ( int i = 0; i < m_nFeatureNum; i++ )
		fscanf_s( file, "%lf", & m_cParam.w[ i ] );

	fscanf_s( file, "%lf", & m_cParam.c );
	fscanf_s( file, "%lf", & m_cParam.c_refine );

	fclose( file );
}

CParamManager * CParamManager::singletonInstance = NULL;
