/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_INPUT_CODES_H_
#define QB_INPUT_CODES_H_

/**
 * @file QBInputCodes.h
 * @brief Definitions of input key codes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


/**
 * @defgroup QBInputCodes Definitions of input key codes
 * @ingroup QBInputEvent
 * @{
 **/

/// A macro for defining command codes.
#define QBKEY_CMD(x) ((unsigned int) 0xe000 + x)
/// A macro for defining factory command codes.
#define QBKEY_FACTORY(x) ((unsigned int) 0xe000 + x)
/// A macro for defining mouse button codes.
#define QBKEY_MOUSE(x) ((unsigned int) 0xe080 + x)
/// A macro for defining rotation wheel codes.
#define QBKEY_ROTATE(x) ((unsigned int) 0xe090 + x)
/// A macro for defining second group of command codes.
#define QBKEY_ECMD(x) ((unsigned int) 0xe080 + x)

/// Test if input code is a command code.
#define QBKEY_IS_CMD(x)    (((x) < 0x20) || ((x) >= 0xe000 && ((x) < 0xe100)))
/// Test if input code is a mouse button code.
#define QBKEY_IS_MOUSE(x)  (((x) >= 0xe080) && ((x) < 0xe090))
/// Test if input code is a rotation wheel code.
#define QBKEY_IS_ROTATE(x)  (((x) >= 0xe090) && ((x) < 0xe0a0))


// modifiers
/// Key code for Shift modifier key.
#define QBKEY_SHIFT        QBKEY_CMD(53) // QBKEY_CMD('5')
/// Key code for Meta (also known as Alt) modifier key.
#define QBKEY_META         QBKEY_CMD(55) // QBKEY_CMD('7')
/// Key code for CapsLock modifier key.
#define QBKEY_CAPSLOCK     QBKEY_CMD(57) // QBKEY_CMD('9')


// basic command codes
/// BACKSPACE key (not the same as BACK!), CTRL-H.
#define QBKEY_BS           0x08
/// TAB (horizontal tabulation) key, CTRL-I.
#define QBKEY_TAB          0x09
/// BACKTAB (reversed horizontal tabulation) key.
#define QBKEY_BACKTAB      QBKEY_ECMD('3')

// device control keys
/// STANDBY key code (passive standby).
#define QBKEY_STANDBY      QBKEY_CMD('P')
/// POWER key code.
#define QBKEY_POWER        QBKEY_CMD('Q')
/// ABORT key code.
#define QBKEY_ABORT        QBKEY_CMD('!')
/// TV key code (selects input from TV tuner).
#define QBKEY_TV           QBKEY_CMD('t')
/// VOD key code (selects VOD function).
#define QBKEY_VOD          QBKEY_CMD('v')
/// SOURCE key code (for selelecting input source).
#define QBKEY_SOURCE       QBKEY_CMD('Z')
/// ASPECT key code (changes image aspect/scaling).
#define QBKEY_ASPECT       QBKEY_CMD('e')
#define QBKEY_ZOOM         QBKEY_ASPECT

// general navigation keys
/// ENTER/OK key code.
#define QBKEY_ENTER        QBKEY_CMD(' ')
/// UP cursor key code (up arrow).
#define QBKEY_UP           QBKEY_CMD('A')
/// DOWN cursor key code (down arrow).
#define QBKEY_DOWN         QBKEY_CMD('B')
/// RIGHT cursor key code (right arrow).
#define QBKEY_RIGHT        QBKEY_CMD('C')
/// LEFT cursor key code (left arrow).
#define QBKEY_LEFT         QBKEY_CMD('D')
/// HOME key code.
#define QBKEY_HOME         QBKEY_CMD('M')
/// END key code.
#define QBKEY_END          QBKEY_ECMD('q')
/// INSERT key code.
#define QBKEY_INS          QBKEY_ECMD('D')
/// DELETE key code.
#define QBKEY_DEL          QBKEY_ECMD('C')
/// PAGEUP key code.
#define QBKEY_PGUP         QBKEY_ECMD('A')
/// PAGEDOWN key code.
#define QBKEY_PGDN         QBKEY_ECMD('B')
/// BACK key code (sometimes used as a BACKSPACE).
#define QBKEY_BACK         QBKEY_CMD('[')
/// FORWARD key code.
#define QBKEY_FORWARD      QBKEY_CMD(']')
/// MENU key code.
#define QBKEY_MENU         QBKEY_CMD('m')
/// MESSAGE key code
#define QBKEY_MESSAGE      QBKEY_CMD('g')

// extra numeric keys, common on remote controllers in Japan
// (0 to 9 keys are mapped to ASCII digits)
/// '11' key code.
#define QBKEY_11           QBKEY_CMD(':')

/// '12' key code.
#define QBKEY_12           QBKEY_CMD(';')

// audio control
/// VOL+ key code.
#define QBKEY_VOLUP        QBKEY_CMD('=')

