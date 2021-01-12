/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef XHTML_WINDOWS_PROVIDER_H_
#define XHTML_WINDOWS_PROVIDER_H_

#include <QBXHTMLWidgets/QBXHTMLWindowsClient.h>
#include <QBDataModel3/QBActiveArray.h>
#include <QBDataModel3/QBActiveTree.h>
#include <SvHTTPClient/SvSSLParams.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvCore/SvErrorInfo.h>
#include <QBSmartCardMonitor.h>
#include <stdlib.h>

/**
 * @file QBXHTMLWindowsProvider.h
 * @brief QBXHTML windows provider class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * QBXHTMLWindowsProvider provider class. Provider retrieves list
 * of XHTMLWindows and mounts in main menu. Each window contains
 * url to XHTML data.
 **/
typedef struct QBXHTMLWindowsProvider_ *QBXHTMLWindowsProvider;

/**
 * Get runtime type identification object representing
 * type of QBXHTMLWindowsProvider class.
 * @return QBXHTMLWindowsProvider runtime type identification object
 **/
SvType
QBXHTMLWindowsProvider_getType(void);

/**
 * Create QBXHTMLWindowsProvider provider.
 *
 * @param[in] sslParams           parameters for ssl connection
 * @param[in] smartCardMonitor    smatrCard monitor handle
 * @param[in] windowsItems        handle to a array to put list of windows
 * @param[in] serviceID           MW service ID
 * @param[in] url                 address of server which provides list of windows
 * @param[in] xhtmlWindowsClient  XHTMLWindowsClient handle
 * @param[out] errorOut           error info
 * @return                        created provider, @c NULL in case of error
 **/
QBXHTMLWindowsProvider
QBXHTMLWindowsProviderCreate(SvSSLParams sslParams,
                             QBSmartCardMonitor smartCardMonitor,
                             QBActiveArray windowsItems,
                             SvString serviceID,
                             SvString url,
                             QBXHTMLWindowsClient xhtmlWindowsClient,
                             SvErrorInfo *errorOut);

#endif
