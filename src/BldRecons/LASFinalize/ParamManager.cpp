#include "StdAfx.h"
#include <direct.h>
#include <AtlConv.h>
#include "ParamManager.h"

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
	char pString[MAX_STRING];

	// m_pFileList
	::GetPrivateProfileStringA( "LASFinalize", "FileList", "stdin", m_pFileList, 1024, m_pIniName );
	
	// m_pOutputFile
	::GetPrivateProfileStringA( "LASFinalize", "Output", "stdout", m_pOutputFile, 1024, m_pIniName );
	const char * pPos = strrchr( m_pOutputFile, '.' );
	if ( _strnicmp( ".spa", pPos, 4 ) == 0 ) {
		m_cOutputFormat = FOF_SPA;
	} else if ( _strnicmp( ".spb", pPos, 4 ) == 0 ) {
		m_cOutputFormat = FOF_SPB;
	} else {
		m_cOutputFormat = FOF_SP;
	}

	// m_nCellDepth
	m_nCellDepth = ::GetPrivateProfileIntA( "LASFinalize", "CellDepth", 9, m_pIniName );

	// m_dbGridLength
	::GetPrivateProfileStringA( "LASFinalize", "GridLength", "1.0", pString, MAX_STRING, m_pIniName );
	m_dbGridLength = atof( pString );

	// m_dbScale
	::GetPrivateProfileStringA( "LASFinalize", "Scale", "1.0", pString, MAX_STRING, m_pIniName );
	m_dbScale = atof( pString );
}

void CParamManager::LoadFileList()
{
	FILE * file;
	fopen_s( &file, m_pFileList, "r" );
	fscanf_s( file, "%d", &m_nFiles );

	m_vecInputFiles.clear();
	m_vecInputFiles.resize( m_nFiles );

	for ( int i = 0; i < m_nFiles; i++ ) {
		memset( m_vecInputFiles[ i ].name, 0, sizeof( char ) * 1024 );
		fscanf_s( file, "%s", m_vecInputFiles[ i ].name, 1024 );
	}

	fclose( file );
}

void CParamManager::RegisterCommandLine( int argc, char *argv[] )
{
	if ( argc > 1 ) {
		sprintf_s( m_pFileList, 1024, "%s", argv[ 1 ] );
	}
	if ( argc > 2 ) {
		sprintf_s( m_pOutputFile, 1024, "%s", argv[ 2 ] );
	}
	LoadFileList();
}

CParamManager * CParamManager::singletonInstance = NULL;
