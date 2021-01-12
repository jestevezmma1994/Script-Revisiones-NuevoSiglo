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

#ifndef QB_HTTP_UPGRADE_MONITOR_H_
#define QB_HTTP_UPGRADE_MONITOR_H_

/**
 * @file QBHTTPUpgradeMonitor.h
 * @brief HTTP upgrade monitor class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBHTTPUpgradeMonitor HTTP upgrade monitor class
 * @ingroup QBUpgrade
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <SvHTTPClient/SvHTTPClientEngine.h>
#include <QBUpgrade/QBUpgradeService.h>


/**
 * HTTP upgrade monitor class.
 * @class QBHTTPUpgradeMonitor
 * @extends SvObject
 **/
typedef struct QBHTTPUpgradeMonitor_ *QBHTTPUpgradeMonitor;


/**
 * Get runtime type identification object
 * representing QBHTTPUpgradeMonitor class.
 *
 * @return QBHTTPUpgradeMonitor type identification object
 **/
extern SvType
QBHTTPUpgradeMonitor_getType(void);

/**
 * Initialize HTTP upgrade monitor.
 *
 * @memberof QBHTTPUpgradeMonitor
 *
 * @param[in] self      HTTP upgrade monitor handle
 * @param[in] service   upgrade service handle
 * @param[in] url       HTTP, HTTPS or FTP upgrade URL
 * @param[in] imageFileName upgrade image file name
 * @param[in] boardName board name
 * @param[in] sslParams SSL parameters
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBHTTPUpgradeMonitor
QBHTTPUpgradeMonitorInit(QBHTTPUpgradeMonitor self,
                         QBUpgradeService service,
                         SvURI url,
                         SvString imageFileName,
                         SvString boardName,
                         SvSSLParams sslParams,
                         SvErrorInfo *errorOut);

/**
 * Get HTTP client engine used by HTTP upgrade monitor.
 *
 * @memberof QBHTTPUpgradeMonitor
 *
 * @param[in] self      HTTP upgrade monitor handle
 * @return              HTTP client engine used by upgrade monitor
 **/
extern SvHTTPClientEngine
QBHTTPUpgradeMonitorGetHTTPClientEngine(QBHTTPUpgradeMonitor self);

/**
 * Invoke instant HTTP check for updates.
 *
 * @memberof QBHTTPUpgradeMonitor
 *
 * @param[in] self_        HTTP upgrade monitor handle
 * @param[out] errorOut    error info
 **/
extern void
QBHTTPUpgradeMonitorCheckNow(SvObject self_,
                             SvErrorInfo *errorOut);
/**
 * @}
 **/

#endif
