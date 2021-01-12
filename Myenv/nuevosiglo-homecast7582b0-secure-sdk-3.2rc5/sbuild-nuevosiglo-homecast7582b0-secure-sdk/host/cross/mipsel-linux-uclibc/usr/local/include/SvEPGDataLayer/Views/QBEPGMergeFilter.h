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

#ifndef QB_EPG_MERGE_FILTER_H_
#define QB_EPG_MERGE_FILTER_H_

/**
 * @file QBEPGMergeFilter.h
 * @brief EPG Merge Channel Filter Class API.
 **/

/**
 * @defgroup QBEPGMergeFilter EPG merge channel filter class
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <SvFoundation/SvGenericObject.h>
#include <SvFoundation/SvType.h>
#include <SvCore/SvErrorInfo.h>
#include <SvEPGDataLayer/SvEPGChannelFilter.h>


/**
 * EPG merge channel filter class.
 *
 * QBEPGMergeFilter is a implementation of an EPG channel
 * filter, merging two or more filters. It pass channels only if
 * all added filters pass them. Sorting method based on filter
 * added in init method.
 *
 * @class QBEPGMergeFilter
 * @implements SvEPGChannelFilter
 **/
typedef struct QBEPGMergeFilter_ *QBEPGMergeFilter;


/**
 * Get runtime type identification object
 * representing QBEPGMergeFilter class.
 **/
extern SvType
QBEPGMergeFilter_getType(void);

/**
 * Initialize QBEPGMergeFilter instance.
 *
 * @param[in] self       EPG merge channel filter handle
 * @param[in] baseFilter base filter handle
 * @param[out] errorOut  error info
 * @return               self or @c NULL in case of error
 **/
extern QBEPGMergeFilter
QBEPGMergeFilterInit(QBEPGMergeFilter self,
                     SvObject baseFilter,
                     SvErrorInfo *errorOut);

/**
 * Add filter to QBEPGMergeFilter.
 *
 * @param[in] self      EPG merge channel filter handle
 * @param[in] addFilter added filter handle
 * @param[out] errorOut error info
 **/
extern void
QBEPGMergeFilterAddFilter(QBEPGMergeFilter self,
                          SvObject addedFilter,
                          SvErrorInfo *errorOut);


/**
 * @}
 **/

#endif // QB_EPG_MERGE_FILTER_H_

