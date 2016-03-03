#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "bloom.h"
#include "stdfloat.h"

#define WORD 64
#define BLOOM_SHIFT 6
#define BUFFER_LENGTH 16

#define U8TO64_LE(p)                                         \
  (((uint64_t)((p)[0])) | ((uint64_t)((p)[1]) << 8) |        \
   ((uint64_t)((p)[2]) << 16) | ((uint64_t)((p)[3]) << 24) | \
   ((uint64_t)((p)[4]) << 32) | ((uint64_t)((p)[5]) << 40) | \
   ((uint64_t)((p)[6]) << 48) | ((uint64_t)((p)[7]) << 56))


#define GET_WORD(f, i) ((f)->bits[(i)>>BLOOM_SHIFT])
#define MOD_WORD(i) ((i) & (WORD - 1))

static bool is_set(Filter *f, const uint64_t i);
static void set(Filter *f, const uint64_t i);
static void hash(const char* key, const size_t key_len, uint64_t *restrict a, uint64_t *restrict b);

static bool is_set(Filter *f, const uint64_t i) {
	return ((GET_WORD(f, i) >> MOD_WORD(i)) & 1) != 0;
}

static void set(Filter *f, const uint64_t i) {
	GET_WORD(f, i) |= (uint64_t)(1) << MOD_WORD(i);
}

// new_filter creates a new Filter, initialized to hold up to
// n elements with p false-positive probability.
// p should be a float64_t (aka double) between 0 and 1
// indicating the false-positive rate.
Filter* new_filter(const int64_t n, const float64_t p) {

	// log(1 / pow(2, log(2)))
	#define LNSQ -0.480453013918201424667102526326664971730552951594545586866864133623665382259834472199948263443926990932715597661358897481255128413358268503177555294880844290839184664798896404335252423673643658092881230886029639112807153031

	// http://oeis.org/A002162
	#define LN2 0.693147180559945309417232121458176568075500134360255254120680009 

	float64_t n0 = (float64_t)(n);
	float64_t m = ceil(n0 * log(p) / LNSQ);
	uint64_t nbits = (uint64_t)(m);

	if (nbits <= 512) {
		nbits = 512;
	} else {
		--nbits;
		nbits |= nbits >> 1;
		nbits |= nbits >> 2;
		nbits |= nbits >> 4;
		nbits |= nbits >> 8;
		nbits |= nbits >> 16;
		nbits |= nbits >> 32;
		++nbits;
	}

	Filter *f = calloc(1, sizeof(Filter));
	if (f == NULL) {
		return NULL;
	}

	f->bits = calloc(nbits>>6, sizeof(uint64_t));
	if (f->bits == NULL) {
		free(f);
		return NULL;
	}
	f->nbits = nbits;
	f->hashes = (int64_t)(ceil(LN2 * m / n0));
	f->items = 0;

	return f;
}

// free_filter frees f.
void free_filter(Filter *f) {
	if (f != NULL) {
		free(f->bits);
		free(f);
	}
}

static const uint8_t hash_key[BUFFER_LENGTH] = {
	210, 40, 228, 187, 182, 248, 110, 170,
	199, 138, 64, 14, 234, 233, 23, 133
};

static uint8_t out[BUFFER_LENGTH] = { 0 };

// hash hashes key using siphash and places both halves of the
// 128-bit return value inside a and b.
static void hash(const char* key, const size_t key_len, uint64_t *restrict a, uint64_t *restrict b) {

	// Need to be uint8_t* for siphash's API...
	siphash(out, (const uint8_t *)(key), (const uint64_t)(key_len), hash_key);
	*a = U8TO64_LE(out);
	*b = U8TO64_LE(out + 8);
}

// add adds a key to the Filter.
void add(Filter *f, const char* key, const size_t key_len) {
	uint64_t a = 0;
	uint64_t b = 0;

	hash(key, key_len, &a, &b);

	uint64_t m = f->nbits - 1;
	for (int64_t i = 0; i < f->hashes; ++i) {
		set(f, (a + b*i) & m);
	}
	++f->items;
}

// has returns true if the key probably exists in the Filter.
bool has(Filter *f, const char* key, const size_t key_len) {
	uint64_t a = 0;
	uint64_t b = 0;

	hash(key, key_len, &a, &b);

	uint64_t m = f->nbits - 1;
	for (int64_t i = 0; i < f->hashes; ++i) {
		if (!is_set(f, (a + b*i) & m)) {
			return false;
		}
	}
	return true;
}