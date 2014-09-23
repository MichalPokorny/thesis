#include "resizing.h"
#include "insertion.h"

#include <stdlib.h>

#include <inttypes.h>
#include <string.h>
#include <assert.h>

#include "../../log/log.h"

static const uint64_t MIN_SIZE = 2;

static bool too_sparse(uint64_t pairs, uint64_t blocks) {
	// Keep at least MIN_SIZE blocks.
	if (blocks <= MIN_SIZE) return false;

	uint64_t buckets = blocks * 3;

	// At least one quarter full.
	return pairs * 4 < buckets;
}

static bool too_dense(uint64_t pairs, uint64_t blocks) {
	// Keep at least MIN_SIZE blocks.
	if (blocks < MIN_SIZE) return true;

	uint64_t buckets = blocks * 3;

	// At most three quarters full.
	return pairs * 4 > buckets * 3;
}

static uint64_t next_bigger_size(uint64_t x) {
	if (x < MIN_SIZE) return MIN_SIZE;

	// Next power of 2
	uint64_t i = 1;
	while (i <= x) i *= 2;
	return i;
}

static uint64_t next_smaller_size(uint64_t x) {
	// Previous power of 2
	uint64_t i = 1;
	while (i < x) i *= 2;
	return i / 2;
}

static int8_t resize(struct hashtable_data* this, uint64_t new_blocks_size);

int8_t hashtable_resize_to_fit(struct hashtable_data* this, uint64_t to_fit) {
	uint64_t new_blocks_size = this->blocks_size;

	while (too_sparse(to_fit, new_blocks_size)) {
		new_blocks_size = next_smaller_size(new_blocks_size);
	}

	while (too_dense(to_fit, new_blocks_size)) {
		new_blocks_size = next_bigger_size(new_blocks_size);
	}

	assert(!too_sparse(to_fit, new_blocks_size) &&
			!too_dense(to_fit, new_blocks_size));

	// TODO: make this operation a takeback instead?
	if (new_blocks_size != this->blocks_size) {
		// log_info("will resize to %" PRIu64 " to fit %" PRIu64,
		// 		new_blocks_size, to_fit);

		if (resize(this, new_blocks_size)) {
			log_error("failed to resize");
			return 1;
		}
	}

	return 0;
}

// TODO: make public?
// TODO: allow break?
static int8_t foreach(struct hashtable_data* this, int8_t (*iterate)(void*, uint64_t key, uint64_t value), void* opaque) {
	for (uint64_t i = 0; i < this->blocks_size; i++) {
		const struct hashtable_block* block = &this->blocks[i];

		for (int subindex = 0; subindex < 3; subindex++) {
			if (block->occupied[subindex]) {
				const uint64_t key = block->keys[subindex],
				      value = block->values[subindex];
				if (iterate(opaque, key, value)) {
					log_error("foreach iteration failed on %ld=%ld", key, value);
					return 1;
				}
			}
		}
	}
	return 0;
}

static int8_t insert_internal_wrap(void* this, uint64_t key, uint64_t value) {
	return hashtable_insert_internal(this, key, value);
}

static int8_t resize(struct hashtable_data* this, uint64_t new_blocks_size) {
	if (new_blocks_size * 3 < this->pair_count) {
		log_error("cannot resize: %" PRIu64 " blocks don't fit %" PRIu64 " pairs",
			new_blocks_size, this->pair_count);
		goto err_1;
	}
	// TODO: try new hash function?

	// Cannot use realloc, because this can both upscale and downscale.
	struct hashtable_data new_this = {
		.blocks = aligned_alloc(64, sizeof(struct hashtable_block) * new_blocks_size),
		.blocks_size = new_blocks_size,
		.pair_count = 0
	};

	// log_info("resizing to %" PRIu64, new_this.blocks_size);

	if (!new_this.blocks) {
		log_error("cannot allocate memory for %ld blocks", new_blocks_size);
		goto err_1;
	}

	memset(new_this.blocks, 0, sizeof(struct hashtable_block) * new_blocks_size);

	if (foreach(this, insert_internal_wrap, &new_this)) {
		log_error("iteration failed");
		goto err_2;
	}

	free(this->blocks);
	this->blocks = new_this.blocks;
	this->blocks_size = new_this.blocks_size;

	return 0;

err_2:
	free(new_this.blocks);
err_1:
	return 1;
}