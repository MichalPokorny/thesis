#ifndef COBT_OFM_H
#define COBT_OFM_H

#include <stdbool.h>
#include <stdint.h>

struct {
	uint64_t reorganized_size;
} OFM_COUNTERS;

// Stores an ordered list with uint64_t keys and void* values (typedefed
// as ofm_value).
// The density of the entire structure is within [0.5;0.75].

typedef void* ofm_value;

typedef struct {
	// TODO: store as bitmap
	bool* occupied;
	uint64_t* keys;
	ofm_value* values;

	uint64_t capacity;
	uint64_t block_size;
} ofm;

typedef struct {
	uint64_t begin;
	uint64_t size;

	ofm* file;
} ofm_range;

void ofm_dump(ofm file);
ofm_range ofm_insert_before(ofm* file, uint64_t key, ofm_value value,
		uint64_t insert_before_index);
ofm_range ofm_delete(ofm* file, uint64_t index);
void ofm_init(ofm* file);
void ofm_destroy(ofm* file);

ofm_value ofm_get_value(ofm* file, uint64_t index);

typedef struct {
	ofm* file;
	uint64_t scratch;
	uint64_t allowed_capacity;
} ofm_stream;

void ofm_stream_start(ofm* file, uint64_t size, ofm_stream *stream);
void ofm_stream_push(ofm_stream* stream, uint64_t key, ofm_value value);

#endif