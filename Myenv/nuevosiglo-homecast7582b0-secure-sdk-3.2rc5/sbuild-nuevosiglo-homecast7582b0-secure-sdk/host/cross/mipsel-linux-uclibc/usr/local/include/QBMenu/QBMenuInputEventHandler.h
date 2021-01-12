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

#ifndef QB_MENU_INPUT_EVENT_HANDLER_H_
#define QB_MENU_INPUT_EVENT_HANDLER_H_

/**
 * @file QBMenuInputEventHandler.h
 * @brief Menu input event handler interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <CUIT/Core/types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBMenuInputEventHandler Menu input event handler interface
 * @ingroup QBMenu
 * @{
 *
 * An interface for synchronous handling of QBMenu input events.
 **/

/**
 * Menu input event handler interface.
 **/
typedef const struct QBMenuInputEventHandler_ {
    /**
     * Notify that node in QBMenu has an incoming input event.
     *
     * @param[in] self_     handle to object implementing @ref QBMenuEventHandler
     * @param[in] node      handle to the selected tree node
     * @param[in] nodePath  path to @a node
     * @param[in] menu
     * @param[in] event     incoming input event
     * @return              @c true if event has been handled, @c false otherwise
     **/
    bool (*handleInputEvent)(SvObject self_,
                             SvObject node,
                             SvObject nodePath,
                             SvObject menu,
                             SvInputEvent event);
} *QBMenuInputEventHandler;

/**
 * Get runtime type identification object representing
 * QBMenuInputEventHandler interface.
 *
 * @return QBMenuInputEventHandler interface object
 **/
extern SvInterface
QBMenuInputEventHandler_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
