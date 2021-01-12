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

#ifndef AUDIO_OUTPUT_LOGIC_H_
#define AUDIO_OUTPUT_LOGIC_H_

#include <QBAppKit/QBAsyncService.h>
#include <QBPlatformHAL/QBPlatformEDID.h>
#include <main_decl.h>

/**
* @file AudioOutputLogic.h AudioOutpuLogic class API
* @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
* @endxmlonly
**/
/**
 * Audio iutput logic class.
 *
 * @class AudioOutputLogic AudioOutputLogic.h <Logic/AudioOutputLogic.h>
 * @extends SvObject
 *
 * This is an application logic class implementing functionality related to
 * audio output handling. An instance of this class can be created by using
 * @ref QBLogicFactory.
 **/
typedef struct AudioOutputLogic_ {
    struct SvObject_ super_;    ///< SvObject super class
    QBAsyncServiceState state;  ///< async service state
} *AudioOutputLogic;

/**
 * Global input logic virtual methods table.
 **/
typedef const struct AudioOutputLogicVTable_ {
    /**
     * virtual methods table of the base class
     **/
    struct SvObjectVTable_ super_;

    /**
     * Initialize audio logic instance.
     *
     * @private
     *
     * @param[in]  self       audio output logic handle
     * @param[out] errorOut   error info
     * @return                @a self or @c NULL in case of error
     **/
    AudioOutputLogic (*init)(AudioOutputLogic self,
                             SvErrorInfo *errorOut);

    /**
     * Set multi channel mode.
     *
     * @param[in] self               audio output logic handle
     * @param[in] isMultiChannelMode multi channel mode
     **/
    void (*setMultiChannelMode)(AudioOutputLogic self,
                                bool isMultiChannelMode);

    /**
     * Notify audio output preferences changed according to E-EDID info.
     *
     * @param[in] self             audio output logic handle
     * @param[in] videoOutputID    video output ID
     * @param[in] EDID             audio format description
     **/
    void (*HDMIReceiverChanged)(AudioOutputLogic self,
                                unsigned int videoOutputID,
                                QBEDIDBlock *EDID);
} *AudioOutputLogicVTable;

/**
 * Create AudioOutputLogic object
 *
 * @memberof AudioOutputLogic
 *
 * @param[out] errorOut             error info
 * @return AudioOutputLogic         created AudioOutputLogic object
 **/
extern AudioOutputLogic AudioOutputLogicCreate(SvErrorInfo *errorOut);

/**
 * Get runtime type identification object representing the AudioOutputLogic class.
 * @relates AudioOutputLogic
 *
 * @return AudioOutputLogic type identification object
 **/
SvType
AudioOutputLogic_getType(void);

/**
 * Set multi channel mode.
 *
 * @param[in] self               audio output logic handle
 * @param[in] isMultiChannelMode multi channel mode
 **/
static inline void
AudioOutputLogicSetMultiChannelMode(AudioOutputLogic self, bool isMultiChannelMode)
{
    return SvInvokeVirtual(AudioOutputLogic, self, setMultiChannelMode, isMultiChannelMode);
}

/**
 * Notify audio output preferences changed according to E-EDID info.
 *
 * @param[in] self             audio output logic handle
 * @param[in] videoOutputID    video output ID
 * @param[in] EDID             audio format description
 **/
static inline void
AudioOutputLogicHDMIReceiverChanged(AudioOutputLogic self, unsigned int videoOutputID, QBEDIDBlock *EDID)
{

    return SvInvokeVirtual(AudioOutputLogic, self, HDMIReceiverChanged, videoOutputID, EDID);
}

#endif
