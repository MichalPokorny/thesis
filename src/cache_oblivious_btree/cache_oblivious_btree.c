#include "cache_oblivious_btree.h"
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include "../log/log.h"
#include "../math/math.h"
#include "../veb_layout/veb_layout.h"

#define INFINITY 0xFFFFFFFFDEADBEEF

static bool subrange_find_first_gt(struct subrange subrange,
		uint64_t key, uint64_t *index) {
	for (uint64_t i = 0; i < subrange.size; i++) {
		if (subrange.occupied[i] && subrange.keys[i] > key) {
			*index = i;
			return true;
		}
	}
	return false;
}

static bool subrange_find_last_lt(struct subrange subrange,
		uint64_t key, uint64_t *index) {
	bool found_before = false;
	uint64_t index_before = INFINITY;
	for (uint64_t i = 0; i < subrange.size; i++) {
		if (subrange.occupied[i]) {
			if (found_before && subrange.keys[i] >= key) {
				*index = index_before;
				return true;
			}

			if (subrange.keys[i] < key) {
				found_before = true;
				index_before = i;
			}
		}
	}
	if (found_before) {
		*index = index_before;
		return true;
	} else {
		return false;
	}
}

static bool subrange_find(struct subrange subrange, uint64_t key,
		uint64_t *index) {
	for (uint64_t i = 0; i < subrange.size; i++) {
		if (subrange.occupied[i] && subrange.keys[i] == key) {
			if (index != NULL) {
				*index = i;
			}
			return true;
		}
	}
	return false;
}

static uint64_t subrange_get_minimum(struct subrange subrange) {
	uint64_t minimum = INFINITY;
	for (uint64_t i = 0; i < subrange.size; i++) {
		if (subrange.occupied[i] && subrange.keys[i] < minimum) {
			log_info("[%" PRIu64 "]=%" PRIu64,
					i, subrange.keys[i]);
			if (minimum > subrange.keys[i]) {
				minimum = subrange.keys[i];
			}
			assert(minimum < INFINITY);  // sanity check
		}
	}
	return minimum;
}

static struct reorg_range insert_sorted_order(struct ordered_file file,
		struct subrange subrange, uint64_t inserted_item) {
	bool found_before = false;
	uint64_t index_before;
	for (uint64_t i = 0; i < subrange.size; i++) {
		if (subrange.occupied[i]) {
			if (found_before && subrange.keys[i] > inserted_item) {
				// Insert after index_before.
				break;
			}

			if (subrange.keys[i] <= inserted_item) {
				found_before = true;
				index_before = i;
			}
		}
	}

	if (found_before) {
		const uint64_t insert_after_index =
			subrange.keys - file.keys + index_before;
		log_info("inserting %" PRIu64 " after %" PRIu64,
				inserted_item, insert_after_index);
		return ordered_file_insert_after(file,
				inserted_item, insert_after_index);
	} else {
		// TODO: hack. check this is the first subrange.
		// (otherwise we would not need to do this)
		assert(subrange.occupied == file.occupied);
		return ordered_file_insert_first(file, inserted_item);
	}
}

static uint64_t subrange_leaf_count(const struct cob* this) {
	assert(this->file.capacity % subrange_leaf_size(this->file.capacity) == 0);
	return this->file.capacity / subrange_leaf_size(this->file.capacity);
}

static uint64_t get_veb_height(const struct cob* this) {
	return exact_log2(subrange_leaf_count(this)) + 1;
}

void cob_fix_internal_node(struct cob* this, uint64_t veb_node) {
	const veb_pointer left = veb_get_left(veb_node, get_veb_height(this));
	const veb_pointer right = veb_get_right(veb_node, get_veb_height(this));
	assert(left.present && right.present);

	log_info("below %" PRIu64 ": [%" PRIu64 "]=%" PRIu64 ", "
			"[%" PRIu64 "]=%" PRIu64,
			veb_node, left.node, this->veb_minima[left.node],
			right.node, this->veb_minima[right.node]);

	const uint64_t a = this->veb_minima[left.node],
		      b = this->veb_minima[right.node];
	this->veb_minima[veb_node] = (a < b) ? a : b;
}

void cob_fix_stack(struct cob* this, uint64_t* stack, uint64_t stack_size) {
	log_info("fixing stack of size %" PRIu64, stack_size);
	for (uint64_t i = 0; i < stack_size; i++) {
		const uint64_t node = stack[stack_size - 1 - i];
		log_info("fixing node %" PRIu64, node);
		cob_fix_internal_node(this, node);
	}
}

void cob_recalculate_minima(struct cob* this, uint64_t veb_node) {
	log_info("recalculating everything under %" PRIu64, veb_node);
	if (veb_is_leaf(veb_node, get_veb_height(this))) {
		const uint64_t leaf_number =
				veb_get_leaf_index_of_leaf(veb_node,
						get_veb_height(this));
		log_info("leaf_number=%" PRIu64, leaf_number);
		const uint64_t leaf_offset = leaf_number *
				subrange_leaf_size(this->file.capacity);

		struct subrange subrange = {
			.size = subrange_leaf_size(this->file.capacity),
			.keys = this->file.keys + leaf_offset,
			.occupied = this->file.occupied + leaf_offset
		};
		this->veb_minima[veb_node] = subrange_get_minimum(subrange);
		log_info("%" PRIu64 " is a leaf. new minimum is %" PRIu64,
				veb_node, this->veb_minima[veb_node]);
	} else {
		const veb_pointer left =
			veb_get_left(veb_node, get_veb_height(this));
		const veb_pointer right =
			veb_get_right(veb_node, get_veb_height(this));
		assert(left.present && right.present);

		cob_recalculate_minima(this, left.node);
		cob_recalculate_minima(this, right.node);

		cob_fix_internal_node(this, veb_node);
	}
}

