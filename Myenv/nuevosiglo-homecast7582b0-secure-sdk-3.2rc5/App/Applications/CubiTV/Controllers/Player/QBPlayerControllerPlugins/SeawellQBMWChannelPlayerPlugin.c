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

#include "SeawellQBMWChannelPlayerPlugin.h"

#include <stdbool.h>
#include <string.h>

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvCoreErrorDomain.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvTime.h>
#include <QBStringUtils.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvURI.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvValue.h>
#include <SvPlayerKit/SvContent.h>
#include <CubiwareMWClient/Controllers/Player/QBMWProductUsageController.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGChannelListPlugin.h>
#include <QBPlayerControllers/QBPlayerControllerPlugin.h>
#include <QBPlayerControllers/utils/content.h>

SV_DECL_INT_ENV_FUN_DEFAULT(env_MWChPlayerControllerLogLevel, 0, "MWChPlayerControllerLogLevel", "")
SV_DECL_INT_ENV_FUN_DEFAULT(env_UseProductCachingMarginSec, 0, "UseProductCachingMarginSec", "")

#define log_debug(fmt, ...)     if (env_MWChPlayerControllerLogLevel() > 0) { SvLogNotice(COLBEG() "SeawellQBMWChannelPlayerPlugin %s :: " fmt COLEND_COL(blue), __func__, ## __VA_ARGS__); }

#define CACHING_TIME_MARGIN_SEC     (1 * 60)    // 1 minute

struct SeawellQBMWChannelPlayerPlugin_ {
    struct SvObject_ super_;

    QBMWProductUsageController qbMWProductUsageController;
    QBMiddlewareManager middlewareManager;
    QBMWConfigHandler qbMWConfigHandler;
    SvGenericObject dvbChannelList;
    SvGenericObject ipChannelList;

    SvTVChannel channel;
    SvContent content;
    int answerValidityMarginSec;

    bool canInitPlayback;       ///< are we ready for playback init?
    bool usingCachedAnswer;
};


//-------------------------------------------------------
SvLocal QBPlayerControllerPluginAnswerHandlingResult
SeawellQBMWChannelPlayerPluginHandleAnswer(SvGenericObject self_, SvHashTable answer, SvObject listener);


//-------------------------------------------------------

SvLocal bool
SeawellQBMWChannelPlayerPluginIsTokenValid(SeawellQBMWChannelPlayerPlugin self, SvValue validUntilV)
{
    if (!validUntilV) {
        return false;
    }

    SvString validUntilStr = SvValueGetString(validUntilV);
    SvTime validUntil;
    if (QBStringToDateTime(SvStringCString(validUntilStr), QBDateTimeFormat_ISO8601, false, &validUntil) < 0) {
        log_debug("Token's valid-until date '%s' is not a valid ISO8601 date", SvStringCString(validUntilStr));
        return false;
    }

    return (SvTimeGetSeconds(SvTimeSub(validUntil, SvTimeGetCurrentTime())) > self->answerValidityMarginSec);
}

SvLocal bool
SeawellQBMWChannelPlayerPluginCheckCachedAnswer(SeawellQBMWChannelPlayerPlugin self)
{
    SvGenericObject cachedAnswer = QBMWProductUsageControllerGetCachedAnswer(self->qbMWProductUsageController, (SvGenericObject) self->channel);
    if (!cachedAnswer || !SvObjectIsInstanceOf(cachedAnswer, SvHashTable_getType())) {
        log_debug("Channel wasn't found in cache - waiting on UseProduct answer");
        return false;
    }

    log_debug("Cached answer found");

    SvValue authTokenValidUntilV = (SvValue) SvHashTableFind((SvHashTable) cachedAnswer, (SvGenericObject) SVSTRING("auth_token_valid_until"));
    if (SeawellQBMWChannelPlayerPluginIsTokenValid(self, authTokenValidUntilV)) {
        log_debug("Using cached answer");
        SeawellQBMWChannelPlayerPluginHandleAnswer((SvGenericObject) self, (SvHashTable) cachedAnswer, NULL);
        self->usingCachedAnswer = true;
        return true;
    }

    log_debug("Cached answer expired");
    return false;
}

SvLocal void
SeawellQBMWChannelPlayerPluginUpdateContentURI(SeawellQBMWChannelPlayerPlugin self, SvValue authTokenV)
{
    bool isURIUpdated = false;
    SvURI newURI = createUpdatedIpContentURI(self->middlewareManager, self->content, SvValueGetString((SvValue) authTokenV), &isURIUpdated);
    if (newURI && isURIUpdated) {
        SvContentSetURI(self->content, newURI);
    } else if (isURIUpdated) {
        SvLogNotice("%s received auth_token but couldn't create final URI (self->content=%p)", __func__, self->content);
    }
    SVTESTRELEASE(newURI);
}

