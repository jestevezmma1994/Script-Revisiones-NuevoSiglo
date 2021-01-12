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

#include <QBAppKit/QBAsyncService.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvFoundation/SvString.h>
#include <Logic/AudioOutputLogic.h>
#include <Logic/QBLogicFactory.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBPlatformHAL/QBPlatformOutput.h>
#include <QBPlatformHAL/QBPlatformHDMIOutput.h>
#include <QBPlatformHAL/QBPlatformEDID.h>
#include <QBPlatformHAL/QBPlatformAudioOutput.h>
#include <QBConf.h>
#include <main.h>


/*
 * this table is completely dumb - it just says 'always output
 * non-multichannel PCM' - and it is used in following settings:
 *
 *  - for analog output this the only supported output format,
 *  - for all other outputs when 'multichannel' setting is OFF
 **/
static const QBAudioOutputFormat stereoOutputFormats[] = {
    {
        .input  = { .codec = QBAudioCodec_mpeg,   .multiChannel = false },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_ac3,    .multiChannel = false },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_ac3,    .multiChannel = true  },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_eac3,   .multiChannel = false },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_eac3,   .multiChannel = true  },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_dts,    .multiChannel = false },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_dts,    .multiChannel = true  },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_aac,    .multiChannel = false },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_aac,    .multiChannel = true  },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_heaac,  .multiChannel = false },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_heaac,  .multiChannel = true  },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }
};

// S/PDIF output format preferences as specified by NorDig Unified Requirements
// and Finnish Unified Requirements
static const QBAudioOutputFormat SPDIFOutputFormats[] = {
    {
        .input  = { .codec = QBAudioCodec_mpeg,   .multiChannel = false },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_ac3,    .multiChannel = false },
        .output = { .codec = QBAudioCodec_ac3,    .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_ac3,    .multiChannel = true  },
        .output = { .codec = QBAudioCodec_ac3,    .multiChannel = true  }
    }, {
        .input  = { .codec = QBAudioCodec_eac3,   .multiChannel = false },
        .output = { .codec = QBAudioCodec_ac3,    .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_eac3,   .multiChannel = true  },
        .output = { .codec = QBAudioCodec_ac3,    .multiChannel = true  }
    }, {
        .input  = { .codec = QBAudioCodec_dts,    .multiChannel = false },
        .output = { .codec = QBAudioCodec_dts,    .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_dts,    .multiChannel = true  },
        .output = { .codec = QBAudioCodec_dts,    .multiChannel = true  }
    }, {
        .input  = { .codec = QBAudioCodec_aac,    .multiChannel = false },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_aac,    .multiChannel = true  },
        .output = { .codec = QBAudioCodec_dts,    .multiChannel = true  }
    }, {
        .input  = { .codec = QBAudioCodec_heaac,  .multiChannel = false },
        .output = { .codec = QBAudioCodec_lpcm,   .multiChannel = false }
    }, {
        .input  = { .codec = QBAudioCodec_heaac,  .multiChannel = true  },
        .output = { .codec = QBAudioCodec_dts,    .multiChannel = true  }
    }
};

SvLocal bool
AudioOutputLogicIsMultiChannelModeActive(AudioOutputLogic self)
{
    const char *multiChannelModeStr = QBConfigGet("MULTICHANNELAUDIO");
    return (multiChannelModeStr && !strcmp(multiChannelModeStr, "YES"));
}

SvLocal void
AudioOutputLogicSetupSPDIFPreferences(AudioOutputLogic self,
                                      unsigned int audioOutputID,
                                      bool multiChannelMode)
{
    if (multiChannelMode) {
        SvLogNotice("AudioOutputLogic: enabling compressed output on S/PDIF");
        unsigned int cnt = sizeof(SPDIFOutputFormats) / sizeof(SPDIFOutputFormats[0]);
        QBPlatformSetAudioOutputPreferences(audioOutputID, SPDIFOutputFormats, cnt);
    } else {
        SvLogNotice("AudioOutputLogic: disabling compressed output on S/PDIF");
        unsigned int cnt = sizeof(stereoOutputFormats) / sizeof(stereoOutputFormats[0]);
        QBPlatformSetAudioOutputPreferences(audioOutputID, stereoOutputFormats, cnt);
    }
}