/// VOL- key code.
#define QBKEY_VOLDN        QBKEY_CMD('-')
/// MUTE key code.
#define QBKEY_MUTE         QBKEY_CMD('\\')
/// Select audio channel/track key code.
#define QBKEY_AUDIOTRACK   QBKEY_CMD('y')

// TV control
/// CH+ key code (switches one channel up).
#define QBKEY_CHUP         QBKEY_CMD('a')
/// CH- key code (switches one channel down).
#define QBKEY_CHDN         QBKEY_CMD('z')
/// LIST key code (displays short channels list).
#define QBKEY_LIST         QBKEY_CMD('r')
/// EPG key code (displays TV Guide).
#define QBKEY_EPG          QBKEY_CMD('E')
/// INFO key code (displays OSD).
#define QBKEY_INFO         QBKEY_CMD('/')
/// SUBTITLES key code.
#define QBKEY_SUBTITLES    QBKEY_CMD('U')
/// TXT key code.
#define QBKEY_TELETEXT     QBKEY_CMD('T')
/// CLOCK key code (usually shows teletext clock).
#define QBKEY_CLOCK        QBKEY_CMD('N')

// playback control
/// PLAY key code.
#define QBKEY_PLAY         QBKEY_CMD('i')
/// PAUSE key code.
#define QBKEY_PAUSE        QBKEY_CMD('k')
/// PLAY/PAUSE key code (when they are on one button).
#define QBKEY_PLAYPAUSE    QBKEY_ECMD('i')
/// REC (record) key code.
#define QBKEY_REC          QBKEY_CMD('s')
/// STOP key code.
#define QBKEY_STOP         QBKEY_CMD('q')
/// REW (rewind) key code.
#define QBKEY_REW          QBKEY_CMD('j')
/// FFW (fast forward) key code.
#define QBKEY_FFW          QBKEY_CMD('l')
/// Step back (previous title or jump back a few seconds) key code.
#define QBKEY_STEP_BACK    QBKEY_CMD('u')
/// Step forward (next title or jump a few seconds forward) key code.
#define QBKEY_STEP_FWD     QBKEY_CMD('o')

// keys for OSK control
/// Start conversion key code.
#define QBKEY_OSK_CONVERT  QBKEY_CMD('%')
/// Change input mode key code.
#define QBKEY_OSK_INPUT    QBKEY_CMD('&')
/// Punctuation.
#define QBKEY_OSK_PUNCT    QBKEY_CMD('#')

// coloured keys, used for navigation in teletext and menus
/// RED key code.
#define QBKEY_RED          QBKEY_ECMD('v')
/// GREEN key code.
#define QBKEY_GREEN        QBKEY_ECMD('p')
/// YELLOW key code.
#define QBKEY_YELLOW       QBKEY_ECMD('c')
/// BLUE key code.
#define QBKEY_BLUE         QBKEY_ECMD('w')

// miscellaneous keys
/// VCS key code.
#define QBKEY_VCS          QBKEY_CMD('c')
/// PVR key code.
#define QBKEY_PVR          QBKEY_CMD('p')
/// WWW key code (launches web browser).
#define QBKEY_WWW          QBKEY_CMD('w')
/// SETTINGS key code.
#define QBKEY_SETTINGS     QBKEY_CMD('S')
/// FUNCTION key code.
#define QBKEY_FUNCTION     QBKEY_CMD('F')
/// LOOP key code.
#define QBKEY_LOOP         QBKEY_CMD('d')
/// BOOKMARK key code.
#define QBKEY_BOOKMARK     QBKEY_CMD('x')
/// RETURN key code.
#define QBKEY_RETURN       QBKEY_CMD('<')
/// RELOAD key code.
#define QBKEY_RELOAD       QBKEY_CMD('R')
/// CANCEL key code.
#define QBKEY_CANCEL       QBKEY_CMD('X')
/// SCREENSHOT key code.
#define QBKEY_SCREENSHOT   QBKEY_CMD('b')
/// PREV key code.
#define QBKEY_PREV         QBKEY_CMD('f')
/// NEXT key code.
#define QBKEY_NEXT         QBKEY_CMD('n')
/// FAVORITES key code.
#define QBKEY_FAVORITES    QBKEY_CMD('h')
/// HISTORY key code.
#define QBKEY_HISTORY      QBKEY_CMD('H')
/// HELP key code.
#define QBKEY_HELP         QBKEY_ECMD('h')
/// LANGUAGE key code.
#define QBKEY_LANGUAGE     QBKEY_CMD('L')
/// VOUT key code.
#define QBKEY_VOUT         QBKEY_CMD('V')
/// USB key code
#define QBKEY_USB          QBKEY_CMD('.')
/// CLEAR key code
#define QBKEY_CLEAR        QBKEY_CMD('"')
/// SEARCH key code.
#define QBKEY_SEARCH       QBKEY_ECMD('s')

