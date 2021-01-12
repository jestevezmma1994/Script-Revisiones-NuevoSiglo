/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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

/* App/Libraries/SvEPGDataLayer/SvEPGRequestListener.h */

#ifndef SV_EPG_REQUEST_LISTENER_H_
#define SV_EPG_REQUEST_LISTENER_H_

/**
 * @file SvEPGRequestListener.h
 * @brief EPG Request Listener Interface.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup SvEPGRequestListener EPG request listener interface
 * @ingroup SvEPGDataLayer
 * @{
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvArray.h>
#include <SvEPGDataLayer/SvEPGDataRequest.h>
#include <SvEPGDataLayer/Plugins/BaseEPGPlugin.h>

/**
 * Get runtime type identification object representing
 * EPG request listener interface.
 **/
extern SvInterface
SvEPGRequestListener_getInterface(void);


/**
 * SvEPGRequestListener interface.
 **/
typedef const struct SvEPGRequestListener_ {
   /**
    * Notify that the state of the submitted EPG data request has changed.
    *
    * @param[in] self_     handle to an object implementing
    *                      @ref SvEPGRequestListener
    * @param[in] request   EPG data request handle
    **/
    void (*requestStateChanged)(SvGenericObject self_,
                                SvEPGDataRequest request);

    /**
     * Notify that new EPG event data has been received.
     *
     * This method is called by the EPG plugin when new EPG event data
     * is available, either as a result of EPG data request (in such case,
     * @a request will be set) or when plugin detected that data may have
     * changed.
     *
     * @param[in] self_     handle to an object implementing
     *                      @ref SvEPGRequestListener
     * @param[in] plugin    EPG data plugin
     * @param[in] request   EPG data request handle
     * @param[in] channelID unique ID of the channel described by new events
     * @param[in] events    array of new EPG event objects
     **/
    void (*dataReceived)(SvGenericObject self_,
                         SvGenericObject plugin,
                         SvEPGDataRequest request,
                         SvValue channelID,
                         SvArray events);
} *SvEPGRequestListener;

/**
 * @}
 **/


#endif