typedef struct {
    QBAudioCodec codec;
    bool multiChannel;
} HDMIOutputFormat;

SvLocal void
AudioOutputLogicSelectHDMIOutputFormat(AudioOutputLogic self,
                                       QBEDIDBlock *EDID,
                                       QBAudioOutputFormat *format,
                                       const HDMIOutputFormat *fallbacks)
{
    bool multiChannelInput = format->input.multiChannel;
    unsigned int i;

    for (i = 0; fallbacks[i].codec != QBAudioCodec_unknown; i++) {
        bool multiChannel = multiChannelInput && fallbacks[i].multiChannel;
        if (QBPlatformEDIDIsAudioFormatSupported(EDID, fallbacks[i].codec, multiChannel)) {
            format->output.codec = fallbacks[i].codec;
            format->output.multiChannel = multiChannel;
            return;
        }
    }

    // use LPCM stereo as last resort
    format->output.codec = QBAudioCodec_lpcm;
    format->output.multiChannel = false;
}

SvLocal void
AudioOutputLogicSetupHDMIPreferences(AudioOutputLogic self,
                                     unsigned int audioOutputID,
                                     QBEDIDBlock *EDID,
                                     bool multiChannelMode)
{
    if (!EDID || !multiChannelMode) {
        SvLogNotice("AudioOutputLogic: disabling compressed output on HDMI");
        unsigned int cnt = sizeof(stereoOutputFormats) / sizeof(stereoOutputFormats[0]);
        QBPlatformSetAudioOutputPreferences(audioOutputID, stereoOutputFormats, cnt);
        return;
    }

    SvLogNotice("AudioOutputLogic: selecting output formats on HDMI according to E-EDID");

    // copy S/PDIF preferences to temporary table and alter according to E-EDID info
    unsigned int i, cnt = sizeof(SPDIFOutputFormats) / sizeof(SPDIFOutputFormats[0]);
    QBAudioOutputFormat outputFormats[cnt];
    for (i = 0; i < cnt; i++) {
        outputFormats[i] = SPDIFOutputFormats[i];
        if (QBPlatformEDIDIsAudioFormatSupported(EDID, outputFormats[i].input.codec, outputFormats[i].input.multiChannel)) {
            // select pass-through
            outputFormats[i].output.codec = outputFormats[i].input.codec;
            outputFormats[i].output.multiChannel = outputFormats[i].input.multiChannel;
            continue;
        }
        // select trancoding and/or downmix options
        if (outputFormats[i].input.codec == QBAudioCodec_lpcm || outputFormats[i].input.codec == QBAudioCodec_mpeg) {
            outputFormats[i].output.codec = QBAudioCodec_lpcm;
            if (outputFormats[i].input.multiChannel && QBPlatformEDIDIsAudioFormatSupported(EDID, QBAudioCodec_lpcm, true))
                outputFormats[i].output.multiChannel = true;
            else
                outputFormats[i].output.multiChannel = false;
        } else if (outputFormats[i].input.codec == QBAudioCodec_ac3) {
            static const HDMIOutputFormat fallbacks[] = {
                { .codec = QBAudioCodec_ac3,    .multiChannel = true  },
                { .codec = QBAudioCodec_lpcm,   .multiChannel = true  },
                { .codec = QBAudioCodec_lpcm,   .multiChannel = false },
                { .codec = QBAudioCodec_unknown }
            };
            AudioOutputLogicSelectHDMIOutputFormat(self, EDID, &(outputFormats[i]), fallbacks);
        } else if (outputFormats[i].input.codec == QBAudioCodec_eac3) {
            static const HDMIOutputFormat fallbacks[] = {
                { .codec = QBAudioCodec_eac3,   .multiChannel = true  },
                { .codec = QBAudioCodec_ac3,    .multiChannel = true  },
                { .codec = QBAudioCodec_lpcm,   .multiChannel = true  },
                { .codec = QBAudioCodec_lpcm,   .multiChannel = false },
                { .codec = QBAudioCodec_unknown }
            };
            AudioOutputLogicSelectHDMIOutputFormat(self, EDID, &(outputFormats[i]), fallbacks);
        } else if (outputFormats[i].input.codec == QBAudioCodec_dts) {
            static const HDMIOutputFormat fallbacks[] = {
                { .codec = QBAudioCodec_lpcm,   .multiChannel = true  },
                { .codec = QBAudioCodec_lpcm,   .multiChannel = false },
                { .codec = QBAudioCodec_unknown }
            };
            AudioOutputLogicSelectHDMIOutputFormat(self, EDID, &(outputFormats[i]), fallbacks);
        } else if (outputFormats[i].input.codec == QBAudioCodec_aac) {
            static const HDMIOutputFormat fallbacks[] = {
                { .codec = QBAudioCodec_aac,    .multiChannel = true  },
                { .codec = QBAudioCodec_lpcm,   .multiChannel = true  },
                { .codec = QBAudioCodec_lpcm,   .multiChannel = false },
                { .codec = QBAudioCodec_unknown }
            };
            AudioOutputLogicSelectHDMIOutputFormat(self, EDID, &(outputFormats[i]), fallbacks);
        } else if (outputFormats[i].input.codec == QBAudioCodec_heaac) {
            static const HDMIOutputFormat fallbacks[] = {
                { .codec = QBAudioCodec_heaac,  .multiChannel = true  },
                { .codec = QBAudioCodec_lpcm,   .multiChannel = true  },
                { .codec = QBAudioCodec_lpcm,   .multiChannel = false },
                { .codec = QBAudioCodec_unknown }
            };
            AudioOutputLogicSelectHDMIOutputFormat(self, EDID, &(outputFormats[i]), fallbacks);
        }
        SvLogNotice("AudioOutputLogic:   using %s %s output for %s %s input",
                    QBAudioCodecToString(outputFormats[i].output.codec),
                    outputFormats[i].output.multiChannel ? "multi-channel" : "stereo",
                    QBAudioCodecToString(outputFormats[i].input.codec),
                    outputFormats[i].input.multiChannel ? "multi-channel" : "stereo");
    }

    QBPlatformSetAudioOutputPreferences(audioOutputID, outputFormats, cnt);
}

