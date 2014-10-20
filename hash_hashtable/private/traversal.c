#include "traversal.h"
#include "hash.h"
#include "helper.h"

#include <string.h>

uint64_t hashtable_next_index(struct hashtable_data* this, uint64_t i) {
	return (i + 1) % this->blocks_size;
}

uint8_t hashtable_scan(struct hashtable_data* this, uint64_t key,
		slot_pointer* key_slot, slot_pointer* last_slot_with_hash,
		bool* _found) {
	*_found = false;

	// Special case for empty hash tables.
	if (this->blocks_size == 0) {
		*_found = false;
		return 0;
	}

	const uint64_t key_hash = hashtable_hash_of(this, key);
	uint64_t index = key_hash;
	const uint64_t keys_with_hash = this->blocks[index].keys_with_hash;
	struct hashtable_block block;

	for (uint64_t i = 0; i < keys_with_hash; index = hashtable_next_index(this, index)) {
		// Make a local copy
		memcpy(&block, &this->blocks[index], sizeof(block));

		for (uint8_t slot = 0; slot < 3; slot++) {
			const uint64_t current_key = block.keys[slot];

			if (block.occupied[slot]) {
				if (current_key == key) {
					*_found = true;

					if (key_slot != NULL) {
						key_slot->block = &this->blocks[index];
						key_slot->slot = slot;
					}

					if (last_slot_with_hash == NULL) {
						// We don't care and we're done.
						return 0;
					}
				}

				if (hashtable_hash_of(this, current_key) == key_hash) {
					if (i == keys_with_hash - 1 &&
							last_slot_with_hash != NULL) {
						last_slot_with_hash->block = &this->blocks[index];
						last_slot_with_hash->slot = slot;
					}
					i++;
				}
			}
		}
		// TODO: guard against infinite loop?
	}

	return 0;
}
