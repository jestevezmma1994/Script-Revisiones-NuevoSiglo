/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_SEARCH_RESULT_H_
#define QB_SEARCH_RESULT_H_

/**
 * @file QBSearchResult.h Search result connected methods and data types
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBSearchResult Search result connected methods and data types
 * @ingroup QBSearch
 * @{
 **/

/**
 * Search result class.
 *
 * @class QBSearchResult QBSearchResult.h <QBSearch/QBSearchResult.h>
 * @extends SvObject
 **/
typedef struct QBSearchResult_ {
    struct SvObject_ _super; ///< super class
    unsigned int relevance;  ///< relevance value
    SvObject obj;            ///< search result object
} *QBSearchResult;

/**
 * Get runtime type identification object representing QBSearchResult class.
 *
 * @memberof QBSearchResult
 *
 * @return Search result class type identification
 **/
SvType QBSearchResult_getType(void);

/**
 * Compare search result.
 *
 * @memberof QBSearchResult
 *
 * @param[in] prv           comparator private value (not used)
 * @param[in] objA          first object
 * @param[in] objB          second object
 * @return @c -1 if objA sorting value is higher than objB value,
 *         @c 0 if objects sorting value is equal,
 *         @c 1 if objA sorting value is lower than objB value,
 **/
int
QBSearchResultCompare(void *prv, SvObject objA, SvObject objB);

/**
 * @}
 **/

#endif /* QB_SEARCH_RESULT_H_ */
