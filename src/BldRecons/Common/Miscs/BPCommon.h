#pragma once

struct BPHeader {
	char signature[16];
	int version;
	int number;
	__int64 patch;
	double ground_z;
	double grid_length;
};

struct BPPoint {
	__int64 plane;
	double flatness;
	double pos[3];
	double n[3];
};

static const char signature_bp[] = "bp format";
