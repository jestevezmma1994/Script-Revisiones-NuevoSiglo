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

#ifndef QB_FS_READER_FILE_H_
#define QB_FS_READER_FILE_H_

/**
 * @file QBFSFile.h
 * @brief Regular file class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBFSFile Regular file class
 * @ingroup QBFSReader
 * @{
 **/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBFSEntry.h>


/**
 * Get runtime type identification object representing
 * type of regular file class.
 **/
extern SvType
QBFSFile_getType(void);

/**
 * Create file system entry representing regular file.
 *
 * @param[in] parent        parent directory handle
 * @param[in] name          file name
 * @param[in] stats         file information
 * @param[out] errorOut     error info
 * @return                  created file object, @c NULL in case of error
 **/
extern QBFSFile
QBFSFileCreate(QBFSDir parent,
               SvString name,
               const struct stat *const stats,
               SvErrorInfo *errorOut);

/**
 * Get file size.
 *
 * @param[in] self          regular file handle
 * @return                  file size in bytes, @c -1 in case of error
 **/
extern off_t
QBFSFileGetSize(QBFSFile self);

/**
 * Get last modification time.
 *
 * @param[in] self          regular file handle
 * @return                  last modification time, @c -1 in case of error
 **/
extern time_t
QBFSFileGetLastModificationTime(QBFSFile self);

/**
 * Get file's MIME type.
 *
 * @param[in] self          regular file handle
 * @return                  MIME type string, @c NULL if unknown
 **/
extern QBFSFileType
QBFSFileGetFileType(QBFSFile self);

/**
 * @}
 **/


#endif
