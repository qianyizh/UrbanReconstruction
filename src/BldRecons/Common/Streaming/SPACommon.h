#pragma once

struct SPAHeader {
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

struct SPAPoint {
	__int64 type;
	double flatness;
	double pos[3];
	double n[3];
};

struct SPACell {
	int type;			// 0 - begin chunk; 1 - end chunk;	(-1) - EOF
	int chunk_index;
	int point_number;
};

#define BLOCK_SIZE_A __max( sizeof( SPAPoint ), sizeof( SPACell ) )

struct ElementBufferA {
	unsigned int descriptor;
	char buffer[ BLOCK_SIZE_A * 32 ];
};

static const char signature_spa[] = "spa format";
