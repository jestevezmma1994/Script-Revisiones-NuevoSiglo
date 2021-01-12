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

#ifndef SV_EPG_NUMERIC_FILTER_H_
#define SV_EPG_NUMERIC_FILTER_H_

/**
 * @file SvEPGNumericFilter.h
 * @brief Numeric channels filter class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvEPGDataLayer/SvEPGChannelFilter.h>

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>


/**
 * @defgroup SvEPGNumericFilter Numeric channels filter class
 * @ingroup SvEPGDataLayer
 * @{
 **/

/**
 * EPG numeric channels filter class.
 *
 * SvEPGNumericFilter is a simple implementation of an EPG channel
 * filter, matching channels within channel numbers range, or sequence of either ranges
 * and single channel numbers.
 * Typically used in conjunction with baseFilter: SvEPGTVOrRadioFilter, SvEPGFTAFilter, etc.
 *
 * @class SvEPGNumericFilter
 * @extends SvEPGBasicFilter
 * @implements SvEPGChannelFilter
 **/
typedef struct SvEPGNumericFilter_ *SvEPGNumericFilter;

/**
 * Get runtime type identification object
 * representing SvEPGNumericFilter class.
 **/
extern SvType
SvEPGNumericFilter_getType(void);

/**
 * Initialize SvEPGNumericFilter instance.
 *
 * @param[in] self      EPG numeric channel filter handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvEPGNumericFilter
SvEPGNumericFilterInit(SvEPGNumericFilter self,
                       SvErrorInfo *errorOut);

extern void
SvEPGNumericFilterAddRange(SvEPGNumericFilter self, int min, int max);

/**
 * Register base filter for numeric sorting filter.
 *
 * This method registers another filter, that will be used by
 * numeric filter for checking which channels should be accepted.
 * Comparing channels will still be done by numeric filter.
 *
 * @param[in] self      EPG numeric channel filter handle
 * @param[in] baseFilter base filter handle
 * @param[out] errorOut error info
 **/
extern void
SvEPGNumericFilterSetBaseFilter(SvEPGNumericFilter self,
                                SvObject baseFilter,
                                SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
