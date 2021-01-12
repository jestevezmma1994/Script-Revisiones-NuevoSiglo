/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef VIDEO_OUTPUT_LOGIC_H_
#define VIDEO_OUTPUT_LOGIC_H_

/**
 * @file VideoOutputLogic.h Video output logic class API
 * @brief Video output logic API
 **/

#include <QBPlatformHAL/QBPlatformTypes.h>
#include <Utils/viewport.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <QBAppKit/QBAsyncService.h>
#include <dataformat/sv_data_format.h>
#include <main_decl.h>
#include <stdbool.h>


/**
 * @defgroup VideoOutputLogic Video output logic class
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * Video output logic class.
 *
 * @class VideoOutputLogic VideoOutputLogic.h <Logic/VideoOutputLogic.h>
 * @extends SvObject
 *
 * This is an application logic class implementing functionality related to
 * video output handling. An instance of this class can be created by using
 * @ref QBLogicFactory.
 **/
typedef struct VideoOutputLogic_ {
    /**
     * super class
     */
    struct SvObject_ super_;
    /**
     * application globals
     */
    AppGlobals appGlobals;
    /**
     * service state
     */
    QBAsyncServiceState serviceState;

    QBTVSystem tvSystem;

    bool analogOutputsPresent;
    bool automaticFrameRateSelectionEnabled;
    QBOutputStandard selectedAnalogMode;

    bool videoInfoPresent;
    QBViewportVideoInfo videoInfo;
    /**
     * flag telling if currently played video is interlaced
     **/
    bool videoInterlaced;
    /**
     * service dependencies list
     */
    SvImmutableArray dependencies;
} *VideoOutputLogic;

/**
 * @relates VideoOutputLogic
 *
 * Get runtime type identification object representing video output logic class.
 *
 * @return VideoOutputLogic type identification object
 **/
SvType VideoOutputLogic_getType(void);

/**
 * Video output logic virtual methods table.
 **/
typedef const struct VideoOutputLogicVTable_ {
    /**
     * virtual method table of the base class
     **/
    struct SvObjectVTable_ super_;

    /**
     * Initialize output handle logic instance.
     *
     * @param[in]  self       global video output logic handle
     * @param[in]  appGlobals CubiTV application state
     **/
    void (*init)(VideoOutputLogic self, AppGlobals appGlobals);

    void (*setupOverscanCompensation)(VideoOutputLogic self);

    void (*setSCARTSignalType)(VideoOutputLogic self, const char *type);

    void (*TVSystemChanged)(VideoOutputLogic self, QBTVSystem tvSystem);

    void (*verifyConfig)(VideoOutputLogic self, QBVideoOutputConfig *outputs, unsigned int outputsCnt, int *RFChannel);

    /**
     * RF modulator find channel.
     *
     * @protected VideoOutputLogic
     *
     * @param[in] self       global video output logic handle
     * @param[in] RFOutputID output RF id
     * @param[in] channel    channel to find
     * @return               @c true if channel exist, @c false otherwise
     **/
    bool (*RFModulatorFindChannel)(VideoOutputLogic self, unsigned int RFOutputID, unsigned int channel);

    /**
     * Set analog video mode.
     *
     * @protected VideoOutputLogic
     *
     * @param[in] self    global video output logic handle
     * @param[in] newMode new mode
     **/
    void (*setAnalogVideoMode)(VideoOutputLogic self, QBOutputStandard newMode);
} *VideoOutputLogicVTable;

/**
 * Create VideoOutputLogic class.
 *
 * @param[in] appGlobals application global data handle
 * @param[out] errorOut  error info
 * @return               video output logic object
 */
extern VideoOutputLogic VideoOutputLogicCreate(AppGlobals appGlobals, SvErrorInfo *errorOut);

extern void VideoOutputLogicSetRFModulator(VideoOutputLogic self, const char *type);

extern int VideoOutputLogicSetContentProtection(VideoOutputLogic self, const struct sv_content_protection *contentProtectionInfo);

extern void VideoOutputLogicAfterOutputsSetup(VideoOutputLogic self);

extern void VideoOutputLogicDisableAutomaticFrameRateSelection(VideoOutputLogic self);

extern void VideoOutputLogicEnableAutomaticFrameRateSelection(VideoOutputLogic self);

extern void VideoOutputLogicSetVideoContentMode(VideoOutputLogic self, const char* output_type, const char* mode_name);

extern void VideoOutputLogicToggleVideoContentMode(VideoOutputLogic self);

/**
 * Indicates if change of video content mode or aspect ratio on one output should also affect another outputs.
 *
 * @param[in] self  VideoOutputLogic handler
 * @return          true if outputs should be linked, false if not.
 **/
extern bool VideoOutputLogicIsOutputConfigurationSeparated(VideoOutputLogic self);

/**
 * Initialize output handle logic instance.
 *
 * @param[in] self       video output logic handle
 * @param[in] appGlobals application global data handle
 **/
static inline void VideoOutputLogicInit(VideoOutputLogic self, AppGlobals appGlobals)
{
    SvInvokeVirtual(VideoOutputLogic, self, init, appGlobals);
}

static inline void VideoOutputLogicSetupOverscanCompensation(VideoOutputLogic self)
{
    SvInvokeVirtual(VideoOutputLogic, self, setupOverscanCompensation);
}

static inline void VideoOutputLogicSetSCARTSignalType(VideoOutputLogic self, const char *type)
{
    SvInvokeVirtual(VideoOutputLogic, self, setSCARTSignalType, type);
}

static inline void VideoOutputLogicTVSystemChanged(VideoOutputLogic self, QBTVSystem tvSystem)
{
    SvInvokeVirtual(VideoOutputLogic, self, TVSystemChanged, tvSystem);
}

static inline void VideoOutputLogicVerifyConfig(VideoOutputLogic self, QBVideoOutputConfig *outputs, unsigned int outputsCnt, int *RFChannel)
{
    SvInvokeVirtual(VideoOutputLogic, self, verifyConfig, outputs, outputsCnt, RFChannel);
}

/**
 * @}
 **/

#endif // #ifndef VIDEO_OUTPUT_LOGIC_H_
