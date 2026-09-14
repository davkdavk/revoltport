#ifndef RV360_ONLINE_STUB_H
#define RV360_ONLINE_STUB_H

// Offline-first stand-ins for OG Xbox Live/matchmaking/content types that the
// 360 XDK removed (M3.4/M7). Lets headers parse and offline single-player
// build; matchmaking, content download, and Live sign-in need a real rewrite
// against XSession/XContent and are NOT provided here.

#include <xtl.h>

//--- Matchmaking attributes (OG xonline.h shape, inferred from usage) ---
#ifndef X_ATTRIBUTE_DATATYPE_INTEGER
#define X_ATTRIBUTE_DATATYPE_INTEGER 0x00100000
#define X_ATTRIBUTE_DATATYPE_STRING  0x00200000
#define X_ATTRIBUTE_DATATYPE_BLOB    0x00300000
#define X_ATTRIBUTE_ID_MASK          0x0000FFFF
#endif

typedef struct {
    DWORD dwAttributeID;
    BOOL fChanged;
    union {
        struct { ULONGLONG qwValue; } integer;
        struct { DWORD dwLength; LPWSTR lpValue; } string;
        struct { DWORD dwLength; LPBYTE pbValue; } blob;
    } info;
} XONLINE_ATTRIBUTE;

typedef struct {
    DWORD dwType;
    DWORD cbLength;
} XONLINE_ATTRIBUTE_SPEC;

//--- Content/download types (OG shape, inferred from Content.h usage) ---
typedef ULONGLONG XOFFERING_ID;

#define XCONTENT_MAX_DISPLAYNAME_LENGTH 128

typedef struct {
    ULONGLONG qwOfferingId;
    DWORD dwFlags;
    WIN32_FIND_DATA wfd;
    char szContentDirectory[MAX_PATH];
    WCHAR szDisplayName[XCONTENT_MAX_DISPLAYNAME_LENGTH];
} XCONTENT_FIND_DATA;

typedef struct {
    BOOL fOfferingIsFree;
    DWORD dwPrice;
} XONLINE_PRICE;

typedef DWORD XONLINE_OFFERING_FREQUENCY;

typedef struct {
    DWORD dwPackageSize;
    DWORD dwInstallSize;
    DWORD dwOfferingType;
    DWORD dwBitFlags;
    WORD fOfferingFlags;
    WORD wPad;
    DWORD dwRating;
    FILETIME ftActivationDate;
    XOFFERING_ID OfferingId;
    BYTE *pbTitleSpecificData;
    DWORD dwTitleSpecificData;
} XONLINEOFFERING_INFO, *PXONLINEOFFERING_INFO;

typedef struct {
    DWORD dwInstances;
    XONLINE_PRICE Price;
    DWORD dwFreeMonthsBeforeCharge;
    DWORD dwDuration;
    XONLINE_OFFERING_FREQUENCY Frequency;
    BYTE *pbDetailsBuffer;
    DWORD dwDetailsBuffer;
} XONLINEOFFERING_DETAILS;

static inline DWORD XGetDisplayBlocks(LPCSTR szDirectory) {
    (void)szDirectory;
    return 0;
}

typedef struct {
    DWORD dwOfferingType;
    DWORD dwBitFilter;
    WORD wStartingIndex;
    WORD wMaxResults;
    DWORD dwDescriptionIndex;
} XONLINEOFFERING_ENUM_PARAMS;

#endif
