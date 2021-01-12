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

/* App/Libraries/SvEPGDataLayer/SvEPGAlphabeticFilter.h */

#ifndef SV_EPG_ALPHABETIC_FILTER_H_
#define SV_EPG_ALPHABETIC_FILTER_H_

/**
 * @file SvEPGAlphabeticFilter.h
 * @brief EPG Alphabetic Channel Filter Class API.
 **/

/**
 * @defgroup SvEPGAlphabeticFilter EPG alphabetic channel filter class
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>
#include <SvEPGDataLayer/SvEPGChannelFilter.h>
#include <stdbool.h>


/**
 * EPG alphabetic channel filter class.
 *
 * SvEPGAlphabeticFilter is a simple implementation of an EPG channel
 * filter. It does not filter out any channels, and provides simple
 * sorting method based on case-insensitive dictionary ordering
 * of channel names.
 **/
typedef struct SvEPGAlphabeticFilter_ {
   struct SvObject_ super_;
   SvGenericObject baseFilter;
   SvEPGChannelFilter baseFilterMethods;
   bool descending;
} *SvEPGAlphabeticFilter;


/**
 * Get runtime type identification object
 * representing SvEPGAlphabeticFilter class.
 **/
extern SvType
SvEPGAlphabeticFilter_getType(void);

/**
 * Initialize SvEPGAlphabeticFilter instance.
 *
 * @param[in] self      EPG alphabetic channel filter handle
 * @param[in] descending true for descending channel order,
 *                      false for ascending
 * @param[out] errorOut error info
 * @return              self or NULL in case of error
 **/
extern SvEPGAlphabeticFilter
SvEPGAlphabeticFilterInit(SvEPGAlphabeticFilter self,
                          bool descending,
                          SvErrorInfo *errorOut);

/**
 * Register base filter for alphabetic sorting filter.
 *
 * This method registers another filter, that will be used by
 * alphabetic filter for checking which channels should be accepted.
 * Comparing channels will still be done by alphabetic filter.
 *
 * @param[in] self      EPG alphabetic channel filter handle
 * @param[in] baseFilter base filter handle
 * @param[out] errorOut error info
 **/
extern void
SvEPGAlphabeticFilterSetBaseFilter(SvEPGAlphabeticFilter self,
                                   SvGenericObject baseFilter,
                                   SvErrorInfo *errorOut);


/**
 * @}
 **/

#endif
