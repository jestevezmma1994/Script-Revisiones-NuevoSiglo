/*****************************************************************************
 ** Cubiware Sp. z o.o. Software License Version 1.0
 **
 ** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
 **
 ** Any rights which are not expressly granted in this License are entirely and
 ** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
 ** modify, translate, reverse engineer, decompile, disassemble, or create
 ** derivative works based on this Software. You may not make access to this
 ** Software available to others in connection with a service bureau,
 ** application service provider, or similar business, or make any other use of
 ** this Software without express written permission from Cubiware Sp z o.o.
 **
 ** Any User wishing to make use of this Software must contact
 ** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
 ** includes, but is not limited to:
 ** (1) integrating or incorporating all or part of the code into a product for
 **     sale or license by, or on behalf of, User to third parties;
 ** (2) distribution of the binary or source code to third parties for use with
 **     a commercial product sold or licensed by, or on behalf of, User.
 ******************************************************************************/
#ifndef QB_STATIC_STORAGE_ASYNC_H
#define QB_STATIC_STORAGE_ASYNC_H

/**
 * @file QBStaticStorageAsync.h
 * @brief Asynchronous file loader/writer.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <fibers/c/fibers.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvData.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBStaticStorageAsync Asynchronous file loader/writer class
 * @ingroup QBStaticStorageLibrary
 * @{
 *
 * QBStaticStorageAsync is an asynchronous file reader/writer.
 *
 * Accessing mass storage (HDD or Flash memory) is relatively slow and can
 * block the execution thread for tens of milliseconds. This class reads
 * and writes ordinary files in his own thread and doesn't block the caller:
 * it provides a notification (callback) after operation is finished.
 **/

/**
 * Asynchronous file loader/writer class.
 *
 * @class QBStaticStorageAsync
 * @extends SvObject
 **/
typedef struct QBStaticStorageAsync_* QBStaticStorageAsync;

/**
 * Callback called when QBStaticStorageAsyncFileLoad() is done.
 *
 * @param[in] target callback owner
 * @param[in] filename path to loaded file
 * @param[in] data loaded data or NULL on error
 */
typedef void (QBStaticStorageAsyncFileLoadDoneCallback)(void* target, SvString filename, SvData data);

/**
 * Create asynchronous loader/writer instance.
 *
 * @memberof QBStaticStorageAsync
 *
 * @param[in] callbackScheduler scheduler which should be used to call callbacks
 * @return created instance, @c NULL in case of error
 **/
QBStaticStorageAsync QBStaticStorageAsyncCreate(SvScheduler callbackScheduler);

/**
 * Finish all save operations and release asynchronous loader/writer instance.
 *
 * @memberof QBStaticStorageAsync
 *
 * @param[in] self asynchronous loader/writer instance.
 **/
void QBStaticStorageAsyncAutodestroy(QBStaticStorageAsync self);

/**
 * Request file write operation.
 *
 * @memberof QBStaticStorageAsync
 *
 * @param[in] self asynchronous loader/writer instance.
 * @param[in] filename path to file
 * @param[in] content data that should be written into file
 */
void QBStaticStorageAsyncFileWrite(QBStaticStorageAsync self, SvString filename, SvData content);

/**
 * Request file load operation.
 *
 * @memberof QBStaticStorageAsync
 *
 * @param[in] self asynchronous loader/writer instance.
 * @param[in] filename path to file
 * @param[in] maxFileSize max size in bytes that is allowed to read
 * @param[in] callback function invoked when loading file is finished
 * @param[in] target first parameter in callback
 */
void QBStaticStorageAsyncFileLoad(QBStaticStorageAsync self, SvString filename, int maxFileSize, QBStaticStorageAsyncFileLoadDoneCallback* callback, void* target);

/**
 * Cancel file load request, after this method callback won't be invoked
 *
 * @memberof QBStaticStorageAsync
 *
 * @param[in] self asynchronous loader/writer instance.
 * @param[in] filename path to file
 * @param[in] callback function invoked when loading file is finished
 * @param[in] target first parameter in callback
 */
void QBStaticStorageAsyncCancelOperation(QBStaticStorageAsync self, SvString filename, QBStaticStorageAsyncFileLoadDoneCallback callback, void* target);

/**
 * Remove allowed device.
 *
 * @memberof QBStaticStorageAsync
 *
 * @param[in] self asynchronous loader/writer instance
 * @param[in] allowedDevice path to root dir of allowed device
 **/
void QBStaticStorageAsyncRemoveAllowedDevice(QBStaticStorageAsync self, SvString allowedDevice);

/**
 * Add allowed device.
 *
 * @memberof QBStaticStorageAsync
 *
 * @param[in] self asynchronous loader/writer instance
 * @param[in] allowedDevice path to root dir of allowed device
 **/
void QBStaticStorageAsyncAddAllowedDevice(QBStaticStorageAsync self, SvString allowedDevice);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_STATIC_STORAGE_ASYNC_H
