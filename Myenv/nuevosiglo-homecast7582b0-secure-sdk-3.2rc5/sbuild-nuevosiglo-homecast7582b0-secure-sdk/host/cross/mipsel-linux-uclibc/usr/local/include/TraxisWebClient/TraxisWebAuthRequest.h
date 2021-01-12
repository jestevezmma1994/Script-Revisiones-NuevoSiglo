/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef TRAXIS_WEB_AUTH_REQUEST_H_
#define TRAXIS_WEB_AUTH_REQUEST_H_

/**
 * @file TraxisWebAuthRequest.h
 * @brief Traxis.Web authentication request type
 **/

#include <SvFoundation/SvCoreTypes.h>

/**
 * @addtogroup TraxisWebSessionManager
 * @{
 **/

/**
 * Traxis.Web authentication request.
 **/
typedef struct TraxisWebAuthRequest_* TraxisWebAuthRequest;

typedef enum {
    TraxisWebAuthRequestState_inProgress,
    TraxisWebAuthRequestState_finished,
} TraxisWebAuthRequestState;

typedef struct TraxisWebAuthRequestListener_t {
    void (*changed)(SvObject self_, TraxisWebAuthRequest req);
} * TraxisWebAuthRequestListener;

SvInterface TraxisWebAuthRequestListener_getInterface(void);

TraxisWebAuthRequest TraxisWebAuthRequestCreate(void);

TraxisWebAuthRequestState TraxisWebAuthRequestGetState(TraxisWebAuthRequest self);

void TraxisWebAuthRequestSetState(TraxisWebAuthRequest self, TraxisWebAuthRequestState state);

void TraxisWebAuthRequestAddListener(TraxisWebAuthRequest self, SvObject listener);

void TraxisWebAuthRequestRemoveListener(TraxisWebAuthRequest self, SvObject listener);

/**
 * @}
 **/

#endif
