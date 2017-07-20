#include "StdAfx.h"
#include "PointCloud.h"
#include "Miscs\BPReader.h"
#include "Miscs\BPWriter.h"

CPointCloud::CPointCloud(void)
{
}

CPointCloud::~CPointCloud(void)
{
}

void CPointCloud::LoadFromBP( char filename[] )
{

	CBPReader reader;

	reader.OpenFile( filename );
	BPHeader header = reader.ReadHeader();

	m_dbGroundZ = header.ground_z;
	m_cBoundingBox.Reset();

	m_vecPoint.clear();
	m_vecPoint.resize( header.number );
	m_vecNormal.clear();
	m_vecNormal.resize( header.number );

	for ( int i = 0; i < header.number; i++ ) {
		BPPoint point = reader.ReadPoint();

		m_vecPoint[ i ] = CVector3D( point.pos );
		m_vecNormal[ i ] = CVector3D( point.n );

		m_cBoundingBox.Push( m_vecPoint[ i ] );
	}

	reader.CloseFile();

}

void CPointCloud::LoadFromXYZN( char filename[] )
{
	m_cBoundingBox.Reset();

	m_vecPoint.clear();
	m_vecNormal.clear();

	FILE * file;
	fopen_s( & file, filename, "r" );

	char buf[ 1024 ];
	double x, y, z, nx, ny, nz;
	while ( fgets( buf, 1024, file ) != NULL ) {
		if ( buf[ 0 ] == '#' ) {
			if ( strncmp( buf, "# ground ", 9 ) == 0 ) {
				sscanf_s( & buf[ 9 ], "%lf", & m_dbGroundZ );
			}
		} else {
			sscanf_s( buf, "%lf %lf %lf %lf %lf %lf", &x, &y, &z, &nx, &ny, &nz );
			m_vecPoint.push_back( CVector3D( x, y, z ) );
			m_vecNormal.push_back( CVector3D( nx, ny, nz ) );

			m_cBoundingBox.Push( m_vecPoint.back() );
		}
	}

	fclose( file );
}

void CPointCloud::SaveToXYZN( char filename[] )
{
	FILE * file;
	fopen_s( & file, filename, "w" );

	fprintf_s( file, "# ground %.10f\n", m_dbGroundZ );

	for ( int i = 0; i < ( int )m_vecPoint.size(); i++ ) {
		CVector3D & p = m_vecPoint[ i ];
		CVector3D & n = m_vecNormal[ i ];
		fprintf_s( file, "%.10f %.10f %.10f %.10f %.10f %.10f\n", p[ 0 ], p[ 1 ], p[ 2 ], n[ 0 ], n[ 1 ], n[ 2 ] );
	}

	fclose( file );
}