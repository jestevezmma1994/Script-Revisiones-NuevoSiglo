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

#ifndef GENERIC_EIT_CHANNEL_MAPPER_H_
#define GENERIC_EIT_CHANNEL_MAPPER_H_

/**
 * @file GenericEITChannelMapper.h
 * @brief Generic EIT channel mapper class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>


/**
 * @defgroup GenericEITChannelMapper Generic EIT channel mapper class
 * @ingroup GenericEITPlugin
 * @{
 **/

/**
 * Generic EIT channel mapper class.
 **/
typedef struct GenericEITChannelMapper_ *GenericEITChannelMapper;


/**
 * Get runtime type identification object
 * representing Generic EIT channel mapper class.
 **/
extern SvType
GenericEITChannelMapper_getType(void);

/**
 * Initialize Generic EIT channel mapper instance.
 *
 * @param[in] self      Generic EIT channel mapper handle
 * @param[in] ignoreTSID @c true to ignore channels' TSID
 * @param[in] ignoreONID @c true to ignore channels' ONID
 * @param[out] errorOut error info
 * @return              self or @c NULL in case of error
 **/
extern GenericEITChannelMapper
GenericEITChannelMapperInit(GenericEITChannelMapper self,
                            bool ignoreTSID,
                            bool ignoreONID,
                            SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
