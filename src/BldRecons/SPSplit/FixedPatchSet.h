#pragma once

#include <hash_map>
#include "Lidar\LidarCommon.h"

class CFixedPatchSet
{
public:
	class CFixedPatchSet_hash_compare
	{	// traits class for hash containers
	public:
		enum
		{	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N

			CFixedPatchSet_hash_compare()
			{	// construct with default comparator
			}

			size_t operator()(const PatchIndex & _Keyval) const
			{	// hash _Keyval to size_t value by pseudorandomizing transform
				long _Quot = GLOBAL_INDEX( _Keyval ) ^ LOCAL_INDEX( _Keyval );
				ldiv_t _Qrem = ldiv(_Quot, 127773);

				_Qrem.rem = 16807 * _Qrem.rem - 2836 * _Qrem.quot;
				if (_Qrem.rem < 0)
					_Qrem.rem += LONG_MAX;
				return ((size_t)_Qrem.rem);
			}

			bool operator()(const PatchIndex & _Keyval1, const PatchIndex & _Keyval2) const
			{	// test if _Keyval1 ordered before _Keyval2
				return (_Keyval1 < _Keyval2);
			}
	};

public:
	CFixedPatchSet(void);
	~CFixedPatchSet(void);

protected:
	static CFixedPatchSet * singletonInstance;

public:
	static CFixedPatchSet * GetFixedPatchSet( void )
	{
		if ( singletonInstance == NULL )
		{
			singletonInstance = new CFixedPatchSet( );
		}
		return singletonInstance;
	}

public:
	stdext::hash_map< PatchIndex, PatchInfoEx, CFixedPatchSet_hash_compare > m_hashData;
	int m_nMerged;

public:
	void Init();
	void Push( PatchIndex patch, int num );
	void PushEx( PatchIndex patch, int num, double height );
	PatchInfo * Find( PatchInfo * patch );
	PatchInfoEx * FindEx( PatchInfo * patch );
	void Merge( PatchInfo * patch1, PatchInfo * patch2 );

public:
	void Save( char filename[] );
	void Load( char filename[] );
	static const char file_signature[];
	static const int file_version;
};
