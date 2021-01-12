/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_WINDOW_CONTEXT_INPUT_EVENT_HANDLER_H_
#define QB_WINDOW_CONTEXT_INPUT_EVENT_HANDLER_H_

/**
 * @file QBWindowContextInputEventHandler.h QBWindowContextInputEventHandler interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <QBInput/QBInputEvent.h>
#include <stdbool.h>

/**
 * @defgroup QBWindowContextInputEventHandler Window context input event handler interface
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * An interface for window context implementations that react to global input events.
 **/

/**
 * QBWindowContextInputEventHandler interface.
 **/
typedef const struct QBWindowContextInputEventHandler_t {
    /**
     * Notifies that window got an input event.
     *
     * @param[in] w   window context handle
     * @param[in] ev  event object
     **/
    bool (*handleInputEvent)(SvObject w, const QBInputEvent *ev);
} *QBWindowContextInputEventHandler;

/**
 * Get runtime type identification object representing window context input event handler interface.
 * @return Window context input event handler interface identification object
 **/
SvInterface QBWindowContextInputEventHandler_getInterface(void);

/**
 * @}
 **/

#endif /* QB_WINDOWCONTEXTINPUTEVENTHANDLER_H_ */
