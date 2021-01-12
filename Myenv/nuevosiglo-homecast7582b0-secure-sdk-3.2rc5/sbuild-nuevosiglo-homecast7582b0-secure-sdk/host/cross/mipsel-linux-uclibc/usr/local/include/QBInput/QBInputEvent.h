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

#ifndef QB_INPUT_EVENT_H_
#define QB_INPUT_EVENT_H_

/**
 * @file QBInputEvent.h
 * @brief Input event API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBInputEvent Input device library events
 * @ingroup QBInputCore
 * @{
 **/

/**
 * Input event types.
 **/
typedef enum {
   QBInputEventType_invalid = 0,          ///< marks uninitialized (empty) event
   QBInputEventType_deviceAdded = 0x01,   ///< new input device registered
   QBInputEventType_deviceRemoved = 0x02, ///< input device deregistered
   QBInputEventType_keyPressed = 0x10,    ///< key pressed
   QBInputEventType_keyTyped = 0x20,      ///< key typed (synthetic event)
   QBInputEventType_keyReleased = 0x40,   ///< key released
   QBInputEventType_moveAbsolute = 0x100, ///< move to absolute coordinates
   QBInputEventType_moveRelative = 0x200  ///< move by relative offset
} QBInputEventType;


/**
 * Check if event is invalid.
 *
 * @param[in] type      event type
 * @return              @c true for invalid event, @c false otherwise
 **/
static inline bool
QBInputEventIsInvalid(int type)
{
   return (type == QBInputEventType_invalid);
}

/**
 * Check if event is of one of system event types.
 *
 * @param[in] type      event type
 * @return              @c true for system event, @c false otherwise
 **/
static inline bool
QBInputEventIsSystemEvent(int type)
{
   return (type == QBInputEventType_deviceAdded || type == QBInputEventType_deviceRemoved);
}

/**
 * Check if event is a key event.
 *
 * @param[in] type      event type
 * @return              @c true for key event, @c false otherwise
 **/
static inline bool
QBInputEventIsKeyEvent(int type)
{
   return (type >= QBInputEventType_keyPressed && type <= QBInputEventType_keyReleased);
}

/**
 * Check if event is a move event.
 *
 * @param[in] type      event type
 * @return              @c true for move event, @c false otherwise
 **/
static inline bool
QBInputEventIsMoveEvent(int type)
{
   return (type >= QBInputEventType_moveAbsolute && type <= QBInputEventType_moveRelative);
}


/**
 * Key event class.
 **/
struct QBKeyEvent_t {
   uint32_t rawCode;    ///< raw key code (before mapping/filtering)
   uint32_t code;       ///< high-level input code, after filtering
};

/**
 * Move event class.
 **/
struct QBMoveEvent_t {
   int32_t x;           ///< value in X axis
   int32_t y;           ///< value in Y axis
   int16_t pressure;    ///< pressure value (for absolute movement only)
};

/**
 * Generic input event class.
 *
 * @note The size of this structure is carefully choosen to be
 * 16 bytes, so it should be pretty efficient on most platforms
 * to pass input events by value. Don't try to add new fields
 * to this structure unless you have very, very, very good reason!
 **/
typedef struct QBInputEvent_ {
   int16_t type;        ///< input event type (QBInputEventType_* value)
   uint16_t src;        ///< input device ID this event came from
   union {
      uint32_t data[3];    ///< force total struct size and alignment
      struct QBKeyEvent_t key; ///< data specific for key events
      struct QBMoveEvent_t move; ///< data specific for movement events
   } u; ///< union of all possible input event types, QBInputEvent_::type determines which element is used
} QBInputEvent;


/**
 * Return invalid input event.
 *
 * @return              invalid input event
 **/
static inline QBInputEvent
QBInputEventGetInvalid(void)
{
   QBInputEvent ev;

   ev.type = QBInputEventType_invalid;
   ev.src = 0;
   ev.u.data[0] = ev.u.data[1] = ev.u.data[2] = 0;
   return ev;
}

/**
 * Convert input event from host byte order to little endian byte order.
 *
 * @param[in,out] event input event
 **/
extern void
QBInputEventHToLE(QBInputEvent *event);

/**
 * Convert input event from little endian byte order to host byte order.
 *
 * @param[in,out] event input event
 **/
extern void
QBInputEventLEToH(QBInputEvent *event);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
