#ifndef RV360_ONLINE_STUB_H
#define RV360_ONLINE_STUB_H

// Offline-first stand-ins for OG Xbox Live/matchmaking/content types that the
// 360 XDK removed (M3.4/M7). Lets headers parse and offline single-player
// build; matchmaking, content download, and Live sign-in need a real rewrite
// against XSession/XContent and are NOT provided here.

#include <xtl.h>

#ifndef XONLINE_GAMERTAG_SIZE
#define XONLINE_GAMERTAG_SIZE 16
#endif

// OG task handles are opaque; void* keeps call sites parsing until the
// XSession-based task rewrite (M3.4/M7).
typedef void *XONLINETASK_HANDLE;

// OG user record removed on 360. Preserve its field shape for offline UI;
// RV360_OG_XUID converts to the scalar 360 XUID where APIs expect one.
struct RV360_OG_XUID {
    ULONGLONG qwUserID;
    DWORD dwUserFlags;
    operator XUID() const { return (XUID)qwUserID; }
};
struct XONLINE_USER {
    RV360_OG_XUID xuid;
    char szGamertag[XONLINE_GAMERTAG_SIZE];
    BYTE passcode[4];
    DWORD dwUserOptions;
    HRESULT hr;
    DWORD reserved;
};

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

//--- Friends/presence types (OG Live friends API removed on 360) ---
// Pointer-only declarations keep dependents parsing; the friends/presence
// rewrite is deferred with Live (M3.4/M7). XONLINE_FRIEND itself still exists
// in the 360 XDK (different shape), so only declare what is missing.
struct XONLINE_MUTELISTUSER;
struct XONLINE_ACCEPTED_GAMEINVITE {
    XONLINE_FRIEND InvitingFriend;
    XUID xuidAcceptedFriend;
};
typedef DWORD XONLINE_REQUEST_ANSWER_TYPE;
typedef DWORD XONLINE_GAMEINVITE_ANSWER_TYPE;
typedef DWORD XONLINE_FEEDBACK_TYPE;
#ifndef XONLINE_MAX_LOGON_USERS
#define XONLINE_MAX_LOGON_USERS 4
#endif

// OG XVoice masks became a different subsystem on 360. Values are kept only
// as menu selections while voice chat remains disabled.
typedef DWORD XVOICE_MASK;
#define XVOICE_MASK_NONE       0
#define XVOICE_MASK_DARKMASTER 1
#define XVOICE_MASK_CARTOON    2
#define XVOICE_MASK_ROBOT      3
#define XVOICE_MASK_BIGGUY     4
#define XVOICE_MASK_CHILD      5
#define XVOICE_MASK_WHISPER    6

// Offline UI constants. No request/feedback operation is sent.
#define XONLINE_REQUEST_YES             1
#define XONLINE_REQUEST_NO              2
#define XONLINE_REQUEST_BLOCK           3
#define XONLINE_GAMEINVITE_NO           0
#define XONLINE_FEEDBACK_NEG_NICKNAME   1
#define XONLINE_FEEDBACK_NEG_ATTITUDE   2
#define XONLINE_FEEDBACK_NEG_GAMEPLAY   3
#define XONLINE_FEEDBACK_POS_ATTITUDE   4
#define MAX_TITLENAME_LEN               128
#define XONLINE_NOTIFICATION_GAME_INVITE    1
#define XONLINE_NOTIFICATION_FRIEND_REQUEST 2
#define XONLINE_GAMEINVITE_YES           1
#define XONLINE_FEEDBACK_NEG_SCREAMING   5
#define XONLINE_FEEDBACK_NEG_HARASSMENT  6
#define XONLINE_FEEDBACK_NEG_LEWDNESS    7
#define XONLINE_FEEDBACK_POS_SESSION     8
#define XONLINETASK_S_RUNNING            ((HRESULT)1)
#define XONLINETASK_S_SUCCESS            S_OK
#define XONLINETASK_S_RESULTS_AVAIL      ((HRESULT)2)
#define XONLINE_MAX_STORED_ONLINE_USERS  4
#define XONLINE_OFFERING_CONTENT         1
#define XONLINE_USER_OPTION_REQUIRE_PASSCODE 1
#define XDEVICE_TYPE_MEMORY_UNIT ((VOID*)0)
#define XONLINE_PASSCODE_DPAD_UP                1
#define XONLINE_PASSCODE_DPAD_DOWN              2
#define XONLINE_PASSCODE_DPAD_LEFT              3
#define XONLINE_PASSCODE_DPAD_RIGHT             4
#define XONLINE_PASSCODE_GAMEPAD_X              5
#define XONLINE_PASSCODE_GAMEPAD_Y              6
#define XONLINE_PASSCODE_GAMEPAD_LEFT_TRIGGER   7
#define XONLINE_PASSCODE_GAMEPAD_RIGHT_TRIGGER  8
#define XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT ((HRESULT)0x80151002)
#define XONLINE_S_LOGON_USER_HAS_MESSAGE ((HRESULT)0x00151003)

typedef struct { DWORD dwReason; } LD_LAUNCH_DASHBOARD;
#define XLD_LAUNCH_DASHBOARD_NEW_ACCOUNT_SIGNUP      1
#define XLD_LAUNCH_DASHBOARD_ACCOUNT_MANAGEMENT      2
#define XLD_LAUNCH_DASHBOARD_NETWORK_CONFIGURATION   3
#define PLAUNCH_DATA(_p) ((PVOID)(_p))

typedef struct { DWORD reserved; } XONLINE_STARTUP_PARAMS;

static inline BOOL XOnlineGetNotification(DWORD, DWORD) { return FALSE; }
static inline BOOL XOnlineTitleIdIsSameTitle(DWORD) { return FALSE; }
static inline HRESULT XOnlineVerifyNickname(const WCHAR*, VOID*, XONLINETASK_HANDLE *task)
{ if (task) *task = NULL; return E_NOTIMPL; }
static inline HRESULT XOnlineTaskContinue(XONLINETASK_HANDLE) { return E_NOTIMPL; }
static inline HRESULT XOnlineTaskClose(XONLINETASK_HANDLE) { return S_OK; }
static inline HRESULT XOnlineGetUsers(XONLINE_USER*, DWORD *count)
{ if (count) *count = 0; return E_NOTIMPL; }
static inline BOOL XOnlineIsUserVoiceAllowed(DWORD) { return FALSE; }
static inline BOOL XGetDeviceChanges(VOID*, DWORD *insertions, DWORD *removals)
{ if (insertions) *insertions = 0; if (removals) *removals = 0; return FALSE; }
static inline XONLINE_USER *XOnlineGetLogonUsers(void) { return NULL; }
#define XOnlineStartup(...) E_NOTIMPL
#define XOnlineMatchSessionUpdate(...) E_NOTIMPL
#define XOnlineMatchSessionFindFromID(...) E_NOTIMPL
#define XOnlineTitleUpdate(...) E_NOTIMPL

#endif
