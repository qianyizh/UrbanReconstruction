#pragma once

struct SPHeader {
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

struct SPPoint {
	double pos[3];
};

struct SPCell {
	int type;			// 0 - begin chunk; 1 - end chunk;	(-1) - EOF
	int chunk_index;
	int point_number;
};

#define BLOCK_SIZE __max( sizeof( SPPoint ), sizeof( SPCell ) )

struct ElementBuffer {
	unsigned int descriptor;
	char buffer[ BLOCK_SIZE * 32 ];
};

static const char signature_sp[] = "sp format";
