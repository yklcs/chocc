// dynarrsaur.h
// A single header C generic dynamic array ("vector") library
//                   __
//                  / _)
//      _.-------._/ /
//     / dynarrsaur /
//  __/ (  |    (  |
// /__.-'|_|-----|_|
//
// The correctness of this header is dubious at best.
// No tests have been conducted (yet).
// But hey, it seems to mostly work!
//
// Usage:
//    #include "dynarrsaur.h"
//    ...
//    typedef dynarr_t(int) dynarr_int_t;
//    dynarr_int_t v = dynarr_init();
//    dynarr_allocate(&v, 16);
//    dynarr_push(&v, 0);
//    int first = v.data[0];
//    dynarr_free(v);

#ifndef DYNARRSAUR_H
#define DYNARRSAUR_H

#pragma once

#define DYNARRSAUR_H_VERSION "too early to even version"

#include <stddef.h>
#include <stdlib.h>

#define dynarr_t(T)  \
  struct {           \
    T *data;         \
    T *data_end;     \
    size_t capacity; \
  }

#define dynarr_init() \
  { NULL, NULL, 0 }

#define dynarr_allocate(v, cap)                    \
  do {                                             \
    (v)->data = calloc((cap), sizeof(*(v)->data)); \
    (v)->data_end = (v)->data;                     \
    (v)->capacity = (cap);                         \
  } while (0)

#define dynarr_reallocate_po2(v)                                            \
  do {                                                                      \
    ptrdiff_t size = (v)->data_end - (v)->data;                             \
    (v)->data = realloc((v)->data, sizeof(*(v)->data) * (v)->capacity * 2); \
    (v)->data_end = (v)->data + size;                                       \
    (v)->capacity *= 2;                                                     \
  } while (0)

#define dynarr_is_full(v) (v)->data_end - (v)->data == (v)->capacity

#define dynarr_push(v, val)       \
  do {                            \
    if (dynarr_is_full((v))) {    \
      dynarr_reallocate_po2((v)); \
    }                             \
    *((v)->data_end++) = (val);   \
  } while (0)

#define dynarr_free(v) free(v.data)

#endif
