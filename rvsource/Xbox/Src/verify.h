//-----------------------------------------------------------------------------
// File: verify.h
//
// Desc: content signature verification functions
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef VERIFY_H
#define VERIFY_H

#include "revolt.h"

#ifdef _XBOX360
// OG console signature API (XCalculateSignature) was removed on 360.
// Same 20-byte shape, filled by a plain checksum in verify.cpp (tamper
// evidence for local settings only, not platform security).
struct XCALCSIG_SIGNATURE { BYTE rgbSignature[20]; };
#endif


#define MAX_VERIFY_LENGTH  1024


//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
extern bool g_bSigFailure;





//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool VerifySignature(const char* szFileName);

//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool SaveSignature(const char* szFileName);


//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool GetSignature(const char* szFileName, XCALCSIG_SIGNATURE* pSig);




#endif // VERIFY_H