SvLocal void
AudioOutputLogicSetupOutputs(AudioOutputLogic self,
                             bool digitalOnly,
                             bool multiChannelMode)
{
    int outputID, outputsCount;
    QBAudioOutputConfig cfg;

    outputsCount = QBPlatformGetAudioOutputsCount();
    for (outputID = 0; outputID < outputsCount; outputID++) {
        if (QBPlatformGetAudioOutputConfig(outputID, &cfg) < 0)
            continue;

        if (cfg.type == QBAudioOutputType_analog && !digitalOnly) {
            // analog output
            unsigned int cnt = sizeof(stereoOutputFormats) / sizeof(stereoOutputFormats[0]);
            QBPlatformSetAudioOutputPreferences(outputID, stereoOutputFormats, cnt);
        } else if (cfg.type == QBAudioOutputType_SPDIF) {
            // S/PDIF output
            AudioOutputLogicSetupSPDIFPreferences(self, outputID, multiChannelMode);
        } else if (cfg.type == QBAudioOutputType_HDMI) {
            // HDMI output
            int HDMIOutputID = QBPlatformFindOutput(QBOutputType_HDMI, NULL, 0, true, false);
            if (HDMIOutputID >= 0) {
                QBEDIDBlock *EDID = NULL;
                bool isConnected = false;
                if (QBPlatformHDMIOutputCheckConnection(HDMIOutputID, &isConnected) >= 0 && isConnected)
                    EDID = QBPlatformHDMIOutputGetEDID(HDMIOutputID);
                AudioOutputLogicSetupHDMIPreferences(self, outputID, EDID, multiChannelMode);
                if (EDID)
                    free(EDID);
            }
        } else if (!digitalOnly) {
            // unknown output type, setup default preferences just like for analog output
            unsigned int cnt = sizeof(stereoOutputFormats) / sizeof(stereoOutputFormats[0]);
            QBPlatformSetAudioOutputPreferences(outputID, stereoOutputFormats, cnt);
        }
    }
}