static void veb_walk(const struct cob* this, uint64_t key,
		uint64_t* stack, uint64_t* _stack_size,
		uint64_t* _leaf_index) {
	log_info("veb_walking to key %" PRIu64, key);
	uint64_t stack_size = 0;

	// Walk down vEB layout to find where does the key belong.
	uint64_t pointer = 0;  // 0 == van Emde Boas root node
	uint64_t leaf_index = 0;
	do {
		log_info("==> %" PRIu64, pointer);
		stack[stack_size++] = pointer;

		if (veb_is_leaf(pointer, get_veb_height(this))) {
			// This is the leaf.
			break;
		} else {
			const veb_pointer left =
				veb_get_left(pointer, get_veb_height(this));
			const veb_pointer right =
				veb_get_right(pointer, get_veb_height(this));
			log_info("right min = %" PRIu64,
					this->veb_minima[right.node]);

			if (key >= this->veb_minima[right.node]) {
				// We want to go right.
				pointer = right.node;
				leaf_index = (leaf_index << 1) + 1;
			} else {
				// We want to go left.
				pointer = left.node;
				leaf_index = leaf_index << 1;
			}
		}
	} while (true);

	*_stack_size = stack_size;
	*_leaf_index = leaf_index;
}

void cob_insert(struct cob* this, uint64_t key) {
	// TODO: make this recursive instead
	uint64_t node_stack[50];
	uint64_t node_stack_size = 0;

	// Walk down vEB layout to find where does the key belong.
	uint64_t leaf_index;
	veb_walk(this, key, node_stack, &node_stack_size, &leaf_index);

	// Insert into ordered file.
	const struct reorg_range reorg_range = insert_sorted_order(this->file,
			get_leaf_subrange(this->file, leaf_index), key);
	const uint64_t levels_up = exact_log2(
			reorg_range.size / subrange_leaf_size(
					this->file.capacity));

	// Rebuild reorganized subtree.
	cob_recalculate_minima(this, node_stack[node_stack_size - 1 - levels_up]);
	cob_fix_stack(this, node_stack, node_stack_size - levels_up - 1);
}

void cob_delete(struct cob* this, uint64_t key) {
	// TODO: make this recursive instead
	uint64_t node_stack[50];
	uint64_t node_stack_size = 0;

	// Walk down vEB layout to find where does the key belong.
	uint64_t leaf_index;
	veb_walk(this, key, node_stack, &node_stack_size, &leaf_index);

	// Insert into ordered file.
	struct subrange leaf_sr = get_leaf_subrange(this->file, leaf_index);
	uint64_t i;
	if (!subrange_find(leaf_sr, key, &i)) {
		log_fatal("deleting nonexistant key %" PRIu64 " "
				"(leaf index %" PRIu64 ")", key, leaf_index);
	}
	const struct reorg_range reorg_range = ordered_file_delete(this->file,
			i + leaf_index * subrange_leaf_size(this->file.capacity));
	const uint64_t levels_up = exact_log2(
			reorg_range.size / subrange_leaf_size(
					this->file.capacity));
	log_info("levels_up=%" PRIu64);

	// Rebuild reorganized subtree.
	cob_recalculate_minima(this, node_stack[node_stack_size - 1 - levels_up]);
	cob_fix_stack(this, node_stack, node_stack_size - levels_up - 1);
}

// TODO: test
void cob_has_key(const struct cob* this, uint64_t key, bool *found) {
	// TODO: make this recursive instead
	uint64_t node_stack[50];
	uint64_t node_stack_size = 0;

	// Walk down vEB layout to find where does the key belong.
	uint64_t leaf_index;
	veb_walk(this, key, node_stack, &node_stack_size, &leaf_index);
	*found = subrange_find(get_leaf_subrange(this->file, leaf_index), key,
			NULL);
}

// TODO: test
void cob_next_key(const struct cob* this, uint64_t key,
		bool *next_key_exists, uint64_t* next_key) {
	// TODO: make this recursive instead
	uint64_t node_stack[50];
	uint64_t node_stack_size = 0;

	// Walk down vEB layout to find where does the key belong.
	uint64_t leaf_index;
	veb_walk(this, key, node_stack, &node_stack_size, &leaf_index);
	struct subrange leaf = get_leaf_subrange(this->file, leaf_index);

	// TODO: plain single FOR?
	while (leaf_index < subrange_leaf_count(this)) {
		uint64_t subrange_index;
		if (subrange_find_first_gt(leaf, key, &subrange_index)) {
			*next_key_exists = true;
			*next_key = leaf.keys[subrange_index];
			return;
		}
		leaf.occupied += leaf.size;
		leaf.keys += leaf.size;
		leaf_index++;
	}

	*next_key_exists = false;
}

// TODO: test
void cob_previous_key(const struct cob* this, uint64_t key,
		bool *previous_key_exists, uint64_t* previous_key) {
	// TODO: make this recursive instead
	uint64_t node_stack[50];
	uint64_t node_stack_size = 0;

	// Walk down vEB layout to find where does the key belong.
	uint64_t leaf_index;
	veb_walk(this, key, node_stack, &node_stack_size, &leaf_index);
	struct subrange leaf = get_leaf_subrange(this->file, leaf_index);

	// TODO: plain single FOR?
	do {
		uint64_t subrange_index;
		if (subrange_find_last_lt(leaf, key, &subrange_index)) {
			*previous_key_exists = true;
			*previous_key = leaf.keys[subrange_index];
			return;
		}
		leaf.occupied -= leaf.size;
		leaf.keys -= leaf.size;
	} while (leaf_index-- > 0);

	*previous_key_exists = false;
}