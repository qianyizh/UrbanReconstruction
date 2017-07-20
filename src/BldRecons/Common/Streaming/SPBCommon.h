#pragma once

struct SPBHeader {
	char signature[16];
	int version;
	int number;
	double pos_min[3];
	double pos_max[3];
	int element_size;
	int cell_depth;
	double grid_length;
	int unit_number[2];
};

struct SPBPoint {
	__int64 type;
	__int64 patch;
	double flatness;
	double pos[3];
	double n[3];
};

struct SPBCell {
	int type;			// 0 - begin chunk; 1 - end chunk;	(-1) - EOF
	int chunk_index;
	int point_number;
};

#define BLOCK_SIZE_B __max( sizeof( SPBPoint ), sizeof( SPBCell ) )

struct ElementBufferB {
	unsigned int descriptor;
	char buffer[ BLOCK_SIZE_B * 32 ];
};

static const char signature_spb[] = "spb format";
