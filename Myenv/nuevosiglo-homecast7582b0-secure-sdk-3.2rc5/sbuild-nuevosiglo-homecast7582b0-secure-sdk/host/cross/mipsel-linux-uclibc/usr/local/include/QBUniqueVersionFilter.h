/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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


#ifndef QBUNIQUEVERSIONFILTER_H_
#define QBUNIQUEVERSIONFILTER_H_

#include <QBTSTableParser.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file QBUniqueVersionFilter.h QBUniqueVersionFilter class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

// TODO: ingroup
/**
 * @defgroup QBTSParser QBTSParser classes
 * @{
 *
 * Unique version filter could filter out many tables defined by user.
 */

typedef struct QBSubTableIdentifier_ {
    const unsigned int byteOffset; /**< indicate for which section byte offset 'bitMask' should be applied, counted from @c 0 */
    const unsigned char bitMask; /**< mask indicating which bits should be used for filtering @c 1 means that current bit is taking into account */
} *QBSubTableIdentifier;

/**
 * Unique version filter class.
 * @class QBUniqueVersionFilter
 **/
typedef struct QBUniqueVersionFilter_t *QBUniqueVersionFilter;

/**
 * Create QBUniqueVersionFilter
 * Unique version filter could filter out many tables defined by user.
 * Each table is defined as 'tableId' plus mask defining which bits should be used as a table identifier.
 *
 * @param[in] tableParser table parser used as data source
 * @return new QBUniqueVersionFilter or @c NULL in case of error
 **/
QBUniqueVersionFilter QBUniqueVersionFilterCreate(QBTSTableParser *tableParser);

/**
 * Registers new filer defined by table ID and bit mask
 * Unique table is defined as 'tableId' both with 'tableIdentifier'.
 *
 * @param[in] self instance of @link QBUniqueVersionFilter @endlink.
 * @param[in] tableID number defining SI table identifier
 * @param[in] tableIdentifier table with byte offset and bit mask
 * @param[in] identifierBytesCount number of entries in tableIdentifier
 **/
void QBUniqueVersionFilterRegisterTableID(QBUniqueVersionFilter self, int tableID, QBSubTableIdentifier tableIdentifier, unsigned int identifierBytesCount);

/**
 * Removes filter for table identified by table ID
 *
 * @param[in] self instance of @link QBUniqueVersionFilter @endlink.
 * @param[in] tableID number defining SI table identifier
 **/
void QBUniqueVersionFilterRemoveTableID(QBUniqueVersionFilter self, int tableID);

/**
 * Sets callback. Callback will be called while new table will be received
 *
 * @param[in] self instance of @link QBUniqueVersionFilter @endlink.
 * @param[in] callbacks callback with new parsed table
 * @param[in] target callback owner
 **/
void QBUniqueVersionFilterSetCallbacks(QBUniqueVersionFilter self, const struct QBTSTableParserCallbacks_s* callbacks, void* target);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // QBUNIQUEVERSIONFILTER_H_
