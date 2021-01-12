/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SVCHANNELMERGERBASIC_H
#define SVCHANNELMERGERBASIC_H

/**
 * @file SvChannelMergerBasic.h
 * @brief Merger for channels comming from different sources
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvType.h>

/**
 * @defgroup SvChannelMergers Mergers for channels comming from different sources
 * @ingroup SvEPGDataLayer
 *
 * Basic merger exposes only channels from the selected source plugin in
 * case of the channels with the same external_channel_id attribute comming from
 * different sources.
 **/

/**
 * SvChannelMergerBasic type.
 */
struct SvChannelMergerBasic_;
typedef struct SvChannelMergerBasic_ *SvChannelMergerBasic;

/**
 * Get runtime type identification object representing SvChannelMergerBasic class.
 **/
SvType
SvChannelMergerBasic_getType(void);

/**
 * SvChannelMergerBasic initialization.
 * @param[in] self      SvChannelMergerBasic handler
 */
void
SvChannelMergerBasicInit(SvChannelMergerBasic self);

/** @} */

#endif // SVCHANNELMERGERBASIC_H
