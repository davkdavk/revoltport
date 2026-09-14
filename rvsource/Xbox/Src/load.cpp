//-----------------------------------------------------------------------------
// File: load.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef WIN32_LEAN_AND_MEAN  //<-- $ADDITION
#define WIN32_LEAN_AND_MEAN
#endif //WIN32_LEAN_AND_MEAN //<-- $ADDITION
#ifndef _PC   //<-- $ADDITION
#define _PC
#endif //_PC  //<-- $ADDITION

//$MODIFIED
//#include <windows.h>
#include <xtl.h>
//$END_MODIFICATIONS
#include <stdio.h>

#ifdef _XBOX360
static const char *RV360_MapPath(const char *filename, char mapped[MAX_PATH])
{
    if (!filename) return filename;
    // Legacy D:/T:/N: roots become the XEX directory on an RGH title.
    if ((filename[0] == 'D' || filename[0] == 'd' ||
         filename[0] == 'T' || filename[0] == 't' ||
         filename[0] == 'N' || filename[0] == 'n') &&
        filename[1] == ':' && (filename[2] == '\\' || filename[2] == '/')) {
        _snprintf(mapped, MAX_PATH, "game:\\%s", filename + 3);
        mapped[MAX_PATH - 1] = 0;
        return mapped;
    }
    return filename;
}
#endif

extern char LastFile[];
extern void WriteLogEntry(char *s);
extern char *DBG_LogFile;

/////////////////
// open a file //
/////////////////

FILE *BKK_fopen(const char *filename, const char *mode)
{
#ifdef _XBOX360
    char mapped[MAX_PATH];
    filename = RV360_MapPath(filename, mapped);
#endif
    FILE *fp = fopen(filename, mode);

    if (_stricmp(filename, DBG_LogFile)) //$MODIFIED: changed stricmp to _stricmp
    {
        char buf[MAX_PATH];
        sprintf(buf, "Loading: %s: %s\n", filename, fp ? "Found" : "Not Found");
        WriteLogEntry(buf);
    }

    strncpy(LastFile, filename, MAX_PATH);

    return fp;
}

#ifdef _XBOX360
FILE *BKK_wfopen(const wchar_t *filename, const wchar_t *mode)
{
    char narrow_filename[MAX_PATH];
    char narrow_mode[16];
    size_t i;
    for (i = 0; i + 1 < MAX_PATH && filename[i]; i++)
        narrow_filename[i] = (char)filename[i];
    narrow_filename[i] = 0;
    for (i = 0; i + 1 < sizeof(narrow_mode) && mode[i]; i++)
        narrow_mode[i] = (char)mode[i];
    narrow_mode[i] = 0;
    return BKK_fopen(narrow_filename, narrow_mode);
}
#endif

