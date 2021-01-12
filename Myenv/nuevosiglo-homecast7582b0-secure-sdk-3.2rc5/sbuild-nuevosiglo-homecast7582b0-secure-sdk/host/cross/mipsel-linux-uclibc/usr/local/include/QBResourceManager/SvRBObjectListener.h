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

#ifndef SV_RB_OBJECT_LISTENER_H_
#define SV_RB_OBJECT_LISTENER_H_

/**
 * @file SvRBObjectListener.h
 * @brief Resource manager listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvFoundation/SvCoreTypes.h>
#include <QBResourceManager/SvRBObject.h>


/**
 * @defgroup SvRBObjectListener Resource manager object listener interface
 * @ingroup QBResourceManagerLibrary
 *
 * An interface for object that dynamically manages its resources usage.
 *
 * @{
 **/

/**
 * Resource manager object listener interface.
 **/
typedef const struct SvRBObjectListener_ {
    /**
     * Notify listener that asynchronous resource loading is finished.
     *
     * @param[in] self_     handle to a @ref SvRBObjectListener
     * @param[in] object_   handle to an object representing resource
     **/
    void (*loadingFinished)(SvObject self_, SvRBObject object_);

    /**
     * Ask if listener agrees to free the shared resource it uses.
     *
     * @param[in] self_     handle to a @ref SvRBObjectListener
     * @param[in] object_   handle to an object representing resource
     * @return              @c true if object removal request has been accepted, @c false if not
     **/
    bool (*canFreeResource)(SvObject self_, SvRBObject object_);

    /**
     * Tell listener to free the shared resource it uses.
     *
     * @param[in] self_     handle to a @ref SvRBObjectListener
     * @param[in] object_   handle to an object representing resource
     **/
    void (*freeResource)(SvObject self_, SvRBObject object_);
} *SvRBObjectListener;


/**
 * Get runtime type identification object representing
 * resource manager object listener interface.
 *
 * @return resource manager object listener interface
 **/
extern SvInterface SvRBObjectListener_getInterface(void);


/**
 * @}
 **/

#endif   // SV_RB_OBJECT_LISTENER_H_
