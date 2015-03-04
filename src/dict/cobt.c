#include "dict/cobt.h"

#include "log/log.h"
#include "cobt/cobt.h"

#include <stdlib.h>

static int8_t init(void** _this, void* args_unused) {
	(void) args_unused;

	struct cob* cob = malloc(sizeof(struct cob));
	if (!cob) return 1;

	cob_init(cob);
	*_this = cob;

	return 0;
}

static void destroy(void** _this) {
	if (_this) {
		struct cob* cob = * (struct cob**) _this;
		cob_destroy(*cob);
		free(cob);
		*_this = NULL;
	}
}

static int8_t find(void* _this, uint64_t key, uint64_t *value, bool *found) {
	cob_find(_this, key, found, value);  // TODO: fix arg order
	return 0;
}

static int8_t insert(void* _this, uint64_t key, uint64_t value) {
	return cob_insert(_this, key, value);
}

static int8_t delete(void* _this, uint64_t key) {
	return cob_delete(_this, key);
}

static void check(void* _this) {
	return cob_check(_this);
}

static int8_t next(void* _this, uint64_t key, uint64_t *next_key, bool *found) {
	cob_next_key(_this, key, found, next_key);
	return 0;
}

static int8_t prev(void* _this, uint64_t key, uint64_t *prev_key, bool *found) {
	cob_previous_key(_this, key, found, prev_key);
	return 0;
}

const dict_api dict_cobt = {
	.init = init,
	.destroy = destroy,

	.insert = insert,
	.find = find,
	.delete = delete,

	.check = check,

	.next = next,
	.prev = prev,

	.name = "dict_cobt"
};
