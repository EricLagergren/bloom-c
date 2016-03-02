#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#ifdef BLOOM_DEBUG
# include <stdio.h>
#endif

#include "bloom.h"
#include "stdfloat.h"

#define WORD 64
#define BLOOM_SHIFT 6
#define BUFFER_LENGTH 16

#define GET_WORD(i, f) ((f)->bits[(i)>>BLOOM_SHIFT])
#define MOD_WORD(i) ((i) & (WORD - 1))

bool is_set(Filter *f, const uint64_t i) {
	return (GET_WORD(i, f) >> MOD_WORD(i)) & 1;
}

void set(Filter *f, const uint64_t i) {
	GET_WORD(i, f) |= 1 << MOD_WORD(i);
}

Filter* new_filter(int64_t n, float64_t p) {
	#define LNSQ -0.480453013918201424667102526326664971730552951594545586866864133623665382259834472199948263443926990932715597661358897481255128413358268503177555294880844290839184664798896404335252423673643658092881230886029639112807153031

	#define LN2 0.693147180559945309417232121458176568075500134360255254120680009 

	Filter *f;
	float64_t n0;
	float64_t m;
	uint64_t nbits;

	n0 = (float64_t)(n);
	m = ceil(n0 * log(p) / LNSQ);
	nbits = (uint64_t)(m);


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

	f = calloc(1, sizeof(Filter));
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
void hash(const uint8_t* key, uint64_t key_len, uint64_t *a, uint64_t *b) {
	siphash(out, key, key_len, hash_key);
	*a = (uint64_t)(out);
	*b = (uint64_t)(out + 8);
}

// add adds a key to the Filter.
void add(Filter *f, const uint8_t* key, const uint64_t key_len) {
	uint64_t a = 0;
	uint64_t b = 0;
	uint64_t m;
	int64_t i;

	hash(key, key_len, &a, &b);

	m = f->nbits - 1;
	for (i = 0; i < f->hashes; ++i) {
		set(f, (a + b*i) & m);
	}
	++f->items;
}

// has returns true if the key probably exists in the Filter.
bool has(Filter *f, const uint8_t* key, const uint64_t key_len) {
	uint64_t a = 0;
	uint64_t b = 0;
	uint64_t m;
	int64_t i;

	hash(key, key_len, &a, &b);

	m = f->nbits - 1;
	for (i = 0; i < f->hashes; ++i) {
		if (!is_set(f, (a + b*i) & m)) {
			return false;
		}
	}
	return true;
}