#ifndef HTABLE_OPEN_H
#define HTABLE_OPEN_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "htable/hash.h"

#define HTLP_EMPTY UINT64_MAX

// TODO: Current sizing policy is between 1/4-full and 3/4-full.
// More free space should speed it up considerably.

// Open-addressing hash table. Linear probing, simple tabulation hashing.
typedef struct {
	uint64_t pair_count;
	uint64_t capacity;

	uint64_t *keys;            // [capacity]
	uint64_t *values;          // [capacity]
	uint32_t *keys_with_hash;  // [capacity]

	rand_generator rand;
	sth hash;
} htlp;

void htlp_init(htlp* this, rand_generator rand);
void htlp_destroy(htlp* this);
bool htlp_delete(htlp* this, uint64_t key);
bool htlp_find(htlp* this, uint64_t key, uint64_t *value);
bool htlp_insert(htlp* this, uint64_t key, uint64_t value);

#endif
