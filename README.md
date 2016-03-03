# bloom-c
Bloom filter in C

## Docs

```
// new_filter creates a new Filter, initialized to hold up to
// n elements with p false-positive probability.
// p should be a float64_t (aka double) between 0 and 1
// indicating the false-positive rate.
Filter* new_filter(const int64_t n, const float64_t p);

// add adds a key to the Filter.
void add(Filter *f, const char* key, const size_t key_len);

// has returns true if the key probably exists in the Filter.
bool has(Filter *f, const char* key, const size_t key_len);

// free_filter frees f.
void free_filter(Filter *f);
```

## License
CC0 1.0 Universal