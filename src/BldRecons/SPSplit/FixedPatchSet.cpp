#include "StdAfx.h"
#include "FixedPatchSet.h"

CFixedPatchSet::CFixedPatchSet(void)
{
}

CFixedPatchSet::~CFixedPatchSet(void)
{
}

void CFixedPatchSet::Init()
{
	m_hashData.clear();
	m_nMerged = 0;
}

void CFixedPatchSet::PushEx( PatchIndex patch, int num, double height )
{
	PatchInfoEx p;
	p.base = patch;
	p.num = num;
	p.height = height;
	m_hashData.insert( std::pair< PatchIndex, PatchInfoEx >( patch, p ) );
}

PatchInfoEx * CFixedPatchSet::FindEx( PatchInfo * patch )
{
	stdext::hash_map< PatchIndex, PatchInfoEx >::iterator it = m_hashData.find( patch->base );
	stdext::hash_map< PatchIndex, PatchInfoEx >::iterator oldit = it;
	while ( it->second.base != it->first ) {
		it = m_hashData.find( it->second.base );
	}

	patch->base = it->first;
	while ( oldit->second.base != oldit->first ) {
		PatchIndex next = oldit->second.base;
		oldit->second.base = it->first;
		oldit = m_hashData.find( next );
	}

	return &( it->second );
}

PatchInfo * CFixedPatchSet::Find( PatchInfo * patch )
{
	return ( PatchInfo * )FindEx( patch );
}

void CFixedPatchSet::Merge( PatchInfo * patch1, PatchInfo * patch2 )
{
	m_nMerged++;

	PatchInfoEx * p1 = FindEx( patch1 );
	PatchInfoEx * p2 = FindEx( patch2 );
	double height = ( p1->height * p1->num + p2->height * p2->num ) / ( double )( p1->num + p2->num );
	if ( p1->base != p2->base ) {
		if ( p1->num >= p2->num ) {
			p2->base = p1->base;
			p1->num += p2->num;
			p1->height = height;
			p2->num = 0;
			Find( patch2 );
		} else {
			p1->base = p2->base;
			p2->num += p1->num;
			p2->height = height;
			p1->num = 0;
			Find( patch1 );
		}
	}
}

void CFixedPatchSet::Save( char filename[] )
{
	FILE * file;
	fopen_s( & file, filename, "w" );

	fprintf_s( file, "%s %d\n", file_signature, file_version );

	fprintf_s( file, "%d %d\n", m_hashData.size(), m_nMerged );

	for ( stdext::hash_map< PatchIndex, PatchInfoEx >::iterator it = m_hashData.begin(); it != m_hashData.end(); it++ ) {
		fprintf_s( file, "%I64x %I64x %d %.4f\n", it->first, it->second.base, it->second.num, it->second.height );
	}

	fclose( file );
}

void CFixedPatchSet::Load( char filename[] )
{
	m_hashData.clear();

	FILE * file;
	fopen_s( & file, filename, "r" );

	char header[ 1024 ];
	fgets( header, 1024, file );

	int temp;
	PatchIndex tempindex;
	PatchInfoEx tempinfo;

	fscanf_s( file, "%d %d", & temp, & m_nMerged );

	for ( int i = 0; i < temp; i++ ) {
		fscanf_s( file, "%I64x %I64x %d %lf", &tempindex, &tempinfo.base, &tempinfo.num, &tempinfo.height );
		m_hashData.insert( std::pair< PatchIndex, PatchInfoEx >( tempindex, tempinfo ) );
	}

	fclose( file );
}

const char CFixedPatchSet::file_signature[] = "CFixedPatchSet";
const int CFixedPatchSet::file_version = 1;

CFixedPatchSet * CFixedPatchSet::singletonInstance = NULL;