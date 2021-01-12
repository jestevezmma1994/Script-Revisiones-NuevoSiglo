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

#ifndef HDMI_SERVICE_H_
#define HDMI_SERVICE_H_

/**
 * @file HDMIService.h HDMI service API
 **/

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <main_decl.h>
#include <stdbool.h>
#include <fibers/c/fibers.h>

typedef struct HDMIService_ *HDMIService;

typedef struct HDMIServiceStatusListener_ {
    /// Notify that status of HDCP authorization was changed
    void (*HDCPStatusChanged)(SvObject self_, const HDCPAuthStatus status);
    /// Notify that status of HDMI output was changed
    void (*hotplugStatusChanged)(SvObject self_, const bool isConnected);
} *HDMIServiceStatusListener;

/**
 * Get the interface to HDMIServiceHDCPStatus listener
 * @return SvInterface      listener interface
 */
SvInterface HDMIServiceStatusListener_getInterface(void);

extern HDMIService HDMIServiceCreate(AppGlobals appGlobals);
extern void HDMIServiceStart(SvObject self_, SvScheduler scheduler, SvErrorInfo *errorOut);
extern void HDMIServiceStop(SvObject self_, SvErrorInfo *errorOut);

extern void HDMIServiceSetVideoMode(HDMIService self, QBOutputStandard mode);
extern QBOutputStandard HDMIServiceGetSelectedVideoMode(HDMIService self);

extern bool HDMIServiceIsOriginalModeActive(HDMIService self);
extern void HDMIServiceDisableOriginalMode(HDMIService self);
extern void HDMIServiceEnableOriginalMode(HDMIService self);

extern bool HDMIServiceIsAutomaticModeActive(HDMIService self);
extern void HDMIServiceDisableAutomaticMode(HDMIService self);
extern void HDMIServiceEnableAutomaticMode(HDMIService self);

extern void HDMIServiceDisableAutomaticFrameRateSelection(HDMIService self) __attribute__((weak));
extern void HDMIServiceEnableAutomaticFrameRateSelection(HDMIService self) __attribute__((weak));

extern void HDMIServiceTVSystemChanged(HDMIService self, QBTVSystem tvSystem);

extern QBTVSystem HDMIServiceGetTVSystem(HDMIService self);

/**
 * HDMI authorization status add listener
 * @param[in] self         HDMIService object handler
 * @param[in] listener     listener object handle
 * @param[out] errorOut    error info
 */
void HDMIServiceStatusAddListener(HDMIService self, SvObject listener, SvErrorInfo *errorOut);

/**
 * HDMI authorization status remove listener
 * @param[in] self         HDMIService object handler
 * @param[in] listener     listener object handle
 * @param[out] errorOut    error info
 */
void HDMIServiceStatusRemoveListener(HDMIService self, SvObject listener, SvErrorInfo *errorOut);

/**
 * Get HDCP Authorization status
 *
 * @param[in] self         HDMIService object handler
 * @return HDCPAuthStatus  HDCP authorization status
 */
extern HDCPAuthStatus HDMIServiceGetHDCPAuthStatus(HDMIService self);

#endif
