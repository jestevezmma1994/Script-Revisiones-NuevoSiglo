/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_EIT_CHANNEL_MAPPER_H_
#define SV_EIT_CHANNEL_MAPPER_H_

/**
 * @file SvEITChannelMapper.h
 * @brief EIT channel mapper interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>


/**
 * @defgroup SvEITChannelMapper EIT channel mapper interface
 * @ingroup GenericEITPlugin
 * @{
 **/

/**
 * EIT channel mapper interface.
 **/
typedef const struct SvEITChannelMapper_ {
    /**
     * Create channel ID for an EIT event with given TSID:ONID:SID triple.
     *
     * @param[in] self      EIT channel mapper handle
     * @param[in] TSID      Transport Stream ID
     * @param[in] ONID      Original Network ID
     * @param[in] SID       Service ID
     * @return              created channel ID (caller is responsible for
     *                      releasing it) or @c NULL if not found
     **/
    SvValue (*createChannelID)(SvGenericObject self_,
                               unsigned int TSID,
                               unsigned int ONID,
                               unsigned int SID);
} *SvEITChannelMapper;


/**
 * Get runtime type identification object representing
 * SvEITChannelMapper interface.
 *
 * @return SvEITChannelMapper interface object
 **/
extern SvInterface
SvEITChannelMapper_getInterface(void);

/**
 * @}
 **/


#endif
