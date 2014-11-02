#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <stdint.h>

struct measurement_results {
	uint64_t cache_misses;
	uint64_t cache_references;
};

// private
struct measurement {
	int cache_misses_fd;
	int cache_references_fd;
};

struct measurement measurement_begin();
struct measurement_results measurement_end(struct measurement);

#endif