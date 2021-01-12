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

#ifndef SV_EPG_HASHTABLE_FILTER_H_
#define SV_EPG_HASHTABLE_FILTER_H_

/**
 * @file SvEPGHashTableFilter.h
 * @brief Hash table channels filter class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvEPGDataLayer/SvEPGChannelFilter.h>

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvCore/SvErrorInfo.h>


/**
 * @defgroup SvEPGHashTableFilter Hash table channels filter class
 * @ingroup SvEPGDataLayer
 * @{
 **/

/**
 * Hash table channels filter class.
 *
 * @class SvEPGHashTableFilter
 * @extends SvEPGBasicFilter
 * @implements SvEPGChannelFilter
 **/
typedef struct SvEPGHashTableFilter_ *SvEPGHashTableFilter;

/**
 * Get runtime type identification object
 * representing SvEPGHashTableFilter class.
 **/
extern SvType
SvEPGHashTableFilter_getType(void);

/**
 * Initialize hash table channels filter instance.
 *
 * @param[in] self      hash table channels filter handle
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvEPGHashTableFilter
SvEPGHashTableFilterInit(SvEPGHashTableFilter self,
                         SvErrorInfo *errorOut);

/**
 * Insert new element to the filter attributes hash table, possibly replacing
 * an old one with the same key.
 *
 * @note Just like in SvHashTableInsert(), key and value are retained and old value,
 *       if replaced, is released.
 *
 * @param[in] self      hash table channels filter handle
 * @param[in] key       element's key
 * @param[in] value     element's value
 **/
extern void
SvEPGHashTableFilterAddFilterAttribute(SvEPGHashTableFilter self,
                                       SvObject key,
                                       SvObject value);

/**
 * Insert new element to the filter attributes hash table, possibly replacing
 * an old one with the same key. Element should be of SvHashTable or SvArray type.
 * Filter will look for element under tableName and check whether it contains
 * inTableKey.
 *
 * @note Just like in SvHashTableInsert(), key and value are retained and old value,
 *       if replaced, is released.
 *
 * @param[in] self          hash table channels filter handle
 * @param[in] tableName     name of the table to be looked for in channels metadata
 * @param[in] inTableKey    key contained in @tableName to be checked for
 **/
extern void
SvEPGHashTableFilterAddTabledFilterAttribute(SvEPGHashTableFilter self,
                                             SvObject tableName,
                                             SvObject inTableKey);


/**
 * @}
 **/


#endif
