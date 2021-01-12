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

#ifndef QB_FS_READER_DIR_H_
#define QB_FS_READER_DIR_H_

/**
 * @file QBFSDir.h
 * @brief Directory class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBFSDir Directory class
 * @ingroup QBFSReader
 * @{
 **/

#include <stdlib.h>
#include <sys/stat.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBFSEntry.h>


/**
 * Get runtime type identification object representing
 * type of directory class.
 **/
extern SvType
QBFSDir_getType(void);

/**
 * Create file system entry representing a directory.
 *
 * @param[in] parent        parent directory handle, @c NULL for root directory
 * @param[in] name          directory name
 * @param[in] level         level of the directory in a file system tree
 * @param[out] errorOut     error info
 * @return                  created directory object, @c NULL in case of error
 **/
extern QBFSDir
QBFSDirCreate(QBFSDir parent,
              SvString rootDirPath,
              SvString name,
              unsigned int level,
              SvErrorInfo *errorOut);

/**
 * Get level of a directory in a file system tree.
 *
 * @param[in] self          directory handle
 * @return                  directory level, @c -1 in case of error
 **/
extern int
QBFSDirGetLevel(QBFSDir self);

/**
 * Get number of entries in a directory.
 *
 * @param[in] self          directory handle
 * @return                  number of entries, @c -1 in case of error
 **/
extern ssize_t
QBFSDirGetEntriesCount(QBFSDir self);

/**
 * Get directory entry at given index.
 *
 * @param[in] self          directory handle
 * @param[in] idx           index of directory entry to get
 * @return                  entry at given index, @c NULL if unavailable
 **/
extern QBFSEntry
QBFSDirGetEntryAtIndex(QBFSDir self,
                       size_t idx);

/**
 * Check if directory is completely read
 *
 * @param[in] self          directory handle
 * @return                  bool indicating copletion of directory reading
 **/
extern bool
QBFSDirIsCompleted(QBFSDir self);

/**
 * Finishes reading of direcory, after calling this function directory
 * is made completely read
 *
 * @param[in] self          directory handle
 **/
extern void
QBFSDirReadingEnded(QBFSDir self);

/**
 * Returns QBFSEntry object that follows given entry
 *
 * @param[in] self          directory handle
 * @param[in] current       entry that precedes QBFSEntry we're looking for
 * @return                  QBFSEntry that follows current
 */
extern SvObject
QBFSDirGetNextEntry(QBFSDir self, QBFSEntry current);

/**
 * Returns QBFSEntry object that precedes given entry
 *
 * @param[in] self          directory handle
 * @param[in] current       entry that follows QBFSEntry we're looking for
 * @return                  QBFSEntry that precedes current
 */
extern SvObject
QBFSDirGetPreviousEntry(QBFSDir self, QBFSEntry current);

/**
 * @}
 **/

#endif
