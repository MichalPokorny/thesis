#include "dict/splay.h"

#include <assert.h>
#include <stdlib.h>

#include "splay_tree/splay_tree.h"

static int8_t init(void** _this, void* args_unused) {
	(void) args_unused;
	struct splay_tree* this = malloc(sizeof(struct splay_tree));
	assert(this);
	this->root = NULL;
	*_this = this;
	return 0;
}

static void destroy(void** _this) {
	if (_this) {
		splay_tree_destroy((struct splay_tree **) _this);
	}
}

static int8_t find(void* _this, uint64_t key, uint64_t *value, bool *found) {
	assert(_this);
	splay_tree_find(_this, key, found, value);
	return 0;
}

static int8_t insert(void* _this, uint64_t key, uint64_t value) {
	assert(_this);
	return splay_tree_insert(_this, key, value);
}

static int8_t delete(void* _this, uint64_t key) {
	assert(_this);
	return splay_tree_delete(_this, key);
}

static int8_t next(void* _this, uint64_t key, uint64_t *next, bool *found) {
	splay_tree_next_key(_this, key, next, found);
	return 0;
}

static int8_t prev(void* _this, uint64_t key, uint64_t *prev, bool *found) {
	splay_tree_previous_key(_this, key, prev, found);
	return 0;
}

const dict_api dict_splay = {
	.init = init,
	.destroy = destroy,

	.insert = insert,
	.find = find,
	.delete = delete,

	.next = next,
	.prev = prev,

	.name = "dict_splay"
};