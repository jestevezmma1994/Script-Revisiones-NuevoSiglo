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

// This file was generated on 2015-08-11 using omniidl
// (beeing part of omniORB_4_1) with Cubiware C backend.

/*****************************************************************************
******************************************************************************
******************************** DO NOT EDIT *********************************
******************************************************************************
******************************************************************************/

#ifndef IDL__IAPPLICATIONCONTROL__
#define IDL__IAPPLICATIONCONTROL__


#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>


struct QBRPCIApplicationControl_ {
    uint16_t (*getPID)(SvObject self_, uint16_t* pid);
    uint16_t (*getPIDAsync)(SvObject self_, SvObject caller, uint16_t* callId);

    uint16_t (*getName)(SvObject self_, SvString* name);
    uint16_t (*getNameAsync)(SvObject self_, SvObject caller, uint16_t* callId);

    uint16_t (*quit)(SvObject self_, uint16_t* quitStatus);
    uint16_t (*quitAsync)(SvObject self_, SvObject caller, uint16_t* callId);
};
typedef const struct QBRPCIApplicationControl_ *QBRPCIApplicationControl;
#define QBRPC_IAPPLICATIONCONTROL_NAME "QBRPC::IApplicationControl"

SvInterface QBRPCIApplicationControl_getInterface(void);


struct QBRPCIApplicationControlAsyncCallListener_ {
    void (*getPIDAsyncCallback)(SvObject self_, uint16_t callId, uint16_t pid);
    void (*getNameAsyncCallback)(SvObject self_, uint16_t callId, SvString name);
    void (*quitAsyncCallback)(SvObject self_, uint16_t callId, uint16_t quitStatus);
};
typedef const struct QBRPCIApplicationControlAsyncCallListener_ *QBRPCIApplicationControlAsyncCallListener;

SvInterface QBRPCIApplicationControlAsyncCallListener_getInterface(void);


#endif // IDL__IAPPLICATIONCONTROL__
