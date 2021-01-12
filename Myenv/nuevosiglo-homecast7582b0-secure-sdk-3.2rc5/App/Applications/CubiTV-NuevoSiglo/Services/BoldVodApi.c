
#include "BoldVodApi.h"

#include "BoldContentTracking.h"
#include "BoldFoxApi.h"
#include "BoldHboApi.h"
#include "BoldHttpApi.h"
#include "BoldTurnerApi.h"

#include <libintl.h>
#include <main.h>
#include <QBConf.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBICSmartcardInfo.h>
#include <QBICSmartcardSubscription.h>
#include <QBWidgets/QBDialog.h>
#include <QBSmartcard2Interface.h>
#include <regex.h>
#include <settings.h>
#include <Utils/authenticators.h>
#include <Utils/dbobject.h>
#include <Widgets/authDialog.h>
#include <Widgets/confirmationDialog.h>
#include <Services/BoldUsageLog.h>

struct BoldVodApi_
{
    struct SvObject_     super_;
    AppGlobals           appGlobals;
    
    BoldVodServiceType   serviceType;
    BoldFoxApi           api_fox;
    BoldTurnerApi        api_turner;
    BoldHboApi           api_hbo;
    BoldVodCallback      callback;
    
    void * callbackData;
    QBCASCmd infoCmd;
    SvString publicUri;
    SvWidget dialog;
    SvWidget activationDialog;
};

SvLocal void
BoldVodApiDestroy(void *self_)
{
    BoldVodApi self = self_;
    SVTESTRELEASE(self->api_turner);
    SVTESTRELEASE(self->api_fox);
    SVTESTRELEASE(self->api_hbo);
}

SvLocal void
BoldVodApiProgressDialogCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    BoldVodApi self = self_;
    self->dialog = NULL;
    
    if (self->serviceType == BoldVodServiceFOX_hls || self->serviceType == BoldVodServiceFOX_ss || self->serviceType == BoldVodServiceFOX_ss_v3)
    {
        BoldFoxCancelHttpReq(self->api_fox);
    }
    else if (self->serviceType == BoldVodServiceHBO)
    {
        BoldHboCancelHttpReq(self->api_hbo);
    }
    else if (self->serviceType == BoldVodServiceTurner)
    {
        BoldTurnerCancelHttpReq(self->api_turner);
    }
}


SvLocal void
BoldVodApiSetupProgressDialog(BoldVodApi self)
{
    svSettingsPushComponent("VoDGrid.settings");
    if (!svSettingsIsWidgetDefined("Dialog"))
    {
        svSettingsPopComponent();
        return;
    }
    self->dialog = QBAnimationDialogCreate(self->appGlobals->res, self->appGlobals->controller);
    QBDialogRun(self->dialog, self, BoldVodApiProgressDialogCallback);
    svSettingsPopComponent();
}


SvLocal void 
BoldVodApiSmartCardCallbackInfo(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    BoldVodApi self = (BoldVodApi) self_;
    SvString deviceId;

    self->infoCmd = NULL;

    if (status != 0)
    {
        SvLogNotice("%s: Unable to get card number", __func__);
        return;
    }
    
    deviceId = info ? QBCASInfoGetDescriptiveCardNum(info) : NULL;
    if (deviceId)
    {
        BoldFoxSetDeviceId(self->api_fox, deviceId);
        BoldHboSetDeviceId(self->api_hbo, deviceId);
        BoldTurnerSetDeviceId(self->api_turner, deviceId);
        SvLogNotice("%s: Got card number as Device Id: %s", __func__, SvStringCString(deviceId));
    }
    
    // AMERELES #2525 Reporte vía syslog de eventos tuner con SCID = "Unknown"
    BoldUsageLogUpdateIds(deviceId ? deviceId : SVSTRING("unknown"), info->chip_id ? info->chip_id : SVSTRING("unknown"));
    BoldUsageLogSendInitialLog(); // AMERELES #2529 Cambios en lógica de envío de reportes TUNER vía Syslog
    BoldContentTrackingUpdateIds(deviceId ? deviceId : SVSTRING("unknown"));
}


SvLocal void 
BoldVodApiSmartCardUpdate (BoldVodApi self)
{
    if (self->infoCmd)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->infoCmd);

    SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, BoldVodApiSmartCardCallbackInfo, self, &self->infoCmd);
}

SvLocal void 
BoldVodApiSmartCardState(void *self_, QBCASSmartcardState state)
{
    BoldVodApi self = self_;
    BoldVodApiSmartCardUpdate(self);
}

SvLocal void
BoldVodCallbackResp(void * _self, SvString param)
{
    BoldVodApi self = (BoldVodApi) _self;
    if (self->dialog)
    {
        QBDialogBreak(self->dialog);
    }
    if (self->callback != NULL)
    {
        self->callback(self->callbackData, param);
    }
}

