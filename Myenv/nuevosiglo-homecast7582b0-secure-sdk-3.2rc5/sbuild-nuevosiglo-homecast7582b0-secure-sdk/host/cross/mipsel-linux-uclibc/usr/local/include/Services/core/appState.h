/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBAPPSTATESERVCE_H_
#define QBAPPSTATESERVCE_H_

#include <SvFoundation/SvCoreTypes.h>

typedef enum {
    QBAppState_InitStateChanged = 0x1u,
    QBAppState_LanguageChanged = 0x2u,
    QBAppState_PVRDiskChanged = 0x4u,
    QBAppState_ParentalControlStateChanged = 0x8u
} QBAppStateChange;

typedef enum {
    QBAppState_initializing,
    QBAppState_starting,
    QBAppState_working,
    QBAppState_stoping,
    QBAppState_destroying
} QBAppInitState;

/**
 * Application PVR disk state
 **/
typedef enum {
    QBPVRDiskState_undefined,           ///< undefined
    QBPVRDiskState_pvr_present,         ///< PVR present
    QBPVRDiskState_ts_present,          ///< time shift present
    QBPVRDiskState_pvr_ts_present,      ///< PVR and time shift present
} QBAppPVRDiskState;

typedef struct QBAppStateService_t* QBAppStateService;

struct QBAppStateServiceListener_t {
    void (*stateChanged)(SvObject self_, uint64_t attrs);
};

typedef struct QBAppStateServiceListener_t* QBAppStateServiceListener;

SvInterface QBAppStateServiceListener_getInterface(void);

SvType QBAppStateService_getType(void);

void QBAppStateServiceAddListener(QBAppStateService self, SvObject listener);

void QBAppStateServiceRemoveListener(QBAppStateService self, SvObject listener);

void QBAppStateServiceRemoveAllListeners(QBAppStateService self);

QBAppInitState QBAppStateServiceGetAppState(QBAppStateService self);

void QBAppStateServiceSetAppState(QBAppStateService self, QBAppInitState state);

void QBAppStateServiceSetLanguage(QBAppStateService self);

void QBAppStateServiceSetPVRDiskState(QBAppStateService self, QBAppPVRDiskState state);

QBAppPVRDiskState QBAppStateServiceGetPVRDiskState(QBAppStateService self);

void QBAppStateParentalControlStateChanged(QBAppStateService self);

#endif
