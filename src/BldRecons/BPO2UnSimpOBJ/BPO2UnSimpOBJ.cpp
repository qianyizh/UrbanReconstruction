// BPO2OBJ.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Geometry\Outline.h"
#include "Miscs\BPOReader.h"
#include "Miscs\OBJWriter.h"
#include "ParamManager.h"

void main( int argc, char * argv[] )
{

	CParamManager * manager = CParamManager::GetParamManager();
	manager->RegisterCommandLine( argc, argv );

	char pSearch[ 1024 ];
	sprintf_s( pSearch, 1024, "%s*.bpo", manager->m_pWorkingDir );

	WIN32_FIND_DATA finder;
	HANDLE handle = FindFirstFile( pSearch, & finder );
	BOOL not_finished = ( handle != INVALID_HANDLE_VALUE );

	while ( not_finished ) {

		if ( strcmp( finder.cFileName, "." ) != 0 && strcmp( finder.cFileName, ".." ) != 0 ) {

			if ( ( finder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != FILE_ATTRIBUTE_DIRECTORY ) {

				fprintf_s( stderr, "Processing %s ... ", finder.cFileName );

				char filename[ 1024 ];
				memset( filename, 0, 1024 );
				sprintf_s( filename, 1024, "%s%s", manager->m_pWorkingDir, finder.cFileName );
				CBPOReader reader;
				reader.OpenFile( filename );

				char truename[ 1024 ];
				memset( truename, 0, 1024 );
				strncat_s( truename, 1024, filename, strlen( filename ) - 3 );
				sprintf_s( filename, "%sobj", truename );
				COBJWriter writer;
				writer.OpenFile( filename );

				BPOHeader header = reader.ReadHeader();
				std::vector< COutline > vecOutline( header.number );

				writer.WriteHeader();

				for ( int i = 0; i < header.number; i++ ) {
					reader.ReadOutline( vecOutline[ i ] );
					writer.WriteOutline( vecOutline[ i ], header.ground_z, manager->m_bWallRectangle );
				}

				reader.CloseFile();
				writer.CloseFile();

				fprintf_s( stderr, "done.\n" );

			}

			not_finished = FindNextFile( handle, &finder );

		}

	}

	FindClose( handle );

}
