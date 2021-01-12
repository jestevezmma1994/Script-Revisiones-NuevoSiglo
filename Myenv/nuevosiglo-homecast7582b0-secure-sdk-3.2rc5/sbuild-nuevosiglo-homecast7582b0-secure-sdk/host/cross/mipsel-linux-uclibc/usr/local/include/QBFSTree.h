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

#ifndef QB_FS_READER_TREE_H_
#define QB_FS_READER_TREE_H_

/**
 * @file QBFSTree.h
 * @brief File system tree class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBFSTree File system tree class
 * @ingroup QBDataModel3
 * @{
 **/

#include <stdlib.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBFSTypes.h>


/**
 * File system tree class.
 **/
typedef struct QBFSTree_ *QBFSTree;


/**
 * Get runtime type identification object representing
 * type of file system tree class.
 **/
extern SvType
QBFSTree_getType(void);

/**
 * Create file system tree instance.
 *
 * @param[in] rootDirPath absolute path to the root directory
 * @param[in] filter    handle to an object implementing ::QBFSFilter
 * @param[in] maxEntries limit of filesystem entries to be read on one level
 * @param[out] errorOut error info
 * @return              created tree or @c NULL in case of error
 **/
extern QBFSTree
QBFSTreeCreate(SvString rootDirPath,
               SvObject filter,
               int maxEntries,
               SvErrorInfo *errorOut);

/**
 * Start asynchronous tasks of file system tree instance.
 *
 * @param[in] self      file system tree handle
 * @param[in] scheduler scheduler to be used by the tree
 * @param[out] errorOut error info
 **/
extern void
QBFSTreeStart(QBFSTree self,
              SvScheduler scheduler,
              SvErrorInfo *errorOut);

/**
 * Stop asynchronous tasks of file system tree instance.
 *
 * @param[in] self      file system tree handle
 * @param[out] errorOut error info
 **/
extern void
QBFSTreeStop(QBFSTree self,
             SvErrorInfo *errorOut);

/**
 * Get root directory of a tree.
 *
 * @param[in] self      file system tree handle
 * @return              root directory handle, @c NULL in case of error
 **/
extern QBFSDir
QBFSTreeGetRootDirectory(QBFSTree self);


/**
 * @}
 **/

#endif
