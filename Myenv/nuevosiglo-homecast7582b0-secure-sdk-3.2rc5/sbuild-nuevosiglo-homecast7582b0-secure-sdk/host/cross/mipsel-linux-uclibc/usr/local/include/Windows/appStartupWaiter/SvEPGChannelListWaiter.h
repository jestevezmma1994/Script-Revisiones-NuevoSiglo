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

#ifndef SVEPGCHANNELLISTWAITER_H_
#define SVEPGCHANNELLISTWAITER_H_

#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>

/**
 * @file SvEPGChannelListWaiter.h
 * @brief AppStartupWaiter's plugin watching DVB channels restoring.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGChannelListWaiter DVB channels restoring AppStartupWaiter plugin
 * @ingroup AppStartupWaiter_plugins
 * @{
 **/

/**
 * SvEPGChannelListWaiter class
 * Implements QBAppStartupWaiterPlugin interface.
 */
typedef struct SvEPGChannelListWaiter_ *SvEPGChannelListWaiter;

/**
 * Create AppStartupWaiter's plugin that watches DVB channels restoring.
 *
 * @param[in] scheduler             scheduler handle
 * @param[in] channelsTimeoutMs     DVB channels restoring timeout in [ms]
 * @param[out] errorOut             error info
 * @return  SvEPGChannelListWaiter instance, @c NULL in case of error
 */
SvEPGChannelListWaiter
SvEPGChannelListWaiterCreate(SvScheduler scheduler,
                             const int channelsTimeoutMs,
                             SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* SVEPGCHANNELLISTWAITER_H_ */