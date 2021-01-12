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

// This file was generated on 2015-11-20 using omniidl
// (beeing part of omniORB_4_1) with Cubiware C backend.
/*****************************************************************************
******************************************************************************
******************************** DO NOT EDIT *********************************
******************************************************************************
******************************************************************************/

#ifndef IDL__IWEBBROWSERLISTENER__
#define IDL__IWEBBROWSERLISTENER__


#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>


typedef enum {
    QBRPCIWebBrowserListenerState_Idle = 0,
    QBRPCIWebBrowserListenerState_StartedLoadingNewPage = 1,
    QBRPCIWebBrowserListenerState_FinishedLoadingNewPage = 2,
} QBRPCIWebBrowserListenerState;

struct QBRPCIWebBrowserListener_ {
    uint16_t (*stateChanged)(SvObject self_, QBRPCIWebBrowserListenerState state, int16_t* __returnValue__);
    uint16_t (*stateChangedAsync)(SvObject self_, SvObject caller, uint16_t* callId, QBRPCIWebBrowserListenerState state);
};
typedef const struct QBRPCIWebBrowserListener_ *QBRPCIWebBrowserListener;
#define QBRPC_IWEBBROWSERLISTENER_NAME "QBRPC::IWebBrowserListener"

SvInterface QBRPCIWebBrowserListener_getInterface(void);


struct QBRPCIWebBrowserListenerAsyncCallListener_ {
    void (*stateChangedAsyncCallback)(SvObject self_, uint16_t callId, int16_t __returnValue__);
};
typedef const struct QBRPCIWebBrowserListenerAsyncCallListener_ *QBRPCIWebBrowserListenerAsyncCallListener;

SvInterface QBRPCIWebBrowserListenerAsyncCallListener_getInterface(void);


#endif // IDL__IWEBBROWSERLISTENER__
