/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_OSK_EVENTS_H_
#define QB_OSK_EVENTS_H_

/**
 * @file QBOSKEvents.h On Screen Keyboard peer event classes
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/QBUIEvent.h>
#include <QBOSK/QBOSKKey.h>


/**
 * @defgroup QBOSKEvents On Screen Keyboard peer event classes
 * @ingroup QBOSK
 *
 * Peer events sent by the OSK.
 **/


/**
 * @defgroup QBOSKEvent On Screen Keyboard base event class
 * @ingroup QBOSKEvents
 * @{
 **/

/**
 * On Screen Keyboard base event class.
 * @class QBOSKEvent QBOSKEvents.h <QBOSK/QBOSKEvents.h>
 * @extends QBUIEvent
 **/
typedef struct QBOSKEvent_ *QBOSKEvent;

/**
 * QBOSKEvent class internals.
 **/
struct QBOSKEvent_ {
    /// super class
    struct QBUIEvent_ super_;
    /// OSK key referred by this event
    QBOSKKey key;
    int selectedKeyLayout; ///< set optionally - ID of layout for key from event
};

/**
 * Get runtime type identification object representing QBOSKEvent class.
 *
 * @return On Screen Keyboard event class
 **/
extern SvType
QBOSKEvent_getType(void);

/**
 * @}
 **/


/**
 * @defgroup QBOSKKeyFocusedEvent On Screen Keyboard key focused event class
 * @ingroup QBOSKEvents
 *
 * This kind of event is posted by the On Screen Keyboard widget whenever
 * focus changes from one key to another.
 *
 * @{
 **/

/**
 * On Screen Keyboard key focused event class.
 * @class QBOSKKeyFocusedEvent QBOSKEvents.h <QBOSK/QBOSKEvents.h>
 * @extends QBOSKEvent
 **/
typedef struct QBOSKKeyFocusedEvent_ *QBOSKKeyFocusedEvent;

/**
 * QBOSKKeyFocusedEvent class internals.
 **/
struct QBOSKKeyFocusedEvent_ {
    /// super class
    struct QBOSKEvent_ super_;
};

/**
 * Get runtime type identification object representing QBOSKKeyFocusedEvent class.
 *
 * @return On Screen Keyboard key focused event class
 **/
extern SvType
QBOSKKeyFocusedEvent_getType(void);

/**
 * @}
 **/


/**
 * @defgroup QBOSKKeyPressedEvent On Screen Keyboard key pressed event class
 * @ingroup QBOSKEvents
 *
 * This kind of event is posted by the On Screen Keyboard widget whenever
 * any key of the OSK is pressed.
 *
 * @{
 **/

/**
 * On Screen Keyboard key pressed event class.
 * @class QBOSKKeyPressedEvent QBOSKEvents.h <QBOSK/QBOSKEvents.h>
 * @extends QBOSKEvent
 **/
typedef struct QBOSKKeyPressedEvent_ *QBOSKKeyPressedEvent;

/**
 * QBOSKKeyPressedEvent class internals.
 **/
struct QBOSKKeyPressedEvent_ {
    /// super class
    struct QBOSKEvent_ super_;
};

/**
 * Get runtime type identification object representing QBOSKKeyPressedEvent class.
 *
 * @return On Screen Keyboard key pressed event class
 **/
extern SvType
QBOSKKeyPressedEvent_getType(void);

/**
 * @}
 **/


#endif
