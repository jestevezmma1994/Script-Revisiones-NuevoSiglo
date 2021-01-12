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

#ifndef QB_FS_READER_TYPES_H_
#define QB_FS_READER_TYPES_H_

/**
 * @file QBFSTypes.h
 * @brief File system entry types
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @ingroup QBFSEntry
 * @{
 **/


/**
 * Type of file system entry.
 **/
typedef enum {
    QBFSEntryType_unknown = 0,
    QBFSEntryType_empty,
    QBFSEntryType_directory,
    QBFSEntryType_regularFile,
} QBFSEntryType;

/**
 * Type of file.
 **/
typedef enum {
    QBFSFileType_unsupported = 0,
    QBFSFileType_video,
    QBFSFileType_audio,
    QBFSFileType_image,
    QBFSFileType_playlist,
} QBFSFileType;

/**
 * Abstract file system entry class.
 **/
typedef struct QBFSEntry_ *QBFSEntry;

/**
 * Directory class, inherits from QBFSEntry.
 **/
typedef struct QBFSDir_ *QBFSDir;

/**
 * Regular file class, inherits from QBFSEntry.
 **/
typedef struct QBFSFile_ *QBFSFile;

/**
 * Entry containing no data - dummy node used for example for "loading" stub,
 * inherits from QBFSEntry.
 **/
typedef struct QBFSEmptyEntry_ *QBFSEmptyEntry;


/**
 * @}
 **/

#endif