SvLocal void
BoldVodVentaPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    BoldVodApi self  = (BoldVodApi) self_;
    if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button"))
    {
        if (self->serviceType == BoldVodServiceFOX_hls || self->serviceType == BoldVodServiceFOX_ss || self->serviceType == BoldVodServiceFOX_ss_v3)
        {
            self->callback = NULL;
            BoldVodApiSetupProgressDialog(self);
            BoldFoxVtaPaq(self->api_fox, self, BoldVodCallbackResp);
        }
        else if (self->serviceType == BoldVodServiceHBO){
            self->callback = NULL;
            BoldVodApiSetupProgressDialog(self);
            BoldHboVtaPaq(self->api_hbo, self, BoldVodCallbackResp); 
        }
        else
        {
            SvLogError ("%s: Unknown service type", __func__);
        }
    }
    else
    {
        if (self->serviceType == BoldVodServiceFOX_hls || self->serviceType == BoldVodServiceFOX_ss || self->serviceType == BoldVodServiceFOX_ss_v3)
        {
            BoldFoxVtaPaqCancel(self->api_fox);
        }
        else if (self->serviceType == BoldVodServiceHBO)
        {
            BoldHboVtaPaqCancel(self->api_hbo);
        }
        else
        {
            SvLogError ("%s: Unknown service type", __func__);
        }
    }
    self->activationDialog = NULL;
}

SvLocal void
BoldVodVentaOkPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    BoldVodApi self  = (BoldVodApi) self_;
    self->activationDialog = NULL;
    
    if (self->serviceType == BoldVodServiceFOX_hls || self->serviceType == BoldVodServiceFOX_ss || self->serviceType == BoldVodServiceFOX_ss_v3)
    {
        BoldFoxVtaPaqResp(self->api_fox);
    }
    else if (self->serviceType == BoldVodServiceHBO)
    {
        BoldHboVtaPaqResp(self->api_hbo);
    }
    else
    {
        SvLogError ("%s: Unknown service type", __func__);
    }
}

SvLocal void
BoldVodActivationPopupCallback(void *self_, SvWidget dialog, SvString buttonTag, unsigned keyCode)
{
    BoldVodApi self  = (BoldVodApi) self_;
    self->activationDialog = NULL;
}


SvLocal SvWidget
BoldVodActivationCreatePopup(BoldVodApi self, const char *title, const char *message)
{
    QBConfirmationDialogParams_t params =
    {
            .title = title,
            .message = message,
            .local = true,
            .focusOK = true,
            .isCancelButtonVisible = false
    };

    svSettingsPushComponent("Dialog.settings");
    SvWidget confirmationDialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);
    svSettingsPopComponent();

    return confirmationDialog;
}

////////////////////////////////////////////////////// Public functions ////////////////////////////////////////////////////////
SvType
BoldVodApi_getType(void)
{
    static const struct SvObjectVTable_ objectVTable =
    {
      .destroy = BoldVodApiDestroy
    };
    
    static SvType myType = NULL;

    if (unlikely(!myType))
    {
        SvTypeCreateManaged("BoldVodApi",
                sizeof(struct BoldVodApi_),
                SvObject_getType(), &myType,
                SvObject_getType(), &objectVTable,
                NULL);
    }
    return myType;
}

BoldVodApi 
BoldVodApiCreate(AppGlobals appGlobals) 
{
    BoldVodApi self = (BoldVodApi) SvTypeAllocateInstance(BoldVodApi_getType(), NULL);
    self->appGlobals = appGlobals;
    BoldHttpApi httpapi = BoldHttpApiCreate();
    self->api_fox = BoldFoxApiCreate(appGlobals, self, httpapi);
    self->api_hbo = BoldHboApiCreate(appGlobals, self, httpapi);
    self->api_turner = BoldTurnerApiCreate(appGlobals, self, httpapi);
    BoldContentTrackingInitialize(appGlobals, httpapi);
    
    return self;
}


static const struct QBCASCallbacks_s s_smartcard_callbacks =
{
    .smartcard_state = BoldVodApiSmartCardState
};


int 
BoldVodApiStart(BoldVodApi self)
{
    SvLogNotice ("%s: Starting ...", __func__);
    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks)&s_smartcard_callbacks, self, NULL);
    BoldVodApiSmartCardUpdate(self);
    return 0;
}

BoldVodServiceType
BoldVodGetServiceType(BoldVodApi self)
{
    return self->serviceType;
}

