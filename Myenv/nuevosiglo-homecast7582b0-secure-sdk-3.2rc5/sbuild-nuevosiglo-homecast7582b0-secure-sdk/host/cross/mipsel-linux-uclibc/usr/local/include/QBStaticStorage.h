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

#ifndef QB_STATIC_STORAGE_H
#define QB_STATIC_STORAGE_H

/**
 * @file QBStaticStorage.h Static storage service API
 **/

#include <fibers/c/fibers.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvStringBuffer.h>
#include "QBStaticStorageAsync.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBStaticStorageLibrary QBStaticStorage: static storage library
 * @ingroup Utils
 *
 * Static storage library provides utilities for asynchronous deferred
 * access to the file system.
 **/


/**
 * @defgroup QBStaticStorage Static storage singleton class
 * @ingroup QBStaticStorageLibrary
 * @{
 **/

void QBStaticStorageStart(SvScheduler sched, int delayMs);
void QBStaticStorageStop(void);

void QBStaticStorageSetDirty(void);

/**
 * Write content into specified file.
 *
 * @see QBStaticStorageWriteFileWithCallback()
 * @see QBStaticStorageWriteFromBuffer()
 *
 * @param[in] fileName      full path to a target file
 * @param[in] content       content to be written to a file
 * @return                  @c true if we are able write to file, otherwise @c false
 **/
bool QBStaticStorageWriteFile(SvString fileName, SvString content);

/**
 * Write content into specified file and notify the listener
 * when file was written to the storage.
 *
 * @see QBStaticStorageWriteFile()
 *
 * @param [in] fileName           full path to file where content should be written
 * @param [in] content            content to be written to a file
 * @param [in] listener           object implementing @ref QBStaticStorageListener to be notified when
 *                                the content was written to the file
 **/
void QBStaticStorageWriteFileWithCallback(SvString fileName, SvString content, SvObject listener);

/// @copydoc QBStaticStorageWriteFile()
bool QBStaticStorageWriteFromBuffer(SvString fileName, SvStringBuffer content);

bool QBStaticStorageIsBusy(void);

void QBStaticStorageSetFilesystemSync(bool doSync);

void QBStaticStorageRemoveTask(SvString fileName);

/**
 * @}
 **/

/**
 * Get global instance of asynchronous file loader/writer.
 *
 * @ingroup QBStaticStorageAsync
 * @return global instance of QBStaticStorageAsync
 **/
QBStaticStorageAsync QBStaticStorageAsyncInstance(void);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_STATIC_STORAGE_H
