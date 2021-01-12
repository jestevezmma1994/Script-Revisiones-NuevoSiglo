/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_FS_READER_ENTRY_H_
#define QB_FS_READER_ENTRY_H_

/**
 * @file QBFSEntry.h
 * @brief File system entry class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBFSEntry File system entry class
 * @ingroup QBFSReader
 * @{
 **/

#include <stdlib.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBFSTypes.h>


/**
 * Get runtime type identification object representing
 * type of file system entry class.
 **/
extern SvType
QBFSEntry_getType(void);

extern SvType
QBFSEmptyEntry_getType(void);

extern QBFSEmptyEntry
QBFSEmptyEntryCreate(QBFSDir parent,
                     SvString name,
                     SvErrorInfo *errorOut);
/**
 * Initialize file system entry.
 *
 * @param[in] self          file system entry handle
 * @param[in] parent        parent directory handle, @c NULL for root of the tree
 * @param[in] type          entry type
 * @param[in] name          entry name (in the directory, not a full path)
 * @param[out] errorOut     error info
 * @return                  @a self or @c NULL in case of error
 **/
extern QBFSEntry
QBFSEntryInit(QBFSEntry self,
              QBFSDir parent,
              QBFSEntryType type,
              SvString rootDirPath,
              SvString name,
              SvErrorInfo *errorOut);

/**
 * Get parent directory of file system entry.
 *
 * @param[in] self          file system entry handle
 * @return                  parent directory, @c NULL in case of error
 **/
extern QBFSDir
QBFSEntryGetParent(QBFSEntry self);

/**
 * Get type of file system entry.
 *
 * @param[in] self          file system entry handle
 * @return                  entry type, ::QBFSEntryType_unknown in case of error
 **/
extern QBFSEntryType
QBFSEntryGetType(QBFSEntry self);

/**
 * Get name of file system entry.
 *
 * @param[in] self          file system entry handle
 * @return                  entry name, @c NULL in case of error
 **/
extern SvString
QBFSEntryGetName(QBFSEntry self);

/**
 * Create full path of file system entry as string.
 *
 * @param[in] self          file system entry handle
 * @return                  path string
 *                          @c NULL in case of error
 **/
extern SvString
QBFSEntryCreatePathString(QBFSEntry self);

extern SvString
QBFSEntryCreateFullPathString(QBFSEntry self);

extern SvString
QBFSEntryGetRootDirPath(QBFSEntry self);

extern void
QBFSEmptyEntrySetName(QBFSEmptyEntry self, SvString name);

/**
 * @}
 **/

#endif
