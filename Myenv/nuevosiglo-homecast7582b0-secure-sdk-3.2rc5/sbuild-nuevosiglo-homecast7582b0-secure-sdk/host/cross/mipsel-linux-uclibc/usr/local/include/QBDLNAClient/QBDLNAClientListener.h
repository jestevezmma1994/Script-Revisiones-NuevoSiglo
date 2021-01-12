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

/* SMP/QBDLNAClient/QBDLNAClientListener.h */

#ifndef QB_DLNA_CLIENT_LISTENER_H_
#define QB_DLNA_CLIENT_LISTENER_H_

/**
 * @file QBDLNAClientListener.h
 * @brief DLNA client listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBDLNAClientListener client listener interface and dlna request listener interface
 * @ingroup QBDLNAClient
 * @{
 **/
#include <SvFoundation/SvCoreTypes.h>
#include <QBDLNAClient/QBDLNAEvent.h>
#include <QBDLNAClient/QBDLNARequest.h>

/**
 * QBDLNARequestListener interface.
 **/
typedef const struct QBDLNARequestListener_ {
    /**
     * Method called when state of DLNA request has changed.
     *
     * @param[in] self      listener handle
     * @param[in] request   DLNA request handle
     **/
    void (*requestStateChanged)(SvObject self, QBDLNARequest request);
} *QBDLNARequestListener;

typedef struct QBDLNARequestListener_ QBDLNARequestListener_;


/**
 * QBDLNAClientListener interface.
 **/
typedef const struct QBDLNAClientListener_ {
    /**
     * Method called when state of DLNA device list has changed.
     *
     * @param[in] self      listener handle
     * @param[in] event     event to be propagated
     **/

    void (*eventHandler)(SvObject self, QBDLNAEvent event);
} *QBDLNAClientListener;

typedef struct QBDLNAClientListener_ QBDLNAClientListener_;


/**
 * Get DLNA request listener interface
 *
 * @return              DLNA request listener interface
 **/
extern SvInterface
QBDLNARequestListener_getInterface(void);

/**
 * Get DLNA client listener interface
 *
 * @return              DLNA client listener interface
 **/
extern SvInterface
QBDLNAClientListener_getInterface(void);


/**
 * @}
 **/

#endif
