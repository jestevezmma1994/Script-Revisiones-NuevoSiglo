/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

// This file was generated on 2015-10-14 using omniidl
// (beeing part of omniORB_4_1) with Cubiware C backend.

/*****************************************************************************
******************************************************************************
******************************** DO NOT EDIT *********************************
******************************************************************************
******************************************************************************/

#ifndef IDL__IINPUT__
#define IDL__IINPUT__


#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>


typedef enum {
    QBRPCIInputKeyCode_0 = 0,
    QBRPCIInputKeyCode_1 = 1,
    QBRPCIInputKeyCode_2 = 2,
    QBRPCIInputKeyCode_3 = 3,
    QBRPCIInputKeyCode_4 = 4,
    QBRPCIInputKeyCode_5 = 5,
    QBRPCIInputKeyCode_6 = 6,
    QBRPCIInputKeyCode_7 = 7,
    QBRPCIInputKeyCode_8 = 8,
    QBRPCIInputKeyCode_9 = 9,
    QBRPCIInputKeyCode_a = 10,
    QBRPCIInputKeyCode_b = 11,
    QBRPCIInputKeyCode_c = 12,
    QBRPCIInputKeyCode_d = 13,
    QBRPCIInputKeyCode_e = 14,
    QBRPCIInputKeyCode_f = 15,
    QBRPCIInputKeyCode_g = 16,
    QBRPCIInputKeyCode_h = 17,
    QBRPCIInputKeyCode_i = 18,
    QBRPCIInputKeyCode_j = 19,
    QBRPCIInputKeyCode_k = 20,
    QBRPCIInputKeyCode_l = 21,
    QBRPCIInputKeyCode_m = 22,
    QBRPCIInputKeyCode_n = 23,
    QBRPCIInputKeyCode_o = 24,
    QBRPCIInputKeyCode_p = 25,
    QBRPCIInputKeyCode_q = 26,
    QBRPCIInputKeyCode_r = 27,
    QBRPCIInputKeyCode_s = 28,
    QBRPCIInputKeyCode_t = 29,
    QBRPCIInputKeyCode_u = 30,
    QBRPCIInputKeyCode_v = 31,
    QBRPCIInputKeyCode_w = 32,
    QBRPCIInputKeyCode_x = 33,
    QBRPCIInputKeyCode_y = 34,
    QBRPCIInputKeyCode_z = 35,
    QBRPCIInputKeyCode_SHIFT = 36,
    QBRPCIInputKeyCode_META = 37,
    QBRPCIInputKeyCode_CAPSLOCK = 38,
    QBRPCIInputKeyCode_BS = 39,
    QBRPCIInputKeyCode_TAB = 40,
    QBRPCIInputKeyCode_BACKTAB = 41,
    QBRPCIInputKeyCode_STANDBY = 42,
    QBRPCIInputKeyCode_POWER = 43,
    QBRPCIInputKeyCode_ABORT = 44,
    QBRPCIInputKeyCode_TV = 45,
    QBRPCIInputKeyCode_VOD = 46,
    QBRPCIInputKeyCode_SOURCE = 47,
    QBRPCIInputKeyCode_ASPECT = 48,
    QBRPCIInputKeyCode_ZOOM = 49,
    QBRPCIInputKeyCode_ENTER = 50,
    QBRPCIInputKeyCode_UP = 51,
    QBRPCIInputKeyCode_DOWN = 52,
    QBRPCIInputKeyCode_RIGHT = 53,
    QBRPCIInputKeyCode_LEFT = 54,
    QBRPCIInputKeyCode_HOME = 55,
    QBRPCIInputKeyCode_END = 56,
    QBRPCIInputKeyCode_INS = 57,
    QBRPCIInputKeyCode_DEL = 58,
    QBRPCIInputKeyCode_PGUP = 59,
    QBRPCIInputKeyCode_PGDN = 60,
    QBRPCIInputKeyCode_BACK = 61,
    QBRPCIInputKeyCode_FORWARD = 62,
    QBRPCIInputKeyCode_MENU = 63,
    QBRPCIInputKeyCode_11 = 64,
    QBRPCIInputKeyCode_12 = 65,
    QBRPCIInputKeyCode_VOLUP = 66,
    QBRPCIInputKeyCode_VOLDN = 67,
    QBRPCIInputKeyCode_MUTE = 68,
    QBRPCIInputKeyCode_AUDIOTRACK = 69,
    QBRPCIInputKeyCode_CHUP = 70,
    QBRPCIInputKeyCode_CHDN = 71,
    QBRPCIInputKeyCode_LIST = 72,
    QBRPCIInputKeyCode_EPG = 73,
    QBRPCIInputKeyCode_INFO = 74,
    QBRPCIInputKeyCode_SUBTITLES = 75,
    QBRPCIInputKeyCode_TELETEXT = 76,
    QBRPCIInputKeyCode_CLOCK = 77,
    QBRPCIInputKeyCode_PLAY = 78,
    QBRPCIInputKeyCode_PAUSE = 79,
    QBRPCIInputKeyCode_PLAYPAUSE = 80,
    QBRPCIInputKeyCode_REC = 81,
    QBRPCIInputKeyCode_STOP = 82,
    QBRPCIInputKeyCode_REW = 83,
    QBRPCIInputKeyCode_FFW = 84,
    QBRPCIInputKeyCode_STEP_BACK = 85,
    QBRPCIInputKeyCode_STEP_FWD = 86,
    QBRPCIInputKeyCode_OSK_CONVERT = 87,
    QBRPCIInputKeyCode_OSK_INPUT = 88,
    QBRPCIInputKeyCode_OSK_PUNCT = 89,
    QBRPCIInputKeyCode_RED = 90,
    QBRPCIInputKeyCode_GREEN = 91,
    QBRPCIInputKeyCode_YELLOW = 92,
    QBRPCIInputKeyCode_BLUE = 93,
    QBRPCIInputKeyCode_VCS = 94,
    QBRPCIInputKeyCode_PVR = 95,
    QBRPCIInputKeyCode_WWW = 96,
    QBRPCIInputKeyCode_SETTINGS = 97,
    QBRPCIInputKeyCode_FUNCTION = 98,
    QBRPCIInputKeyCode_LOOP = 99,
    QBRPCIInputKeyCode_BOOKMARK = 100,
    QBRPCIInputKeyCode_RETURN = 101,
    QBRPCIInputKeyCode_RELOAD = 102,
    QBRPCIInputKeyCode_CANCEL = 103,
    QBRPCIInputKeyCode_SCREENSHOT = 104,
    QBRPCIInputKeyCode_PREV = 105,
    QBRPCIInputKeyCode_NEXT = 106,
    QBRPCIInputKeyCode_FAVORITES = 107,
    QBRPCIInputKeyCode_HISTORY = 108,
    QBRPCIInputKeyCode_HELP = 109,
    QBRPCIInputKeyCode_LANGUAGE = 110,
    QBRPCIInputKeyCode_VOUT = 111,
    QBRPCIInputKeyCode_USB = 112,
    QBRPCIInputKeyCode_F0 = 113,
    QBRPCIInputKeyCode_F1 = 114,
    QBRPCIInputKeyCode_F2 = 115,
    QBRPCIInputKeyCode_F3 = 116,
    QBRPCIInputKeyCode_F4 = 117,
    QBRPCIInputKeyCode_F5 = 118,
    QBRPCIInputKeyCode_F6 = 119,
    QBRPCIInputKeyCode_F7 = 120,
    QBRPCIInputKeyCode_F8 = 121,
    QBRPCIInputKeyCode_F9 = 122,
    QBRPCIInputKeyCode_F10 = 123,
    QBRPCIInputKeyCode_F11 = 124,
    QBRPCIInputKeyCode_F12 = 125,
    QBRPCIInputKeyCode_F13 = 126,
    QBRPCIInputKeyCode_F14 = 127,
    QBRPCIInputKeyCode_F15 = 128,
    QBRPCIInputKeyCode_F16 = 129,
    QBRPCIInputKeyCode_F17 = 130,
    QBRPCIInputKeyCode_F18 = 131,
    QBRPCIInputKeyCode_MOUSELEFT = 132,
    QBRPCIInputKeyCode_MOUSEMIDDLE = 133,
    QBRPCIInputKeyCode_MOUSERIGHT = 134,
    QBRPCIInputKeyCode_MOUSEROLLUP = 135,
    QBRPCIInputKeyCode_MOUSEROLLDN = 136,
    QBRPCIInputKeyCode_MOUSEROLLLT = 137,
    QBRPCIInputKeyCode_MOUSEROLLRT = 138,
    QBRPCIInputKeyCode_ROTATE_CW1 = 139,
    QBRPCIInputKeyCode_ROTATE_CW2 = 140,
    QBRPCIInputKeyCode_ROTATE_CW3 = 141,
    QBRPCIInputKeyCode_ROTATE_CCW1 = 142,
    QBRPCIInputKeyCode_ROTATE_CCW2 = 143,
    QBRPCIInputKeyCode_ROTATE_CCW3 = 144,
    QBRPCIInputKeyCode_THUMBDN = 145,
    QBRPCIInputKeyCode_THUMBUP = 146,
    QBRPCIInputKeyCode_TIVO = 147,
    QBRPCIInputKeyCode_FACTORY1 = 148,
    QBRPCIInputKeyCode_FACTORY2 = 149,
    QBRPCIInputKeyCode_FACTORY3 = 150,
    QBRPCIInputKeyCode_FACTORY4 = 151,
} QBRPCIInputKeyCode;

typedef enum {
    QBRPCIInputKeyState_Pressed = 0,
    QBRPCIInputKeyState_Typed = 1,
    QBRPCIInputKeyState_Released = 2,
} QBRPCIInputKeyState;

typedef struct {
    QBRPCIInputKeyCode keyCode;
    QBRPCIInputKeyState keyState;
} QBRPCIInputEvent;

struct QBRPCIInput_ {
    uint16_t (*handleEvent)(SvObject self_, QBRPCIInputEvent const * event, bool* isHandled);
    uint16_t (*handleEventAsync)(SvObject self_, SvObject caller, uint16_t* callId, QBRPCIInputEvent const * event);
};
typedef const struct QBRPCIInput_ *QBRPCIInput;
#define QBRPC_IINPUT_NAME "QBRPC::IInput"

SvInterface QBRPCIInput_getInterface(void);


struct QBRPCIInputAsyncCallListener_ {
    void (*handleEventAsyncCallback)(SvObject self_, uint16_t callId, bool isHandled);
};
typedef const struct QBRPCIInputAsyncCallListener_ *QBRPCIInputAsyncCallListener;

SvInterface QBRPCIInputAsyncCallListener_getInterface(void);


#endif // IDL__IINPUT__
