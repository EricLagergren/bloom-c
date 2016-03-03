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
void free_filter(Filter *f);
void add(Filter *f, const char* key, const size_t key_len);
bool has(Filter *f, const char* key, const size_t key_len);

// Siphash

extern int siphash(uint8_t *out, const uint8_t *in, uint64_t inlen, const uint8_t *k);