/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DRM_MANAGER_LISTENER_H
#define QB_DRM_MANAGER_LISTENER_H

/**
 * @file QBDRMManagerListener.h
 * @brief DRM Manager listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvTime.h>
#include <stdbool.h>

/**
 * @defgroup QBDRMManagerListener DRM Manager listener
 * @ingroup QBCubiTV_services
 * @{
 **/

/**
 * @relates QBDRMManagerListener
 *
 * QBDRMManagerListener data.
 **/
typedef struct {
    SvTime availabilityTime;     /**< timeshift availability time */
    bool isAvailable;            /**< @c true if timeshift is available, @c false if not */
} QBDRMManagerData;

/**
 * DRM Manager listener.
 **/
typedef const struct QBDRMManagerListener_ {
    /**
     * Notify that state of a URI Data.
     *
     * @param[in] self_   handle to a @ref QBDRMManagerListener
     * @param[in] data    data DRM Manager
     **/
    void (*DRMDataChanged)(SvObject self_,
                           const QBDRMManagerData* data);
} *QBDRMManagerListener;

/**
 * Get runtime type identification object representing
 * QBDRMManagerListener interface.
 *
 * @return QBDRMManagerListener interface object
 **/
extern SvInterface
QBDRMManagerListener_getInterface(void);

/**
 * @}
 **/


#endif // QB_DRM_MANAGER_LISTENER_H
