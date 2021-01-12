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

// This file was generated on 2016-04-18 using omniidl
// (beeing part of omniORB_4_1) with Cubiware C backend.
/*****************************************************************************
******************************************************************************
******************************** DO NOT EDIT *********************************
******************************************************************************
******************************************************************************/

#ifndef IDL__IDIALLISTENER__
#define IDL__IDIALLISTENER__


#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>


typedef enum {
    QBRPCIDIALListenerApplicationState_Unknown = 0,
    QBRPCIDIALListenerApplicationState_Stopped = 1,
    QBRPCIDIALListenerApplicationState_Running = 2,
} QBRPCIDIALListenerApplicationState;

struct QBRPCIDIALListener_ {
    uint16_t (*applicationStateChanged)(SvObject self_, SvString name, QBRPCIDIALListenerApplicationState state);
    uint16_t (*applicationStateChangedAsync)(SvObject self_, SvObject caller, uint16_t *callId, SvString name, QBRPCIDIALListenerApplicationState state);

    uint16_t (*createAdditionalDataUrl)(SvObject self_, SvString appName, SvString *additionalDataUrl);
    uint16_t (*createAdditionalDataUrlAsync)(SvObject self_, SvObject caller, uint16_t *callId, SvString appName);
};
typedef const struct QBRPCIDIALListener_ *QBRPCIDIALListener;
#define QBRPC_IDIALLISTENER_NAME "QBRPC::IDIALListener"

SvInterface QBRPCIDIALListener_getInterface(void);


struct QBRPCIDIALListenerAsyncCallListener_ {
    void (*applicationStateChangedAsyncCallback)(SvObject self_, uint16_t callId);
    void (*createAdditionalDataUrlAsyncCallback)(SvObject self_, uint16_t callId, SvString additionalDataUrl);
};
typedef const struct QBRPCIDIALListenerAsyncCallListener_ *QBRPCIDIALListenerAsyncCallListener;

SvInterface QBRPCIDIALListenerAsyncCallListener_getInterface(void);


#endif // IDL__IDIALLISTENER__