SvLocal AudioOutputLogic
AudioOutputLogicInit_(AudioOutputLogic self,
                      SvErrorInfo *errorOut)
{
    SvLogNotice("AudioOutputLogic: performing initial setup");
    bool isMultiChannelMode = AudioOutputLogicIsMultiChannelModeActive(self);
    AudioOutputLogicSetupOutputs(self, false, isMultiChannelMode);
    self->state = QBAsyncServiceState_running;
    return self;
}

SvLocal void
AudioOutputLogicSetMultiChannelMode_(AudioOutputLogic self, bool isMultiChannelMode)
{
    SvLogNotice("AudioOutputLogic: multi-channel mode %s", isMultiChannelMode ? "ENABLED" : "DISABLED");
    AudioOutputLogicSetupOutputs(self, true, isMultiChannelMode);
}

SvLocal void
AudioOutputLogicHDMIReceiverChanged_(AudioOutputLogic self, unsigned int videoOutputID, QBEDIDBlock *EDID)
{
    int audioOutputID = QBPlatformFindAudioOutput(QBAudioOutputType_HDMI, NULL);
    if (audioOutputID >= 0) {
        bool multiChannelMode = AudioOutputLogicIsMultiChannelModeActive(self);
        AudioOutputLogicSetupHDMIPreferences(self, audioOutputID, EDID, multiChannelMode);
    }
}

SvLocal SvString
AudioOutputLogicGetName(SvObject self_)
{
    return SVSTRING("AudioOutputLogic");
}

SvLocal QBAsyncServiceState
AudioOutputLogicGetState(SvObject self_)
{
    AudioOutputLogic self = (AudioOutputLogic) self_;
    return self->state;
}

SvLocal void
AudioOutputLogicStop(SvObject self_, SvErrorInfo *errorOut)
{
    AudioOutputLogic self = (AudioOutputLogic) self_;
    self->state = QBAsyncServiceState_idle;
}

SvLocal void
AudioOutputLogicStart(SvObject self_, SvScheduler scheduler, SvErrorInfo *errorOut)
{
}

SvLocal SvImmutableArray
AudioOutputLogicGetDependencies(SvObject self_)
{
    return NULL;
}

SvType
AudioOutputLogic_getType(void)
{
    static SvType type = NULL;
    static const struct QBAsyncService_ serviceMethods = {
        .getName         = AudioOutputLogicGetName,
        .getState        = AudioOutputLogicGetState,
        .getDependencies = AudioOutputLogicGetDependencies,
        .start           = AudioOutputLogicStart,
        .stop            = AudioOutputLogicStop
    };
    static const struct AudioOutputLogicVTable_ audioOutputVTable = {
        .init                = AudioOutputLogicInit_,
        .setMultiChannelMode = AudioOutputLogicSetMultiChannelMode_,
        .HDMIReceiverChanged = AudioOutputLogicHDMIReceiverChanged_
    };
    if (!type) {
        type = SvTypeCreateVirtual("AudioOutputLogic", sizeof(struct AudioOutputLogic_),
                                   SvObject_getType(),
                                   sizeof(audioOutputVTable), &audioOutputVTable, &type,
                                   QBAsyncService_getInterface(), &serviceMethods,
                                   NULL);
    }
    return type;
}

AudioOutputLogic AudioOutputLogicCreate(SvErrorInfo *errorOut)
{
    AudioOutputLogic self = NULL;
    SvErrorInfo error = NULL;

    SvType logicClass = QBLogicFactoryFindImplementationOf(QBLogicFactoryGetInstance(),
                                                           AudioOutputLogic_getType(), &error);
    if (!logicClass)
        goto fini;

    self = (AudioOutputLogic) SvTypeAllocateInstance(logicClass, &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory,
                                           error, "can't allocate %s", SvTypeGetName(logicClass));
        goto fini;
    }
    self->state = QBAsyncServiceState_starting;
    SvLogNotice("%s(): created instance of %s", __func__,
                SvTypeGetName(logicClass));

    if (!SvInvokeVirtual(AudioOutputLogic, self, init, &error)) {
        SVRELEASE(self);
        self = NULL;
        goto fini;
    }
fini:
    SvErrorInfoPropagate(error, errorOut);
    return self;
}
