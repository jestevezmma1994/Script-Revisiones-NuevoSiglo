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

// This file was generated on 2016-02-24 using omniidl
// (beeing part of omniORB_4_1) with Cubiware C backend.
/*****************************************************************************
******************************************************************************
******************************** DO NOT EDIT *********************************
******************************************************************************
******************************************************************************/

#ifndef IDL__IDIAL__
#define IDL__IDIAL__


#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>


typedef struct {
    SvString name;
    SvString payload;
    SvString additionalDataUrl;
} QBRPCIDIALApplicationStartParams;

typedef struct {
    SvString networkInterfaceId;
    uint32_t dialPort;
    uint32_t ssdpPort;
    SvString friendlyName;
    SvString modelName;
    SvString uuid;
} QBRPCIDIALServerParams;

typedef struct {
    SvString name;
    SvString corsAllowedOrigin;
    bool useAdditionalData;
} QBRPCIDIALApplication;

typedef struct {
    QBRPCIDIALApplication *elements;
    unsigned int count;
} QBRPCIDIALApps;

struct QBRPCIDIAL_ {
    uint16_t (*getServerParams)(SvObject self_, QBRPCIDIALServerParams *params);
    uint16_t (*getServerParamsAsync)(SvObject self_, SvObject caller, uint16_t *callId);

    uint16_t (*getRegisteredApplications)(SvObject self_, QBRPCIDIALApps *apps);
    uint16_t (*getRegisteredApplicationsAsync)(SvObject self_, SvObject caller, uint16_t *callId);

    uint16_t (*startApplication)(SvObject self_, QBRPCIDIALApplicationStartParams const *params);
    uint16_t (*startApplicationAsync)(SvObject self_, SvObject caller, uint16_t *callId, QBRPCIDIALApplicationStartParams const *params);

    uint16_t (*stopApplication)(SvObject self_, SvString appName);
    uint16_t (*stopApplicationAsync)(SvObject self_, SvObject caller, uint16_t *callId, SvString appName);
};
typedef const struct QBRPCIDIAL_ *QBRPCIDIAL;
#define QBRPC_IDIAL_NAME "QBRPC::IDIAL"

SvInterface QBRPCIDIAL_getInterface(void);


struct QBRPCIDIALAsyncCallListener_ {
    void (*getServerParamsAsyncCallback)(SvObject self_, uint16_t callId, QBRPCIDIALServerParams const *params);
    void (*getRegisteredApplicationsAsyncCallback)(SvObject self_, uint16_t callId, QBRPCIDIALApps const *apps);
    void (*startApplicationAsyncCallback)(SvObject self_, uint16_t callId);
    void (*stopApplicationAsyncCallback)(SvObject self_, uint16_t callId);
};
typedef const struct QBRPCIDIALAsyncCallListener_ *QBRPCIDIALAsyncCallListener;

SvInterface QBRPCIDIALAsyncCallListener_getInterface(void);


#endif // IDL__IDIAL__
