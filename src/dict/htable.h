#ifndef DICT_HTABLE_H_INCLUDED
#define DICT_HTABLE_H_INCLUDED

#include "dict/dict.h"

// Open addressing hashing.
//
// If a bucket is already used, we just keep pushing to the right.

extern const dict_api dict_htable;

#endif