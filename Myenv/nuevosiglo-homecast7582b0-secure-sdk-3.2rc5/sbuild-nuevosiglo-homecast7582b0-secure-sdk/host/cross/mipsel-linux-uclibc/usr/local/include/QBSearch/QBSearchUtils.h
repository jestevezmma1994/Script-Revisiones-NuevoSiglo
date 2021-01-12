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

#ifndef QB_SEARCH_UTILS_H_
#define QB_SEARCH_UTILS_H_

/**
 * @file QBSearchUtils.h Search related utilities
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBSearchUtils Search related utilities
 * @ingroup QBSearch
 * @{
 **/

/** Maximum relevance of a text **/
#define QBSEARCH_MAX_RELEVANCE 1000000000 //10^9

/**
 * Create normalized string.
 *
 * This function creates normalized version of a string,
 * in current implementation it's just string with all letters in lower case
 *
 * @param[in] string                    input string
 * @param[out] *errorOut                error info
 * @return                              normalized string, @c NULL in case of error
 **/
SvString
QBSearchCreateNormalizedString(SvString string, SvErrorInfo *errorOut);

/**
 * Get relevance of a text string for given keywords.
 *
 * This function computes relevance of given text string for input string and given keywords
 *
 * @param[in] text                      text string
 * @param[in] keywords                  keywords array
 * @return                              relevance value, if it's @c 0 it means
 *                                      that at least one keyword was not found in text
 **/
unsigned int
QBSearchGetRelevance(SvString text, SvArray keywords);

/**
 * @}
 **/

#endif /* QB_SEARCH_UTILS_H_ */
