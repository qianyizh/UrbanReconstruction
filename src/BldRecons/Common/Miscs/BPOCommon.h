#pragma once

struct BPOHeader {
	char signature[16];
	int version;
	int number;
	__int64 patch;
	double ground_z;
	double grid_length;
};

static const char signature_bpo[] = "bpo format";
