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

/* App/Libraries/SvEPGDataLayer/SvEPGFTAFilter.h */

#ifndef SV_EPG_FTA_FILTER_H_
#define SV_EPG_FTA_FILTER_H_

/**
 * @file SvEPGFTAFilter.h
 * @brief EPG FTA channels filter class API
 **/

#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>


/**
 * @defgroup SvEPGFTAFilter EPG FTA channels filter class
 * @ingroup SvEPGDataLayer
 * @{
 **/

/**
 * EPG FTA channels filter class.
 *
 * SvEPGFTAFilter is a simple implementation of an EPG channel
 * filter. It filters FTA channels or scrambled channels
 * depends on initialization argument.
 *
 * @class SvEPGFTAFilter
 * @extends SvEPGBasicFilter
 * @implements SvEPGChannelFilter
 **/
typedef struct SvEPGFTAFilter_ *SvEPGFTAFilter;


/**
 * Get runtime type identification object
 * representing SvEPGFTAFilter class.
 **/
extern SvType
SvEPGFTAFilter_getType(void);

/**
 * Initialize FTA channels filter instance.
 *
 * @param[in]  self              EPG FTA channel filter handle
 * @param[in]  showFTA           @c true to show only FTA channels
 * @param[out] errorOut          error info
 * @return                       self or @c NULL in case of error
 **/
extern SvEPGFTAFilter
SvEPGFTAFilterInit(SvEPGFTAFilter self,
                   bool showFTA,
                   SvErrorInfo *errorOut);


/**
 * @}
 **/

#endif

