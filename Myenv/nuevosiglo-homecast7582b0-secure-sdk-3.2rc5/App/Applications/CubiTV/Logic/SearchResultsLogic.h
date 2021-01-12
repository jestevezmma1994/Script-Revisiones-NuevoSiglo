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

#ifndef QBSEARCHRESULTSCONTEXTLOGIC_H
#define QBSEARCHRESULTSCONTEXTLOGIC_H

/**
 * @file SearchResultsLogic.h
 **/

/**
 * @defgroup QBSearchResults Search Results Logic
 * @ingroup CubiTV_logic
 * @{
 **/

#include <stdbool.h>
#include <CUIT/Core/types.h>
#include <QBApplicationController.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>


typedef struct QBSearchResultsLogic_t* QBSearchResultsLogic;

/**
 * Create search results logic class instance.
 *
 * @param[in] appGlobals
 * @return created search results logic class instance or null in case of error
 */
QBSearchResultsLogic QBSearchResultsLogicNew(AppGlobals appGlobals) __attribute__((weak));

/**
 * Search results logic input event handler.
 *
 * @param[in] self    search results logic handle
 * @param[in] ctx     window context handle
 * @param[in] ev      input event handle
 * @return true if event was handled, false otherwise
 */
bool QBSearchResultsLogicHandleInputEvent(QBSearchResultsLogic self, QBWindowContext ctx, SvInputEvent ev) __attribute__((weak));

/**
 * Getter of default focus position in search results context submenu.
 *
 * @param[in] self    search results logic handle
 * @return id of menu item to be focused by default
 */
SvString QBSearchResultsLogicGetDefaultSideMenuItemId(QBSearchResultsLogic self) __attribute__((weak));

/**
 * @}
 **/

#endif
