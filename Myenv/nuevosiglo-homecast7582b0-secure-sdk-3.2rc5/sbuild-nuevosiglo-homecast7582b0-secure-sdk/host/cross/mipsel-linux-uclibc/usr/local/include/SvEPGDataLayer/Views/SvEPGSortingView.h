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

/* App/Libraries/SvEPGDataLayer/SvEPGSortingView.h */

#ifndef SV_EPG_SORTING_VIEW_H_
#define SV_EPG_SORTING_VIEW_H_

/**
 * @file SvEPGSortingView.h
 * @brief EPG Sorting Channel View Class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGSortingView EPG sorting channel view class
 * @ingroup SvEPGChannelViews
 * @{
 **/

#include <SvEPGDataLayer/Views/SvEPGView.h>

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvCore/SvErrorInfo.h>


/**
 * Sorting EPG channel view class.
 *
 * Sorting EPG channel view is a simple channel view, that provides
 * array of channels filtered and ordered using provided channel
 * filter implementation.
 **/
typedef struct SvEPGSortingView_ *SvEPGSortingView;


/**
 * Get runtime type identification object
 * representing SvEPGSortingView class.
 **/
extern SvType
SvEPGSortingView_getType(void);

/**
 * Initialize SvEPGSortingView instance.
 *
 * @param[in] self      sorting EPG channel view handle
 * @param[in] filter    channel filter (an object implementing
 *                      SvEPGChannelFilter inteface)
 * @param[out] errorOut error info
 * @return              self or NULL in case of error
 **/
extern SvEPGSortingView
SvEPGSortingViewInit(SvEPGSortingView self,
                     SvGenericObject filter,
                     SvEPGViewChannelNumbering numbering,
                     SvErrorInfo *errorOut);


/**
 * @}
 **/

#endif