SvLocal bool
SeawellQBMWChannelPlayerPluginCheckMetadata(SeawellQBMWChannelPlayerPlugin self)
{
    SvValue authTokenValidUntilV = (SvValue) SvTVChannelGetAttribute(self->channel, SVSTRING("auth_token_valid_until"));
    if (!authTokenValidUntilV) {
        log_debug("auth_token_valid_until is not available in channel metadata (channel '%s')", self->channel->name ? SvStringCString(self->channel->name) : "--");
        return false;
    }

    SvGenericObject authTokenV = SvTVChannelGetAttribute(self->channel, SVSTRING("auth_token"));
    if (!authTokenV || !SvObjectIsInstanceOf(authTokenV, SvValue_getType()) || !SvValueIsString((SvValue) authTokenV)) {
        log_debug("auth_token is not available in channel metadata (channel '%s')", self->channel->name ? SvStringCString(self->channel->name) : "--");
        return false;
    }

    if (SeawellQBMWChannelPlayerPluginIsTokenValid(self, authTokenValidUntilV) && authTokenV) {
        SeawellQBMWChannelPlayerPluginUpdateContentURI(self, (SvValue) authTokenV);

        log_debug("Using channel metadata token. (channel '%s', token is valid until '%s')",
                  self->channel->name ? SvStringCString(self->channel->name) : "--",
                  authTokenValidUntilV && SvObjectIsInstanceOf((SvObject) authTokenValidUntilV, SvValue_getType()) && SvValueIsString((SvValue) authTokenValidUntilV) ? SvStringCString(SvValueGetString(authTokenValidUntilV)) : "--");
        return true;
    }

    log_debug("Channel metadata token invalid. (channel '%s', token was valid until '%s')",
              self->channel->name ? SvStringCString(self->channel->name) : "--",
              authTokenValidUntilV && SvObjectIsInstanceOf((SvObject) authTokenValidUntilV, SvValue_getType()) && SvValueIsString((SvValue) authTokenValidUntilV) ? SvStringCString(SvValueGetString(authTokenValidUntilV)) : "--");

    return false;
}

SvLocal bool
SeawellQBMWChannelPlayerPluginPrepareContentToPlayback(SvGenericObject self_, SvContent content, SvObject channel)
{
    SeawellQBMWChannelPlayerPlugin self = (SeawellQBMWChannelPlayerPlugin) self_;

    if (!channel || !content || !SvObjectIsInstanceOf(channel, SvTVChannel_getType())) {
        return false;
    }

    bool canInitPlayback = true;

    SVTESTRELEASE(self->channel);
    self->channel = SVRETAIN(channel);
    SVTESTRELEASE(self->content);
    self->content = SVRETAIN(content);

    // check if we have valid token in channel metadata
    bool tokensAvailableInLineup = false;
    if (self->qbMWConfigHandler) {
        SeawellQBMWConfigHandlerPlugin configPlugin = (SeawellQBMWConfigHandlerPlugin) QBMWConfigHandlerGetPluginByName(self->qbMWConfigHandler, SVSTRING(QBMW_CONFIG_HANDLER_PLUGIN_SEAWELL));
        if (configPlugin) {
            tokensAvailableInLineup = SeawellQBMWConfigHandlerPluginAuthTokensInLineupEnabled(configPlugin);
        }
    }

    if (!tokensAvailableInLineup || !SeawellQBMWChannelPlayerPluginCheckMetadata(self)) {
        // no valid token in channel metadata - refresh channel lineup
        SvGenericObject channelListPlugin = self->ipChannelList ? : self->dvbChannelList;
        if (tokensAvailableInLineup && channelListPlugin) {
            SvInvokeInterface(SvEPGChannelListPlugin, channelListPlugin, update, NULL);
        }

        // and check if we have cached data from previous UseProduct requests
        self->usingCachedAnswer = SeawellQBMWChannelPlayerPluginCheckCachedAnswer(self);
        if (!self->usingCachedAnswer) {
            canInitPlayback = false;
        }
    }
    self->canInitPlayback = canInitPlayback;
    return canInitPlayback;
}

