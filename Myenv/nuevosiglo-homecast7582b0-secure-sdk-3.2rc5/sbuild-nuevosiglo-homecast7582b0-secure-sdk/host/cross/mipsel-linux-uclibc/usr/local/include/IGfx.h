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

#ifndef IDL__IGFX__
#define IDL__IGFX__


#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>


typedef enum {
    QBRPCIGfxAspectRatio_Unknown = 0,
    QBRPCIGfxAspectRatio_16x9 = 1,
    QBRPCIGfxAspectRatio_4x3 = 2,
} QBRPCIGfxAspectRatio;

typedef enum {
    QBRPCIGfxMode_Disabled = 0,
    QBRPCIGfxMode_FullScreen = 1,
    QBRPCIGfxMode_Windowed = 2,
} QBRPCIGfxMode;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    QBRPCIGfxAspectRatio aspectRatio;
    QBRPCIGfxMode mode;
} QBRPCIGfxParams;

struct QBRPCIGfx_ {
    uint16_t (*setGfxParams)(SvObject self_, QBRPCIGfxParams const * params);
    uint16_t (*setGfxParamsAsync)(SvObject self_, SvObject caller, uint16_t* callId, QBRPCIGfxParams const * params);

    uint16_t (*getGfxParams)(SvObject self_, QBRPCIGfxParams* params);
    uint16_t (*getGfxParamsAsync)(SvObject self_, SvObject caller, uint16_t* callId);
};
typedef const struct QBRPCIGfx_ *QBRPCIGfx;
#define QBRPC_IGFX_NAME "QBRPC::IGfx"

SvInterface QBRPCIGfx_getInterface(void);


struct QBRPCIGfxAsyncCallListener_ {
    void (*setGfxParamsAsyncCallback)(SvObject self_, uint16_t callId);
    void (*getGfxParamsAsyncCallback)(SvObject self_, uint16_t callId, QBRPCIGfxParams const * params);
};
typedef const struct QBRPCIGfxAsyncCallListener_ *QBRPCIGfxAsyncCallListener;

SvInterface QBRPCIGfxAsyncCallListener_getInterface(void);


#endif // IDL__IGFX__
