/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2017 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_EPG_BOUQUET_FILTER_H_
#define SV_EPG_BOUQUET_FILTER_H_

/**
 * @file SvEPGBouquetFilter.h
 * @brief Source plugin channel filter class API
 **/

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>


/**
 * @defgroup SvEPGBouquetFilter Source plugin channel filter class
 * @ingroup SvEPGDataLayer
 * @{
 **/

/**
 * Source plugin channel filter class.
 *
 * This filter accepts only channels from a single bouquet
 *
 * @class SvEPGBouquetFilter
 * @extends SvEPGBasicFilter
 * @implements SvEPGChannelFilter
 **/
typedef struct SvEPGBouquetFilter_ *SvEPGBouquetFilter;

/**
 * Get runtime type identification object
 * representing SvEPGBouquetFilter class.
 **/
extern SvType
SvEPGBouquetFilter_getType(void);

/**
 * Initialize SvEPGBouquetFilter instance.
 *
 * @param[in]  self              EPG channel source plugin filter handle
 * @param[in]  bouquetID         bouquet ID
 * @param[out] errorOut          error info
 * @return                       @a self or @c NULL in case of error
 **/
extern SvEPGBouquetFilter
SvEPGBouquetFilterInit(SvEPGBouquetFilter self,
                       unsigned int bouquetID,
                       SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
