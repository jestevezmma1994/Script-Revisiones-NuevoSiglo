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

#ifndef QB_CONTENT_FILTER_H_
#define QB_CONTENT_FILTER_H_

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * Content filter parameters wrapper.
 **/
struct QBContentFilterAttributeConstraintParams_s {
    /// parent class
    struct SvObject_ super_;
    /// attribute name
    SvString name;
    /// attribute value
    SvObject value;
    /// filtering method - @c true filter expects attribute values of
    /// two objects to be different, @c false - to be equal
    bool reverse;
};
typedef struct QBContentFilterAttributeConstraintParams_s* QBContentFilterAttributeConstraintParams;

/**
 * Create content filter attribute constraint parameters class object.
 *
 * @param[out] errorOut     error info
 * @return                  @c NULL in case of failure,
 *                          handle to QBContentFilterAttributeConstraintParams object in case of success
 **/
QBContentFilterAttributeConstraintParams
QBContentFilterAttributeConstraintParamsCreate(SvErrorInfo *errorOut);

/**
 * Content filter class.
 * Content filter enables filtering objects from a datasource
 * using keywords and attribute constraints.
 **/
typedef struct QBContentFilter_ *QBContentFilter;

/**
 * Get content filter type definition.
 *
 * @return      content filter type definition
 **/
SvType
QBContentFilter_getType(void);

/**
 * Set filtering keyword for content filter.
 *
 * @param[in]  self          content filter handle
 * @param[in]  keyword       keyword used for filtering
 * @param[out] errorOut      error info
 * @return                   @c 0, @c -1 in case of error
 **/
int
QBContentFilterSetKeyword(QBContentFilter self, SvString keyword, SvErrorInfo *errorOut);

/**
 * Get filtering keyword for content filter.
 *
 * @param[in]  self          content filter handle
 * @param[out] errorOut      error info
 * @return                   keyword used for filtering, @c NULL in case of error
 **/
SvString
QBContentFilterGetKeyword(QBContentFilter self, SvErrorInfo *errorOut);

/**
 * Add object attribute constraint to content filter.
 *
 * @param[in]  self          content filter handle
 * @param[in]  params        filter parameters
 * @param[out] errorOut      error info
 * @return                   @c 0, @c -1 in case of error
 **/
int
QBContentFilterAddAttributeConstraint(QBContentFilter self, QBContentFilterAttributeConstraintParams params, SvErrorInfo *errorOut);

/**
 * Remove object attribute constraint to content filter.
 *
 * @param[in]  self          content filter handle
 * @param[in]  name          attribute name
 * @param[out] errorOut      error info
 * @return                   @c 0, @c -1 in case of error
 **/
int
QBContentFilterRemoveAttributeConstraint(QBContentFilter self, SvString name, SvErrorInfo *errorOut);


/**
 * Check is given object is compatibile with filter's constraints.
 *
 * @param[in]  self          content filter handle
 * @param[in]  object        object handle
 * @param[out] errorOut      error info
 * @return                   @c true if compatibile, @c false otherwsie
 **/
bool
QBContentFilterCheckConstraints(QBContentFilter self, SvObject object, SvErrorInfo *errorOut);

/**
 * Check if filter has constraints.
 *
 * @param[in]  self          content filter handle
 * @return                   @c true if filter has constraints, @c false otherwsie
 **/
bool
QBContentFilterHasConstraints(QBContentFilter self);





#endif