void
BoldVodProductHandler(BoldVodApi self, SvDBRawObject product, void * callbackData, BoldVodCallback callback)
{
    const char * uri;
    if (!product)
    {
        goto fini;
    }
    self->callback = callback;
    self->callbackData = callbackData;

    BoldVodApiSetupProgressDialog(self);

    SvLogNotice("%s : Product %s", __func__, product ? "NOT NULL": "NULL");
    
    SVTESTRELEASE(self->publicUri);
    uri = SvDBRawObjectGetStringAttrValue(product, "source", NULL);
    self->publicUri = SvStringCreate(uri, NULL);
   
    if (uri)
    {
        SvLogNotice("%s : Public URI: %s", __func__, uri);
    }
    else
    {
        SvLogNotice("%s : Public URI:NULL", __func__);
        goto fini;
    }
    self->serviceType = BoldFoxGetType(self->api_fox, uri);
    if (self->serviceType == BoldVodServiceFOX_hls || self->serviceType == BoldVodServiceFOX_ss || self->serviceType == BoldVodServiceFOX_ss_v3)
    {
        SvLogNotice("%s : Vod type: FOX", __func__);
        BoldFoxHandler(self->api_fox, self->publicUri, self, BoldVodCallbackResp);
        return;
    }
    self->serviceType = BoldHboGetType(self->api_hbo, uri);
    if (self->serviceType == BoldVodServiceHBO)
    {
        SvLogNotice("%s : Vod type: HBO", __func__);
        BoldHboHandler(self->api_hbo, self->publicUri, self, BoldVodCallbackResp);
        return;
    } 
    self->serviceType = BoldTurnerGetType(self->api_turner, uri);
    if (self->serviceType == BoldVodServiceTurner)
    {
        SvLogNotice("%s : Vod type: Turner", __func__);
        BoldTurnerHandler(self->api_turner, self->publicUri, self, BoldVodCallbackResp);
        return;
    }
    SvLogNotice("%s : Vod type: UNKNOWN", __func__);
fini:
    callback(callbackData, NULL);
}



SvString 
BoldVodHboGetCustomData(BoldVodApi self)
{
    return BoldHboGetCustomData(self->api_hbo);
}

SvString 
BoldVodHboGetLink(BoldVodApi self)
{
    return BoldHboGetLink(self->api_hbo);
}

SvString 
BoldVodFoxGetPlayreadyParams(BoldVodApi self)
{
    return BoldFoxGetPlayreadyParams(self->api_fox);
}

SvString 
BoldVodFoxGetLink(BoldVodApi self)
{
    return BoldFoxGetLink(self->api_fox);
}

SvString 
BoldVodTurnerGetPlayreadyParams(BoldVodApi self)
{
    return BoldTurnerGetPlayreadyParams(self->api_turner);
}

SvString 
BoldVodTurnerGetLink(BoldVodApi self)
{
    return BoldTurnerGetLink(self->api_turner);
}

void
BoldVodPopup (BoldVodApi self, const char * title, const char*message)
{
    self->activationDialog = BoldVodActivationCreatePopup(self, title, message);

    if (self->activationDialog) {
        QBDialogRun(self->activationDialog, self, BoldVodActivationPopupCallback);
    }
}

void
BoldVodVentaOkPopup (BoldVodApi self, const char * title, const char*message)
{
    QBConfirmationDialogParams_t params =
    {
            .title = title,
            .message = message,
            .local = true,
            .focusOK = true,
            .isCancelButtonVisible = false
    };

    svSettingsPushComponent("Dialog.settings");
    self->activationDialog = QBConfirmationDialogCreate(self->appGlobals->res, &params);
    svSettingsPopComponent();

    if (self->activationDialog)
    {
        QBDialogRun(self->activationDialog, self, BoldVodVentaOkPopupCallback);
    }
}

void
BoldVodVentaPopup (BoldVodApi self, const char * title, const char* message)
{
    SvString domain = NULL;
    const char *forcedDomain = getenv("INNOV8ON_VOD_AUTH");
    if (forcedDomain) {
        domain = SvStringCreate(forcedDomain, NULL);
    } else {
        domain = SvStringCreate("VOD_AUTH", NULL);
    }
    SvObject authenticator = NULL;
    authenticator = QBAuthenticateViaAccessManager(self->appGlobals->scheduler,
                                                   self->appGlobals->accessMgr,
                                                   domain);
    SVTESTRELEASE(domain);
    
    svSettingsPushComponent("AuthenticationDialog.settings");
    
    const char *PURCHASE_AUTH = QBConfigGet("PURCHASE_AUTH");
    bool purchaseAuth = !PURCHASE_AUTH || !(strcmp(PURCHASE_AUTH, "off") == 0);
    self->activationDialog = QBConfCondAuthDialogCreate(self->appGlobals, authenticator, 
                                      title, message, true, NULL, NULL, 
                                      gettext("Buy"), gettext("Exit"), purchaseAuth, true);
    
    svSettingsPopComponent();
    
    if (self->activationDialog)
    {
        QBDialogRun(self->activationDialog, self, BoldVodVentaPopupCallback);
    }
}

