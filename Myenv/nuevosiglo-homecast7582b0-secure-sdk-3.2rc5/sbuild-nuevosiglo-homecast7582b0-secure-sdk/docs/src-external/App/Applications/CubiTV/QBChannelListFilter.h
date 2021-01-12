/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_CHANNEL_LIST_FILTER_H
#define QB_CHANNEL_LIST_FILTER_H

/**
 * @file QBChannelListFilter.h Channel list filter
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvValue.h>
#include <SvCore/SvErrorInfo.h>
#include <SvEPGDataLayer/Plugins/DVBEPGChannel.h>

/**
 * @defgroup QBChannelListFilter Channel list filter
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * QBChannelListFilter class.
 *
 * This class implements channels list filter - keep channels only from specific plugin.
 * @implements SvChannelMergerFilter
 **/
typedef struct QBChannelListFilter_ *QBChannelListFilter;

/**
 * Creates new instance of channel list filter service
 *
 * @return                  Handle of channel list filter service
 */
QBChannelListFilter QBChannelListFilterCreate(void);

/**
 * Start QBChannelListFilter service. Filter keeps channels only from preferred plugin.
 *
 * @param [in] self         QBChannelListFilter handle
 * @param [in] pluginID     Preferired plugin ID
 * @param [in] pathToFile   Path to file with channels list
 **/
void QBChannelListFilterStart(QBChannelListFilter self, unsigned int pluginID, SvString pathToFile);

/**
 * Stop QBChannelListFilter service.
 *
 * @param [in] self         QBChannelListFilter handle
 **/
void QBChannelListFilterStop(QBChannelListFilter self);

/**
 * @}
 **/

#endif
