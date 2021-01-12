/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBSTRINGTOKENIZER_H
#define QBSTRINGTOKENIZER_H

/**
 * @file QBStringTokenizer.h
 * @brief Implementation of lazy String tokenizer class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @attention Delimiter string cannot contain non ASCII characters.
 **/

#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

/**
 * @defgroup QBStringTokenizer  String tokenizing utilities
 * @ingroup QBStringUtils
 * @{
 *
 * Java-like string tokenizer class
 **/

typedef struct QBStringTokenizer_ *QBStringTokenizer;

/**
 * Create String Tokenizer.
 *
 * @param[in] inputString       source string
 * @param[in] delimiter         string of delimiting characters (ASCII)
 * @param[out] error            error info
 * @return              created StringTokenizer,
 **/
QBStringTokenizer QBStringTokenizerCreateWithDelimiter(SvString inputString, SvString delimiter, SvErrorInfo *error);

/**
 * Get number of tokens.
 * @attention Non-lazy method
 * @param[in] self  QBStringTokenizer
 * @return  number of tokens
 **/
size_t QBStringTokenizerGetTokenCount(QBStringTokenizer self);

/**
 * Get SvIterator.
 *
 * @param[in] self   QBStringTokenizer
 * @return  iterator
 **/
SvIterator QBStringTokenizerGetIterator(QBStringTokenizer self);

/**
 * Check if delimiter was found
 *
 * @param[in] self QBStringTokenizer
 * @param[in] it   QBStringTokenizer iterator
 * @return         @c true when delimiter found on end of string, @c false otherwise
 */
bool QBStringTokenizerIsDelimiterFound(QBStringTokenizer self, SvIterator *it);

/**
 * @}
 **/

#endif // QBSTRINGTOKENIZER_H
