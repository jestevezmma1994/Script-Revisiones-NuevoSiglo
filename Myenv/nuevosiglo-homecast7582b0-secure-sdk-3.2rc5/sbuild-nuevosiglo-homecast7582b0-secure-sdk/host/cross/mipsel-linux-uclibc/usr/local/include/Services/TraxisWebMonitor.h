/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef TRAXIS_WEB_MONITOR_H_
#define TRAXIS_WEB_MONITOR_H_

/**
 * @file TraxisWebMonitor.h
 * @brief Traxis.Web monitor service
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <TraxisWebClient/TraxisWebSessionManager.h>
#include <Services/core/QBMiddlewareManager.h>

/**
 * @defgroup TraxisWebMonitor Traxis.Web monitor class
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * Traxis.Web monitor class.
 * @class TraxisWebMonitor
 * @extends SvObject
 **/
typedef struct TraxisWebMonitor_ *TraxisWebMonitor;

/**
 * Create Traxis.Web monitor.
 *
 * @memberof TraxisWebMonitor
 *
 * @param[in] middlewareManager middleware manager handle
 * @param[in] sessionManager Traxis.Web session manager handle
 * @param[out] errorOut error info
 * @return created Traxis.Web monitor, @c NULL in case of error
 **/
extern TraxisWebMonitor
TraxisWebMonitorCreate(QBMiddlewareManager middlewareManager,
                       TraxisWebSessionManager sessionManager,
                       SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
