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

#ifndef QB_XHTML_WINDOWS_SERVICE_H
#define QB_XHTML_WINDOWS_SERVICE_H

/**
 * @file QBXHTMLWindowsService.h
 * @brief QBXHTMLWindowsService service for displaying XHTML defined windows
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBXHTMLWidgets/QBXHTMLWindowsClient.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvURL.h>
#include <main_decl.h>

/**
 * @defgroup QBXHTMLWindowsService service responsible for downloading, parsing and displaying a XHTML document.
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * QBXHTMLWindowsService service class.
 *
 * @class QBXHTMLWindowsService QBXHTMLWindowsService.h <Services/QBXHTMLWindowsService.h>
 * @extends SvObject
 **/
typedef struct QBXHTMLWindowsService_ *QBXHTMLWindowsService;

/**
 * Create QBXHTMLWindowsService service.
 *
 * @public @memberof QBXHTMLWindowsService
 *
 * @param [in] appGlobals   Application global data handle
 * @param [out] errorOut    error output
 * @return                  New QBXHTMLWindowsService XHTML windows service, @c NULL in case of error
 **/
QBXHTMLWindowsService
QBXHTMLWindowsServiceCreate(AppGlobals appGlobals, SvErrorInfo *errorOut);

/**
 * Start QBXHTMLWindowsService service.
 *
 * @public @memberof QBXHTMLWindowsService
 *
 * @param [in] self     QBXHTMLWindowsService handle
 **/
void
QBXHTMLWindowsServiceStart(QBXHTMLWindowsService self);

/**
 * Stop QBXHTMLWindowsService service.
 *
 * @public @memberof QBXHTMLWindowsService
 *
 * @param [in] self     QBXHTMLWindowsService handle
 **/
void
QBXHTMLWindowsServiceStop(QBXHTMLWindowsService self);

/**
 * Display widget representing XHTML document that can be found under a given URL
 *
 * @public @memberof QBXHTMLWindowsService
 *
 * @param [in] self     QBXHTMLWindowsService handle
 * @param [in] url      URL to XHTML document
 * @param [in] id       id of the requested window
 **/
void
QBXHTMLWindowsServiceDisplay(QBXHTMLWindowsService self, SvURL url, SvString id);

/**
 * Get QBXHTMLWindowsClient.
 *
 * @public @memberof QBXHTMLWindowsService
 *
 * @param [in] self     QBXHTMLWindowsService handle
 * @return              QBXHTMLWindowsClient handle
 **/
QBXHTMLWindowsClient
QBXHTMLWindowsServiceGetXHTMLWindowsClient(QBXHTMLWindowsService self);

/**
 * @}
 **/


#endif //QB_XHTML_WINDOWS_SERVICE_H
