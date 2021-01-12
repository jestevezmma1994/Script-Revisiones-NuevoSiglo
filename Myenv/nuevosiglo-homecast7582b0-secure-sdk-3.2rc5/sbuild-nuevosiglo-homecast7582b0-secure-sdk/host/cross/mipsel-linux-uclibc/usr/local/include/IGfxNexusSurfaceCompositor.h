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

// This file was generated on 2015-09-07 using omniidl
// (beeing part of omniORB_4_1) with Cubiware C backend.

/*****************************************************************************
******************************************************************************
******************************** DO NOT EDIT *********************************
******************************************************************************
******************************************************************************/

#ifndef IDL__IGFXNEXUSSURFACECOMPOSITOR__
#define IDL__IGFXNEXUSSURFACECOMPOSITOR__


#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>


struct QBRPCIGfxNexusSurfaceCompositor_ {
    uint16_t (*setClientID)(SvObject self_, int16_t clientID, int16_t* __returnValue__);
    uint16_t (*setClientIDAsync)(SvObject self_, SvObject caller, uint16_t* callId, int16_t clientID);

    uint16_t (*getClientID)(SvObject self_, int16_t* clientID, int16_t* __returnValue__);
    uint16_t (*getClientIDAsync)(SvObject self_, SvObject caller, uint16_t* callId);
};
typedef const struct QBRPCIGfxNexusSurfaceCompositor_ *QBRPCIGfxNexusSurfaceCompositor;
#define QBRPC_IGFXNEXUSSURFACECOMPOSITOR_NAME "QBRPC::IGfxNexusSurfaceCompositor"

SvInterface QBRPCIGfxNexusSurfaceCompositor_getInterface(void);


struct QBRPCIGfxNexusSurfaceCompositorAsyncCallListener_ {
    void (*setClientIDAsyncCallback)(SvObject self_, uint16_t callId, int16_t __returnValue__);
    void (*getClientIDAsyncCallback)(SvObject self_, uint16_t callId, int16_t clientID, int16_t __returnValue__);
};
typedef const struct QBRPCIGfxNexusSurfaceCompositorAsyncCallListener_ *QBRPCIGfxNexusSurfaceCompositorAsyncCallListener;

SvInterface QBRPCIGfxNexusSurfaceCompositorAsyncCallListener_getInterface(void);


#endif // IDL__IGFXNEXUSSURFACECOMPOSITOR__
