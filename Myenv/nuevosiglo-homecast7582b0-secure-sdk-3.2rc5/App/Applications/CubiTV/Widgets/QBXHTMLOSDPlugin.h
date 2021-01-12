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


#ifndef QB_XHTML_OSD_PLUGIN_H
#define QB_XHTML_OSD_PLUGIN_H

/**
 * @file QBXHTMLOSDPlugin.h
 * @brief XHTML plugin for OSD.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvFoundation/SvCoreTypes.h>
#include <main_decl.h>

/**
 * @defgroup QBXHTMLOSDPlugin plugin for OSD
 * @ingroup CubiTV_widgets
 * @{
 *
 */

/**
 * XHTML Plugin for OSD.
 * QBHTMLOSD widget will display xhtmlWindow as a background in OSD.
 *
 * @class QBXHTMLOSDPlugin
 * @implements TVOSDPluginInterface
 **/
typedef struct QBXHTMLOSDPlugin_ *QBXHTMLOSDPlugin;

/**
 * Creates the XHTMLOSD plugin.
 *
 * @param[in] appGlobals    application data handle
 * @param[in] url           address of server which provides xhtmlWindow
 * @return                  newly created XHTMLOSD plugin handle, NULL in case of error
 **/
QBXHTMLOSDPlugin QBXHTMLOSDPluginCreate(AppGlobals appGlobals, SvString url);

/**
 * @}
 */

#endif //QB_XHTML_OSD_PLUGIN_H
