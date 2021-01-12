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

#ifndef QB_IP_STREAM_MANIFEST_LOGIC_IFACE_H
#define QB_IP_STREAM_MANIFEST_LOGIC_IFACE_H

/**
 * @file  QBIPStreamManifestLogicIface.h Streaming manifest logic interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/Common/QBIPStreamManifestManager.h>
#include <QBIPStreamClient/Common/QBIPStreamManifestLogicListener.h>
#include <QBIPStreamClient/Common/QBIPStreamDownloader.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBIPStreamManifestLogicIface Streaming manifest logic interface.
 * @{
 **/

/**
 * Stream manifest logic interface
 * @interface QBIPStreamManifestLogicIface
 */
typedef struct QBIPStreamManifestLogicIface_ *QBIPStreamManifestLogicIface;

struct QBIPStreamManifestLogicIface_ {
    /**
     * Set manifest logic listener.
     *
     * @param[in] self          interface object
     * @param[in] listener      listener to set
     */
    void (*setListener)(SvObject self, SvObject listener);

    /**
     * Notify about new manifest.
     *
     * @param[in] self          interface object
     * @param[in] manifest      downloaded manifest
     * @param[in] requestStartTime  time at which manifest request was issued
     */
    void (*newManifest)(SvObject self, SvObject manifest, SvTime requestStartTime);

    /**
     * Start manifest logic.
     *
     * @param[in] self          interface object
     * @param[in] url           first manifest URL
     */
    void (*start)(SvObject self, SvURL url);

    /**
     * Stop manifest logic.
     *
     * @param[in] self          interface object
     */
    void (*stop)(SvObject self);

    /**
     * Parser error.
     *
     * @param[in] self          interface object
     * @param[in] url           parsed manifest URL
     * @param[in] error         error info
     */
    void (*parserError)(SvObject self, SvURL url, QBIPStreamManifestManagerError manifestManagerErrorType);

    /**
     * Stream downloader error.
     *
     * @param[in] self          interface object
     * @param[in] url           URL whose receiving reported error
     * @param[in] error         stream downloader error
     */
    void (*downloaderError)(SvObject self, SvURL url, QBIPStreamDownloaderError downloaderErrorType);
};

/**
 * Get runtime type identification object representing
 * QBIPStreamManifestLogicIface interface.
 *
 * @relates QBIPStreamManifestLogicIface
 *
 * @return representing interface object
 **/
SvInterface
QBIPStreamManifestLogicIface_getInterface(void);

/**
 * @}
 */

#endif /* QB_IP_STREAM_MANIFEST_LOGIC_IFACE_H */
