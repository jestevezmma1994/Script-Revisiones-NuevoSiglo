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

#ifndef QBIPSTREAMCLIENT_QBIPSTREAMDOWNLOADERLISTENER_H_
#define QBIPSTREAMCLIENT_QBIPSTREAMDOWNLOADERLISTENER_H_

/**
 * @file QBIPStreamDownloaderListener.h Stream Downloader listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/Common/QBIPStreamDownloader.h>
#include <SvPlayerKit/SvBuf.h>
#include <storage_input/storage_input.h>
#include <SvFoundation/SvInterface.h>

#include <stdint.h>

/**
 * @defgroup QBIPStreamDownloaderListener Stream downloader listener
 * @{
 * Stream downloader listener interface
 **/

/**
 * Stream downloader listener interface
 * @interface QBIPStreamDownloaderListener
 */
typedef struct QBIPStreamDownloaderListener_ {
    /**
     * Called when storage input is connected
     *
     * @param[in] self_ listener object
     * @param[in] downloader handle to a downloader that calls the function
     * @param[in] storageInput connected storage input
     */
    void (*inputConnected)(SvObject self_,
                           QBIPStreamDownloader downloader,
                           struct storage_input *storageInput);

    /**
     * Called when downloaded new data chunk
     *
     * Data must not be released by SvBufRelease.
     * Listener may retain data and release later.
     *
     * @param[in] self_ listener object
     * @param[in] downloader handle to a downloader that calls the function
     * @param[in] sb buffer
     * @param[in] pos position in buffer
     */
    void (*data)(SvObject self_,
                 QBIPStreamDownloader downloader,
                 SvBuf sb,
                 int64_t pos);

    /**
     * Called when downloader state is changed
     *
     * @param[in] self_ listener object
     * @param[in] downloader handle to a downloader that calls the function
     * @param[in] state new state
     */
    void (*stateChanged)(SvObject self_,
                         QBIPStreamDownloader downloader,
                         QBIPStreamDownloaderState state);

    /**
     * Called when error occured
     *
     * @param[in] self_ listener object
     * @param[in] downloader handle to a downloader that calls the function
     * @param[in] err error type
     */
    void (*error)(SvObject self_,
                  QBIPStreamDownloader downloader,
                  QBIPStreamDownloaderError err);
} *QBIPStreamDownloaderListener;

/**
 * Get runtime type identification object representing
 * QBIPStreamDownloaderListener interface.
 *
 * @relates QBIPStreamDownloaderListener
 *
 * @return representing interface object
 **/
SvInterface
QBIPStreamDownloaderListener_getInterface(void);

/**
 * Stream downloader listener filter interface
 * @interface QBIPStreamDownloaderFilter
 */
typedef struct QBIPStreamDownloaderFilter_ {
    /// super interface
    struct QBIPStreamDownloaderListener_ super_;

    /**
     * Set or replace listener of the filter
     *
     * @param[in] self_ filter object
     * @param[in] downloader handle to a downloader that calls the function
     * @param[in] listener listener object
     */
    void (*setListener)(SvObject self_,
                        SvObject listener);

} *QBIPStreamDownloaderFilter;

/**
 * Get runtime type identification object representing
 * QBIPStreamDownloaderFilter interface.
 *
 * @relates QBIPStreamDownloaderFilter
 *
 * @return representing interface object
 **/
SvInterface
QBIPStreamDownloaderFilter_getInterface(void);

/**
 * @}
 */

#endif /* QBIPSTREAMCLIENT_QBIPSTREAMDOWNLOADERLISTENER_H_ */
