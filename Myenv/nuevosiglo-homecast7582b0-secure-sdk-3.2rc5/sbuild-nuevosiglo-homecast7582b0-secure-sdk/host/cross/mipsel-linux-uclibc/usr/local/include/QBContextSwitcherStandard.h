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

#ifndef QB_CONTEXT_SWITCHER_STANDARD_H_
#define QB_CONTEXT_SWITCHER_STANDARD_H_

/**
 * @file QBContextSwitcherStandard.h QBContextSwitcherStandard class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBContextSwitcherStandard Standard context switcher class
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * A simple implementation of the @ref QBContextSwitcher, that fades out
 * current window and then fades in new one.
 **/

#include <CUIT/Core/app.h>
#include <QBWindowContext.h>
#include <QBContextSwitcher.h>
#include <QBGlobalWindowManager.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvType.h>

/**
 * Standard context switcher class.
 **/
typedef struct QBContextSwitcherStandard_t* QBContextSwitcherStandard;

/**
 * Get runtime type identification object representing standard context switcher class.
 * @return context switcher runtime type identification object
 **/
SvType QBContextSwitcherStandard_getType(void);

/**
 * Initialize allocated QBContextSwitcherStandard instance.
 *
 * @param[in] self  standard context switcher handle
 * @param[in] app   CUIT application handle
 * @param[in] scheduler a handle to the scheduler that drives GUI thread
 * @param[in] gm    global window manager handle
 * @param[in] lm    local window manager handle
 * @param[in] switchTime how long should the switch take, in seconds
 * @return a new context switcher standard instance
 **/
QBContextSwitcherStandard QBContextSwitcherStandardInitialize(QBContextSwitcherStandard self, SvApplication app, SvScheduler scheduler, QBGlobalWindowManager gm, QBLocalWindowManager lm, float switchTime);


/**
 * @}
 **/

#endif
