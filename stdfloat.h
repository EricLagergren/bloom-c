#pragma once

#define float32_t float

_Static_assert(sizeof(float32_t) == 4, "32 bit float wasn't 4 bytes long");

#define float64_t double

_Static_assert(sizeof(float64_t) == 8, "64 bit float wasn't 8 bytes long");