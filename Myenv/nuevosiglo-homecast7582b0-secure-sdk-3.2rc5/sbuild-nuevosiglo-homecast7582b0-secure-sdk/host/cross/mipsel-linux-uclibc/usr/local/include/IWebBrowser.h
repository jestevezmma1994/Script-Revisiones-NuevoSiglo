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

// This file was generated on 2016-08-17 using omniidl
// (beeing part of omniORB_4_1) with Cubiware C backend.
/*****************************************************************************
******************************************************************************
******************************** DO NOT EDIT *********************************
******************************************************************************
******************************************************************************/

#ifndef IDL__IWEBBROWSER__
#define IDL__IWEBBROWSER__


#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>


typedef enum {
    QBRPCIWebBrowserPopupHandlingPolicy_AcceptAll = 0,
    QBRPCIWebBrowserPopupHandlingPolicy_CancelAll = 1,
    QBRPCIWebBrowserPopupHandlingPolicy_HandleAll = 2,
} QBRPCIWebBrowserPopupHandlingPolicy;

typedef struct {
    SvString address;
    SvString userAgent;
    bool spatialNavigation;
    bool enableUserJS;
    bool enableRemoteDebug;
    uint16_t remoteDebugPort;
    QBRPCIWebBrowserPopupHandlingPolicy popupHandlingPolicy;
} QBRPCIWebBrowserParams;

struct QBRPCIWebBrowser_ {
    uint16_t (*setParams)(SvObject self_, QBRPCIWebBrowserParams const *params);
    uint16_t (*setParamsAsync)(SvObject self_, SvObject caller, uint16_t *callId, QBRPCIWebBrowserParams const *params);

    uint16_t (*getParams)(SvObject self_, QBRPCIWebBrowserParams *params);
    uint16_t (*getParamsAsync)(SvObject self_, SvObject caller, uint16_t *callId);

    uint16_t (*start)(SvObject self_);
    uint16_t (*startAsync)(SvObject self_, SvObject caller, uint16_t *callId);
};
typedef const struct QBRPCIWebBrowser_ *QBRPCIWebBrowser;
#define QBRPC_IWEBBROWSER_NAME "QBRPC::IWebBrowser"

SvInterface QBRPCIWebBrowser_getInterface(void);


struct QBRPCIWebBrowserAsyncCallListener_ {
    void (*setParamsAsyncCallback)(SvObject self_, uint16_t callId);
    void (*getParamsAsyncCallback)(SvObject self_, uint16_t callId, QBRPCIWebBrowserParams const *params);
    void (*startAsyncCallback)(SvObject self_, uint16_t callId);
};
typedef const struct QBRPCIWebBrowserAsyncCallListener_ *QBRPCIWebBrowserAsyncCallListener;

SvInterface QBRPCIWebBrowserAsyncCallListener_getInterface(void);


#endif // IDL__IWEBBROWSER__
