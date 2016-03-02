#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "stdfloat.h"

typedef struct {
	char 	 *bits;
	uint64_t nbits;
	int 	 hashes;
	uint64_t items;
} Filter;


Filter* new_filter(int64_t n, float64_t p);
bool is_set(Filter *f, const uint64_t i);
void set(Filter *f, const uint64_t i);
void free_filter(Filter *f);
void add(Filter *f, const uint8_t* key, const uint64_t key_len);
bool has(Filter *f, const uint8_t* key, const uint64_t key_len);
void hash(const uint8_t* key, uint64_t key_len, uint64_t *a, uint64_t *b);

// Siphash

extern int siphash(uint8_t *out, const uint8_t *in, uint64_t inlen, const uint8_t *k);

#define DOUBLE 1 // for siphash