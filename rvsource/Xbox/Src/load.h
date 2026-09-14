//-----------------------------------------------------------------------------
// File: load.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef LOAD_H
#define LOAD_H

// re-define fopen

#define fopen(_f, _m) BKK_fopen(_f, _m)

#ifdef _XBOX360
#define _wfopen(_f, _m) BKK_wfopen(_f, _m)
#endif

// prototypes

FILE *BKK_fopen(const char *filename, const char *mode);
#ifdef _XBOX360
FILE *BKK_wfopen(const wchar_t *filename, const wchar_t *mode);
#endif

#endif // LOAD_H