// function keys
/// F0 key code
#define QBKEY_F0           QBKEY_ECMD(0x20)
/// F1 key code
#define QBKEY_F1           QBKEY_ECMD(0x21)
/// F2 key code
#define QBKEY_F2           QBKEY_ECMD(0x22)
/// F3 key code
#define QBKEY_F3           QBKEY_ECMD(0x23)
/// F4 key code
#define QBKEY_F4           QBKEY_ECMD(0x24)
/// F5 key code
#define QBKEY_F5           QBKEY_ECMD(0x25)
/// F6 key code
#define QBKEY_F6           QBKEY_ECMD(0x26)
/// F7 key code
#define QBKEY_F7           QBKEY_ECMD(0x27)
/// F8 key code
#define QBKEY_F8           QBKEY_ECMD(0x28)
/// F9 key code
#define QBKEY_F9           QBKEY_ECMD(0x29)
/// F10 key code
#define QBKEY_F10          QBKEY_ECMD(0x2a)
/// F11 key code
#define QBKEY_F11          QBKEY_ECMD(0x2b)
/// F12 key code
#define QBKEY_F12          QBKEY_ECMD(0x2c)
/// F13 key code
#define QBKEY_F13          QBKEY_ECMD(0x2d)
/// F14 key code
#define QBKEY_F14          QBKEY_ECMD(0x2e)
/// F15 key code
#define QBKEY_F15          QBKEY_ECMD(0x2f)
/// F16 key code
#define QBKEY_F16          QBKEY_ECMD(0x30)
/// F17 key code
#define QBKEY_F17          QBKEY_ECMD(0x31)
/// F18 key code
#define QBKEY_F18          QBKEY_ECMD(0x32)

// mouse buttons
/// Left mouse button.
#define QBKEY_MOUSELEFT    QBKEY_MOUSE(0)
/// Middle mouse button.
#define QBKEY_MOUSEMIDDLE  QBKEY_MOUSE(1)
/// Right mouse button.
#define QBKEY_MOUSERIGHT   QBKEY_MOUSE(2)
/// One step up of vertical mouse wheel.
#define QBKEY_MOUSEROLLUP  QBKEY_MOUSE(3)
/// One step down of vertical mouse wheel.
#define QBKEY_MOUSEROLLDN  QBKEY_MOUSE(4)
/// One step left of horizontal mouse wheel.
#define QBKEY_MOUSEROLLLT  QBKEY_MOUSE(5)
/// One step right of horizontal mouse wheel.
#define QBKEY_MOUSEROLLRT  QBKEY_MOUSE(6)

// rotation wheel steps
/// Rotate clockwise with low speed.
#define QBKEY_ROTATE_CW1   QBKEY_ROTATE(0)
/// Rotate clockwise with medium speed.
#define QBKEY_ROTATE_CW2   QBKEY_ROTATE(1)
/// Rotate clockwise with high speed.
#define QBKEY_ROTATE_CW3   QBKEY_ROTATE(2)
/// Rotate counterclockwise with low speed.
#define QBKEY_ROTATE_CCW1  QBKEY_ROTATE(4)
/// Rotate counterclockwise with medium speed.
#define QBKEY_ROTATE_CCW2  QBKEY_ROTATE(5)
/// Rotate counterclockwise with high speed.
#define QBKEY_ROTATE_CCW3  QBKEY_ROTATE(6)

// TiVo extended keys
/// TiVo Thumb Down
#define QBKEY_THUMBDN      QBKEY_CMD('G')
/// TiVo Thumb Up
#define QBKEY_THUMBUP      QBKEY_CMD('I')
/// TiVo Central
#define QBKEY_TIVO         QBKEY_CMD('\'')
/// TiVo Enter (Distinguish from Select that is mapped as QBKEY_ENTER)
#define QBKEY_TIVOENTER    QBKEY_CMD(0x2b)
/// TiVo Slow
#define QBKEY_SLOW         QBKEY_CMD(0x2a)
/// TiVo Input
#define QBKEY_INPUT        QBKEY_CMD(0x29)
/// TiVo Skip Forward
#define QBKEY_SKIPFORWARD  QBKEY_CMD(0x28)

/*
 * factory command codes: they are used to access special functions
 * (for example: extended configuration or diagnostics) so they
 * should NEVER be generated by RC delivered with the STB to customer
 */
/// Special command key code for use during production.
#define QBKEY_FACTORY1     QBKEY_FACTORY(1)
/// Special command key code for use during production.
#define QBKEY_FACTORY2     QBKEY_FACTORY(2)
/// Special command key code for use during production.
#define QBKEY_FACTORY3     QBKEY_FACTORY(3)
/// Special command key code for use during production.
#define QBKEY_FACTORY4     QBKEY_FACTORY(4)

/**
 * @}
 **/

#endif