SvLocal QBPlayerControllerPluginAnswerHandlingResult
SeawellQBMWChannelPlayerPluginHandleAnswer(SvGenericObject self_, SvHashTable answer, SvGenericObject listener)
{
    SeawellQBMWChannelPlayerPlugin self = (SeawellQBMWChannelPlayerPlugin) self_;

    if (!self || !self->content) {
        return QBPlayerControllerPluginAnswerHandlingResult_error;
    }

    QBPlayerControllerPluginAnswerHandlingResult ret = QBPlayerControllerPluginAnswerHandlingResult_ok;
    bool gotAuth = false;
    bool authValidUntilFound = false;

    SvGenericObject authTokenV = SvHashTableFind((SvHashTable) answer, (SvGenericObject) SVSTRING("auth_token"));
    if (likely(authTokenV && SvObjectIsInstanceOf(authTokenV, SvValue_getType()) && SvValueIsString((SvValue) authTokenV))) {
        if (!self->usingCachedAnswer) {
            SeawellQBMWChannelPlayerPluginUpdateContentURI(self, (SvValue) authTokenV);
        }
        gotAuth = true;
    }

    // if we are using cached answer then just verify it and update the cache

    //TODO: check auth_token_valid_from
    SvGenericObject authTokenValidUntilV = SvHashTableFind((SvHashTable) answer, (SvGenericObject) SVSTRING("auth_token_valid_until"));
    if (likely(authTokenValidUntilV &&
               SvObjectIsInstanceOf(authTokenValidUntilV, SvValue_getType()) &&
               SvValueIsString((SvValue) authTokenValidUntilV))) {
        authValidUntilFound = true;
    }

    if (self->channel) {
        SvString urlString = SvURLString(self->channel->sourceURL);
        if ((strncmp("dvb://", SvStringCString(urlString), 6) != 0) &&
            strstr(SvStringCString(urlString), "<auth_token>")) {

            if (!gotAuth) {
                SvLogWarning("%s Expected <auth_token> wasn't' provided", __func__);
                ret = QBPlayerControllerPluginAnswerHandlingResult_error;
            }

            if (!authValidUntilFound) {
                SvLogWarning("%s Expected <auth_token_valid_from>, <auth_token_valid_until> weren't provided", __func__);
            }
        }
    }

    if (authValidUntilFound && (ret != QBPlayerControllerPluginAnswerHandlingResult_error)) {
        QBMWProductUsageControllerAddAnswerToCache(self->qbMWProductUsageController, (SvGenericObject) self->channel, (SvGenericObject) answer);
    }

    return ret;
}

SvLocal void
SeawellQBMWChannelPlayerPluginStart(SvGenericObject self_)
{
    SeawellQBMWChannelPlayerPlugin self = (SeawellQBMWChannelPlayerPlugin) self_;
    self->canInitPlayback = false;
}

SvLocal void
SeawellQBMWChannelPlayerPluginStop(SvGenericObject self_)
{
    SeawellQBMWChannelPlayerPlugin self = (SeawellQBMWChannelPlayerPlugin) self_;
    SVTESTRELEASE(self->channel);
    self->channel = NULL;
    SVTESTRELEASE(self->content);
    self->content = NULL;
}

SvLocal bool
SeawellQBMWChannelPlayerPluginHasLicense(SvGenericObject self_)
{
    SeawellQBMWChannelPlayerPlugin self = (SeawellQBMWChannelPlayerPlugin) self_;
    return self->canInitPlayback;
}

SvLocal void
SeawellQBMWChannelPlayerPluginDestroy(void *self_)
{
    SeawellQBMWChannelPlayerPluginStop(self_);
}

SvLocal SvType
SeawellQBMWChannelPlayerPlugin_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = SeawellQBMWChannelPlayerPluginDestroy
    };
    static SvType type = NULL;

    static const struct QBPlayerControllerPlugin_ mwChannelPlayerMethods = {
        .prepareContentToPlayback = SeawellQBMWChannelPlayerPluginPrepareContentToPlayback,
        .handleAnswer             = SeawellQBMWChannelPlayerPluginHandleAnswer,
        .start                    = SeawellQBMWChannelPlayerPluginStart,
        .stop                     = SeawellQBMWChannelPlayerPluginStop,
        .hasLicense               = SeawellQBMWChannelPlayerPluginHasLicense
    };

    if (!type) {
        SvTypeCreateManaged("SeawellQBMWChannelPlayerPlugin",
                            sizeof(struct SeawellQBMWChannelPlayerPlugin_),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            QBPlayerControllerPlugin_getInterface(), &mwChannelPlayerMethods,
                            NULL);
    }

    return type;
}

SeawellQBMWChannelPlayerPlugin
SeawellQBMWChannelPlayerPluginCreate(QBMWProductUsageController qbMWProductUsageController,
                                     QBMiddlewareManager middlewareManager,
                                     QBMWConfigHandler qbMWConfigHandler,
                                     SvGenericObject dvbChannelList,
                                     SvGenericObject ipChannelList,
                                     SvErrorInfo *errorOut)
{
    SvErrorInfo error = NULL;
    SeawellQBMWChannelPlayerPlugin self = (SeawellQBMWChannelPlayerPlugin) SvTypeAllocateInstance(SeawellQBMWChannelPlayerPlugin_getType(), &error);
    if (!self) {
        error = SvErrorInfoCreateWithCause(SvCoreErrorDomain, SvCoreError_noMemory, error,
                                           "can't allocate SeawellQBMWChannelPlayerPlugin");
        goto fini;
    }

    self->canInitPlayback = false;
    self->qbMWProductUsageController = qbMWProductUsageController;
    self->middlewareManager = middlewareManager;
    self->qbMWConfigHandler = qbMWConfigHandler;
    self->dvbChannelList = dvbChannelList;
    self->ipChannelList = ipChannelList;
    self->answerValidityMarginSec = (env_UseProductCachingMarginSec() > 0) ? env_UseProductCachingMarginSec() : CACHING_TIME_MARGIN_SEC;

fini:
    SvErrorInfoPropagate(error, errorOut);
    return error ? NULL : self;
}

