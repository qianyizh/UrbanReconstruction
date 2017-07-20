// SPInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include "Streaming\SPReader.h"
#include "Grid\StreamingGrid.h"

void PrintHelp() {
	fprintf_s( stderr, "Please use correct command format:\n\n" );
	fprintf_s( stderr, "\t> SPInfo input_file output_file\n\n" );
}

void main( int argc, char * argv[] )
{
	if ( argc <= 2 ) {
		PrintHelp();
		exit( 0 );
	}

	CStreamingGrid grid;

	CSPReader reader;
	reader.OpenFile( argv[ 1 ] );
	reader.RegisterGrid( &grid );
	reader.ReadHeader();

	std::vector< int > grid_index( grid.m_nSideNumber * grid.m_nSideNumber, -1 );

	int num = 0;
	int chunk_num = 0;
	int progress = 0;

	fprintf_s( stderr, "Converting progress ...   0%%" );

	bool bLoop = true;
	while ( bLoop ) {
		if ( reader.ReadNextElement() ) {		// read chunk information

			SPCell * cell = reader.GetCell();
			if ( cell->type == -1 ) {
				bLoop = false;
			} else if ( cell->type == 1 ) {
				grid_index[ cell->chunk_index ] = chunk_num;
				chunk_num++;
			}

		} else {

			num++;

			int new_progress = ( int )( num * 100.0 / grid.m_nPointNumber );
			if ( new_progress > progress ) {
				progress = new_progress;
				fprintf_s( stderr, "\b\b\b\b%3d%%", progress );
			}

		}
	}

	fprintf_s( stderr, " ... done.\n" );
	fprintf_s( stderr, "Writing to info file ...... " );

	FILE * file;
	fopen_s( & file, argv[ 2 ], "w" );
	for ( int x = 0; x < grid.m_nSideNumber; x++ ) {
		for ( int y = 0; y < grid.m_nSideNumber; y++ ) {
			fprintf_s( file, "%d ", grid_index[ x * grid.m_nSideNumber + y ] );
		}
		fprintf_s( file, "\n" );
	}

	fprintf_s( stderr, "done.\n" );

}
