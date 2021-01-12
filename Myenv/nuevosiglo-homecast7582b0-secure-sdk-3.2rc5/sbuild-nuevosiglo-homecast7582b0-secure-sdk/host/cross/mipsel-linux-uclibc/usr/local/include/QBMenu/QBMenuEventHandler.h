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

#ifndef QB_MENU_EVENT_HANDLER_H_
#define QB_MENU_EVENT_HANDLER_H_

/**
 * @file QBMenuEventHandler.h
 * @brief Menu event handler interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBMenuEventHandler Menu event handler interface
 * @ingroup QBMenu
 * @{
 *
 * An interface for synchronous handling of QBMenu 'item selected' events.
 **/

/**
 * Menu event handler interface.
 **/
typedef const struct QBMenuEventHandler_ {
    /**
     * Notify that node in QBMenu have been selected (clicked).
     *
     * @param[in] self_     handle to object implementing @ref QBMenuEventHandler
     * @param[in] node      handle to the selected tree node
     * @param[in] nodePath  path to @a node
     * @return              @c true if event has been handled, @c false otherwise
     **/
    bool (*nodeSelected)(SvObject self_,
                         SvObject node,
                         SvObject nodePath);
} *QBMenuEventHandler;

/**
 * Get runtime type identification object representing
 * QBMenuEventHandler interface.
 *
 * @return QBMenuEventHandler interface object
 **/
extern SvInterface
QBMenuEventHandler_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
