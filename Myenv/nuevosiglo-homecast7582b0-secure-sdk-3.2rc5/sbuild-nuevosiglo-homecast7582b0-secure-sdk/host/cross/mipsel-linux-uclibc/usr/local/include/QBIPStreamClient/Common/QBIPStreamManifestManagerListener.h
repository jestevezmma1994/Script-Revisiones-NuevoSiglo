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

#ifndef QB_IP_STREAM_MANIFEST_MANAGER_LISTENER_H
#define QB_IP_STREAM_MANIFEST_MANAGER_LISTENER_H

/**
 * @file  QBIPStreamManifestManagerListener.h Streaming manifest manager listener.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBIPStreamClient/Common/QBIPStreamManifestManager.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBIPStreamManifestManagerListener Stream manifest manager listener
 * @{
 **/

/**
 * Stream manifest manager listener interface
 * @interface QBIPStreamManifestManagerListener
 */
typedef struct QBIPStreamManifestManagerListener_ *QBIPStreamManifestManagerListener;

struct QBIPStreamManifestManagerListener_ {
    /**
     * Called when new manifest is ready.
     *
     * @param[in] self          listener object
     * @param[in] manifest      new reported manifest
     */
    void (*newManifestReady)(SvObject self, SvObject manifest);

    /**
     * Called when error occurred while downloading manifest
     *
     * @param[in] self      listener object
     * @param[in] error     error info
     */
    void (*manifestError)(SvObject self, QBIPStreamManifestManagerError manifestManagerErrorType);
};

/**
 * Get runtime type identification object representing
 * QBIPStreamManifestManagerListener interface.
 *
 * @relates QBIPStreamManifestManagerListener
 *
 * @return representing interface object
 **/
SvInterface
QBIPStreamManifestManagerListener_getInterface(void);

/**
 * @}
 */

#endif /* QB_IP_STREAM_MANIFEST_MANAGER_LISTENER_H */
