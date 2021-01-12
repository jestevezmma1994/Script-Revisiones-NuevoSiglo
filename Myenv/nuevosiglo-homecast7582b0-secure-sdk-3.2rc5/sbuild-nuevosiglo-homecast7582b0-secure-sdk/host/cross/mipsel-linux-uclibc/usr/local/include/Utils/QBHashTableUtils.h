/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely ad
** exclusively reserved to ad by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make ay other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange a appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_HASH_TABLE_UTILS_H_
#define QB_HASH_TABLE_UTILS_H_

/**
 * @file QBHashTableUtils.h
 * @brief Hash table utilities
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBHashTableUtils Hash table utilities
 * @ingroup QBUtils
 * @{
 **/

/**
 * Extract SvValue from passed description.
 *
 * Extraction will only be performed if attribute with given tag
 * is instance of SvValue.
 *
 * @param[in]  description     description
 * @param[in]  attributeTag    name of attribute to be returned
 * @param[out] errorOut        error information
 * @return                     extracted value, @c NULL in case of error
 **/
SvValue
QBHashTableUtilsGetValueAttribute(SvHashTable description, SvString attributeTag, SvErrorInfo *errorOut);

/**
 * Extract SvString from passed description.
 *
 * Extraction will only be performed if attribute with given tag
 * is instance of SvValue of string kind.
 *
 * @param[in]  description     description
 * @param[in]  attributeTag    name of attribute to be returned
 * @param[out] errorOut        error information
 * @return                     extracted string, @c NULL in case of error
 **/
SvString
QBHashTableUtilsGetStringAttribute(SvHashTable description, SvString attributeTag, SvErrorInfo *errorOut);

/**
 * Extract integer from passed description.
 *
 * Extraction will only be performed if attribute with given tag
 * is instance of SvValue of integer kind.
 *
 * @param[in]  description     description
 * @param[in]  attributeTag    name of attribute to be returned
 * @param[out] errorOut        error information
 * @return                     extracted integer, @c 0 in case of error
 **/
int
QBHashTableUtilsGetIntegerAttribute(SvHashTable description, SvString attributeTag, SvErrorInfo *errorOut);

/**
 * Extract boolean from passed description.
 *
 * Extraction will only be performed if attribute with given tag
 * is instance of SvValue of boolean kind.
 *
 * @param[in]  description     description
 * @param[in]  attributeTag    name of attribute to be returned
 * @param[out] errorOut        error information
 * @return                     extracted boolean, @c false in case of error
 **/
bool
QBHashTableUtilsGetBooleanAttribute(SvHashTable description, SvString attributeTag, SvErrorInfo *errorOut);

/**
 * Extract date time from passed description.
 *
 * Extraction will only be performed if attribute with given tag
 * is instance of SvValue of dateTime kind.
 *
 * @param[in]  description     description
 * @param[in]  attributeTag    name of attribute to be returned
 * @param[out] errorOut        error information
 * @return                     extracted date time, @c SvTimeGetZero() in case of error
 **/
SvTime
QBHashTableUtilsGetDateTimeAttribute(SvHashTable description, SvString attributeTag, SvErrorInfo *errorOut);

/**
 * Extract SvHashTable from passed description.
 *
 * @param[in]  description     description
 * @param[in]  attributeTag    name of attribute to be returned
 * @param[out] errorOut        error information
 * @return                     extracted hash table, @c NULL in case of error
 **/
SvHashTable
QBHashTableUtilsGetHashTableAttribute(SvHashTable description, SvString attributeTag, SvErrorInfo *errorOut);

/**
 * Extract SvArray from passed description.
 *
 * @param[in]  description     description
 * @param[in]  attributeTag    name of attribute to be returned
 * @param[out] errorOut        error information
 * @return                     extracted array, @c NULL in case of error
 **/
SvArray
QBHashTableUtilsGetArrayAttribute(SvHashTable description, SvString attributeTag, SvErrorInfo *errorOut);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_HASH_TABLE_H_ */
