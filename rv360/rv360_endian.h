#ifndef RV360_ENDIAN_H
#define RV360_ENDIAN_H

// Portable endian-safe helpers for the 360 port.
// Include from Xbox/Src files only under _XBOX360 first, then roll out.
// No xtl.h / xdk headers required — compiles on PC too for testing.

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(_XBOX360) || defined(__PPC__) || defined(_PPC_)
#define RV360_BIG_ENDIAN 1
#else
#define RV360_LITTLE_ENDIAN 1
#endif

static inline uint16_t rv_swap16(uint16_t v) { return (uint16_t)((v << 8) | (v >> 8)); }

static inline uint32_t rv_swap32(uint32_t v) {
    return ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) << 8) |
           ((v & 0x00FF0000u) >> 8) | ((v & 0xFF000000u) >> 24);
}

static inline uint64_t rv_swap64(uint64_t v) {
    return ((v & 0xFFull) << 56) | ((v & 0xFF00ull) << 40) |
           ((v & 0xFF0000ull) << 24) | ((v & 0xFF000000ull) << 8) |
           ((v & 0xFF00000000ull) >> 8) | ((v & 0xFF0000000000ull) >> 24) |
           ((v & 0xFF000000000000ull) >> 40) | ((v & 0xFF00000000000000ull) >> 56);
}

static inline float rv_swapf(float v) {
    uint32_t t; memcpy(&t, &v, 4); t = rv_swap32(t); memcpy(&v, &t, 4); return v;
}

// Read helpers: file data on disk is LITTLE-endian (x86 authored).
// On 360 (big-endian) these swap; on PC they are plain fread.
static inline int rv_fread_u16le(uint16_t *out, FILE *fp) {
    if (fread(out, 2, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    *out = rv_swap16(*out);
#endif
    return 1;
}

static inline int rv_fread_u32le(uint32_t *out, FILE *fp) {
    if (fread(out, 4, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    *out = rv_swap32(*out);
#endif
    return 1;
}

static inline int rv_fread_f32le(float *out, FILE *fp) {
    if (fread(out, 4, 1, fp) != 1) return 0;
#ifdef RV360_BIG_ENDIAN
    *out = rv_swapf(*out);
#endif
    return 1;
}

// Fixed-width doctrine: file/network structs must use these, never long.
typedef int32_t  rv_s32;
typedef uint32_t rv_u32;
typedef int16_t  rv_s16;
typedef uint16_t rv_u16;

// Write helpers: disk format is ALWAYS little-endian, so swap on BE writers.
static inline int rv_fwrite_u16le(uint16_t v, FILE *fp) {
#ifdef RV360_BIG_ENDIAN
    v = rv_swap16(v);
#endif
    return fwrite(&v, 2, 1, fp) == 1;
}

static inline int rv_fwrite_u32le(uint32_t v, FILE *fp) {
#ifdef RV360_BIG_ENDIAN
    v = rv_swap32(v);
#endif
    return fwrite(&v, 4, 1, fp) == 1;
}

static inline int rv_fwrite_f32le(float v, FILE *fp) {
#ifdef RV360_BIG_ENDIAN
    v = rv_swapf(v);
#endif
    return fwrite(&v, 4, 1, fp) == 1;
}

#endif
