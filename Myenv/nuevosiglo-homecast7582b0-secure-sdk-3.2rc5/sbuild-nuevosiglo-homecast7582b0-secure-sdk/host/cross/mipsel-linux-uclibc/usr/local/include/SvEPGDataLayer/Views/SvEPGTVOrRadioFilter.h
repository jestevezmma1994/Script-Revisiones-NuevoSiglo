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

/* App/Libraries/SvEPGDataLayer/SvEPGTVOrRadioFilter.h */

#ifndef SV_EPG_TV_OR_RADIO_FILTER_H_
#define SV_EPG_TV_OR_RADIO_FILTER_H_

/**
 * @file SvEPGTVFilter.h
 * @brief EPG TV or radio Channel Filter Class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGTVOrRadioFilter EPG alphabetic channel filter class
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <SvEPGDataLayer/SvEPGChannelFilter.h>

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvCore/SvErrorInfo.h>


/**
 * EPG alphabetic channel filter class.
 *
 * SvEPGTVOrRadioFilter is a simple implementation of an EPG channel
 * filter. It matches TV and radio channels and provides simple
 * sorting method based on case-insensitive dictionary ordering
 * of channel names.
 **/
typedef struct SvEPGTVOrRadioFilter_t *SvEPGTVOrRadioFilter;
struct SvEPGTVOrRadioFilter_t {
    struct SvObject_ super_;
};


/**
 * Get runtime type identification object
 * representing SvEPGTVOrRadioFilter class.
 **/
extern SvType
SvEPGTVOrRadioFilter_getType(void);

/**
 * Initialize SvEPGTVOrRadioFilter instance.
 *
 * @param[in] self      EPG alphabetic channel filter handle
 * @param[out] errorOut error info
 * @return              self or NULL in case of error
 **/
extern SvEPGTVOrRadioFilter
SvEPGTVOrRadioFilterInit(SvEPGTVOrRadioFilter self,
                         SvErrorInfo *errorOut);


/**
 * @}
 **/

#endif
