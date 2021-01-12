/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBDLNAIMAGEBROWSERCONTROLLER_H_
#define QBDLNAIMAGEBROWSERCONTROLLER_H_

/**
 * @file QBDLNAImageBrowserController.h
 * @brief File system controller for QBImageBrowser
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include "main_decl.h"


/**
 * @defgroup QBDLNAImageBrowserController DLNA image browser controller class
 * @ingroup CubiTV_windows_imageBrowser_controllers
 * @{
 **/

/**
 * DLNA image browser controller class.
 * @extends SvObject
 * @implements QBImageBrowserController
 **/
typedef struct QBDLNAImageBrowserController_ *QBDLNAImageBrowserController;

/**
 * Create object of DLNA image browser controller class.
 * @param[in] appGlobals  application globals
 * @param[out] errorOut   error info
 * @return DLNA image browser object
 */
extern QBDLNAImageBrowserController QBDLNAImageBrowserControllerCreate(AppGlobals appGlobals, SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif /* QBDLNAIMAGEBROWSERCONTROLLER_H_ */
