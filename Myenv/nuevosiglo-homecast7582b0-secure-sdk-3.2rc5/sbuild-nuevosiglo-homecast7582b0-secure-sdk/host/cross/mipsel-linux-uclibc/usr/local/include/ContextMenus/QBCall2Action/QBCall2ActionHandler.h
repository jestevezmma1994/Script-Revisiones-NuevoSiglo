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
#ifndef QBCALL2ACTION_HANDLER_H_
#define QBCALL2ACTION_HANDLER_H_

/**
 * @file QBCall2ActionHandler.h interface API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvArray.h>
#include <stdbool.h>

/**
 * @defgroup QBCall2ActionHandler Call2ActionHandler interface
 * @ingroup QBCall2Action
 * @{
 **/

/**
 * QBCall2ActionHandler is an interface to handle external logic.
 **/
typedef const struct QBCall2ActionHandler_ {
    /**
     * Notify that there is ready new pane with given options.
     *
     * @param[in] self_     handle to object implementing this interface
     * @param[in] path      path
     * @param[in] option    array of options
     * @return              @c true if handler handle this call, @c false otherwise
     **/
    bool (*onNewLevel)(SvObject self_, SvObject path, SvArray options);

    /**
     * Notify that there was option selected at pane.
     *
     * @param[in] self_     handle to object implementing this interface
     * @param[in] path      path
     * @param[in] option    selected option (QBCall2ActionTreeNode)
     * @return              @c true if handler handle this call, @c false otherwise
     **/
    bool (*onOptionSelected)(SvObject self_, SvObject path, SvObject option);
} *QBCall2ActionHandler;

/**
 * Get runtime type identification object representing
 * QBCall2ActionHandler interface.
 *
 * @return QBCall2ActionHandler interface object
 **/
SvInterface
QBCall2ActionHandler_getInterface(void);

/**
 * @}
 **/

#endif
