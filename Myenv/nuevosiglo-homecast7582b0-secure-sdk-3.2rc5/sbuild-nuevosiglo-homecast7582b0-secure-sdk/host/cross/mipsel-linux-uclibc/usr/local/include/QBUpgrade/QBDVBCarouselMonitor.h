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

#ifndef QB_DVB_CAROUSEL_MONITOR_H_
#define QB_DVB_CAROUSEL_MONITOR_H_

/**
 * @file QBDVBCarouselMonitor.h
 * @brief DVB carousel upgrade monitor class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDVBCarouselMonitor DVB carousel upgrade monitor class
 * @ingroup QBUpgrade
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBUpgrade/QBUpgradeService.h>
#include <QBUpgrade/QBDVBCarouselDemuxer.h>
#include <QBUpgrade/QBDVBUpgradeSelector.h>


/**
 * DVB carousel upgrade monitor class.
 * @class QBDVBCarouselMonitor
 * @extends SvObject
 **/
typedef struct QBDVBCarouselMonitor_ *QBDVBCarouselMonitor;


/**
 * Get runtime type identification object
 * representing QBDVBCarouselMonitor class.
 *
 * @return QBDVBCarouselMonitor type identification object
 **/
extern SvType
QBDVBCarouselMonitor_getType(void);

/**
 * Initialize DVB carousel upgrade monitor.
 *
 * @memberof QBDVBCarouselMonitor
 *
 * @param[in] self      DVB carousel upgrade monitor handle
 * @param[in] service   upgrade service handle
 * @param[in] demuxer   DVB carousel demultiplexer handle
 * @param[in] selector  upgrade selector values
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBDVBCarouselMonitor
QBDVBCarouselMonitorInit(QBDVBCarouselMonitor self,
                         QBUpgradeService service,
                         QBDVBCarouselDemuxer demuxer,
                         const QBDVBUpgradeSelector *const selector,
                         SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif