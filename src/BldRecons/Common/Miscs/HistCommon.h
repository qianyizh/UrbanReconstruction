#pragma once

struct HistHeader {
	char signature[16];
	int version;
	int number;
	double center_distance;
	double histogram_distance;
};

static const char signature_hist[] = "hist format";
