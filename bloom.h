#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "stdfloat.h"

typedef struct {
	uint64_t *bits;
	uint64_t nbits;
	int64_t  hashes;
	uint64_t items;
} Filter;


Filter* new_filter(const int64_t n, const float64_t p);
bool is_set(Filter *f, const uint64_t i);
void set(Filter *f, const uint64_t i);
void free_filter(Filter *f);
void add(Filter *f, const char* key, const size_t key_len);
bool has(Filter *f, const char* key, const size_t key_len);
void hash(const char* key, const size_t key_len, uint64_t *restrict a, uint64_t *restrict b);

// Siphash

extern int siphash(uint8_t *out, const uint8_t *in, uint64_t inlen, const uint8_t *k);