//-----------------------------------------------------------------------------
// File: XBInput.h
//
// Desc: Input functions for Xbox controllers.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef XBINPUT_H
#define XBINPUT_H

#include <xtl.h>

#ifdef _XBOX360
// 360 XInput removed the OG analog-button array model: A/B/X/Y became
// wButtons bits, WHITE/BLACK became shoulders, triggers became bytes.
// Game code uses these names ONLY as bAnalogButtons[] indices (verified: no
// wButtons & A/B/X/Y/WHITE/BLACK uses), so restore index semantics here.
// The 360 fill in XBInput_ReadControllers maps real hardware into slots.
#ifdef XINPUT_GAMEPAD_A
#undef XINPUT_GAMEPAD_A
#endif
#ifdef XINPUT_GAMEPAD_B
#undef XINPUT_GAMEPAD_B
#endif
#ifdef XINPUT_GAMEPAD_X
#undef XINPUT_GAMEPAD_X
#endif
#ifdef XINPUT_GAMEPAD_Y
#undef XINPUT_GAMEPAD_Y
#endif
#define XINPUT_GAMEPAD_A 0
#define XINPUT_GAMEPAD_B 1
#define XINPUT_GAMEPAD_X 2
#define XINPUT_GAMEPAD_Y 3
#define XINPUT_GAMEPAD_BLACK 4
#define XINPUT_GAMEPAD_WHITE 5
#define XINPUT_GAMEPAD_LEFT_TRIGGER 6
#define XINPUT_GAMEPAD_RIGHT_TRIGGER 7
#ifndef XINPUT_GAMEPAD_MAX_CROSSTALK
#define XINPUT_GAMEPAD_MAX_CROSSTALK 30
#endif
// OG debug-flight vector removed with xgraphics; minimal math for the
// non-shipping debug flight path in input.cpp.
struct XGVECTOR3 {
    float x, y, z;
    XGVECTOR3() {}
    XGVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    XGVECTOR3(const float *v) : x(v[0]), y(v[1]), z(v[2]) {}
};
inline XGVECTOR3 operator+(const XGVECTOR3 &a, const XGVECTOR3 &b)
{ return XGVECTOR3(a.x + b.x, a.y + b.y, a.z + b.z); }
inline XGVECTOR3 operator-(const XGVECTOR3 &a, const XGVECTOR3 &b)
{ return XGVECTOR3(a.x - b.x, a.y - b.y, a.z - b.z); }
inline XGVECTOR3 operator*(float s, const XGVECTOR3 &v)
{ return XGVECTOR3(s * v.x, s * v.y, s * v.z); }
inline XGVECTOR3 &operator+=(XGVECTOR3 &a, const XGVECTOR3 &b)
{ a.x += b.x; a.y += b.y; a.z += b.z; return a; }
inline XGVECTOR3 &operator-=(XGVECTOR3 &a, const XGVECTOR3 &b)
{ a.x -= b.x; a.y -= b.y; a.z -= b.z; return a; }
#endif




//-----------------------------------------------------------------------------
// Menu input enum
//-----------------------------------------------------------------------------
enum XBInputTypeEnum 
{
    XBINPUT_NONE = -1,
    
    XBINPUT_UP = 0,
    XBINPUT_DOWN,
    XBINPUT_LEFT,
    XBINPUT_RIGHT,
    
    XBINPUT_BACK_BUTTON,
    XBINPUT_START_BUTTON,
    
    XBINPUT_A_BUTTON,
    XBINPUT_B_BUTTON,
    XBINPUT_X_BUTTON,
    XBINPUT_Y_BUTTON,
    XBINPUT_BLACK_BUTTON,
    XBINPUT_WHITE_BUTTON,
    XBINPUT_LEFT_TRIGGER,
    XBINPUT_RIGHT_TRIGGER,

    XBINPUT_NUM_INPUT_TYPES
};

// Time values for repeating input
#define XBINPUT_REPEAT_1ST_DELAY    0.5f
#define XBINPUT_REPEAT_NEXT_DELAY   0.1f




//-----------------------------------------------------------------------------
// Controller types
//-----------------------------------------------------------------------------
enum 
{ 
    XBINPUT_CONTROLLER_UNSUPPORTED=0, // Unsupported controller type
    XBINPUT_CONTROLLER_GAMEPAD,       // Controller is a gamepad
    XBINPUT_CONTROLLER_ARCADESTICK,   // Controller is an arcade stick
    XBINPUT_CONTROLLER_WHEEL,         // Controller is a steering wheel
};




//-----------------------------------------------------------------------------
// Name: struct XBINPUT_CONTROLLER
// Desc: Structure for input controllers (gamepads, steering wheels, etc.)
//-----------------------------------------------------------------------------
struct XBINPUT_CONTROLLER
{
    // Note that the first part of this structure mimics the XINPUT_STATE
    // structure so that we can conveniently call:
    //    XInputGetState( hDevice, &controller );

    DWORD   dwPacketNumber;    // Input packet number
    WORD    wButtons;          // Current state of the digital buttons
    BYTE    bAnalogButtons[8]; // Current state of the analog buttons
    SHORT   sThumbLX;          // Current state of the thumbsticks
    SHORT   sThumbLY;
    SHORT   sThumbRX;
    SHORT   sThumbRY;

    // Thumb stick values converted to range [-1,+1]
    FLOAT   fX1;
    FLOAT   fY1;
    FLOAT   fX2;
    FLOAT   fY2;
    
    // State of buttons tracked since last poll
    WORD    wLastButtons;
    BOOL    bLastAnalogButtons[8];
    WORD    wPressedButtons;
    BOOL    bPressedAnalogButtons[8];

    // Translation of input as a menu input event
    DWORD   dwMenuInput;
    DWORD   dwLastMenuInput;
    FLOAT   fTimeUntilRepeat;

/*
    // Rumble properties
    XINPUT_RUMBLE   Rumble;
    XINPUT_FEEDBACK Feedback;
*/

    // Device properties
    XINPUT_CAPABILITIES caps;             // Capability bits of the device
    HANDLE              hDevice;          // Device handle
    DWORD               dwDeviceType;     // Whether this a gamepad or wheel

/*
    // Flags for whether gamepad was just inserted or removed
    BOOL       bInserted;
    BOOL       bRemoved;
*/
};




// Global variable containing state for all controllers (max of 4)
extern XBINPUT_CONTROLLER g_Controllers[4];

// Global flag that indicates whether any input occured on any gamepad
extern BOOL g_bAnyButtonPressed;

// Global flag to control thumbstick behavior
extern BOOL g_bMapThumbstickToDpadControls;




// Function prototypes
HRESULT XBInput_InitControllers();
HRESULT XBInput_ReadControllers();
void    XBInput_GetInput();




#endif // XBINPUT_H

