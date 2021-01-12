
#include "BoldTurnerApi.h"

#include <QBConf.h>
#include <Utils/dbobject.h>
#include <QBWidgets/QBDialog.h>
#include <Widgets/confirmationDialog.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <QBSmartcard2Interface.h>
#include <QBICSmartcardInfo.h>
#include <QBICSmartcardSubscription.h>
#include <regex.h>

#define TOKEN_LEN 512

#define PROXY_OK        0
#define PROXY_ERROR     1
#define PROXY_VTA_PTE   2
#define PROXY_VTA_OK    3
#define PROXY_VTA_ERROR 4

struct BoldTurnerApi_ {
    struct SvObject_ super_;
    BoldHttpApi api;
    BoldVodApi vod_api;
    AppGlobals appGlobals;
    BoldTurnerCallback callback;
    void * callbackData;
    bool response_ok;
    
    SvWidget BoldTurnerActivationDialog;
    QBCASCmd infoCmd;
    
    SvString proxy;
    SvString pr_params;
    SvString publicUri;
    SvString deviceId;
    SvString url_pattern;
};
SvLocal bool  
BoldCheck(const char * uri); 

SvLocal void 
BoldTurnerErrorPopup (BoldTurnerApi self, const char *sz_tmpBuff);


SvLocal char *
clean(const char * orig, char * dst)
{
    char * tmp;
    tmp = dst;

    while (*orig != '-')
    {
        if ((*orig >= 48) && (*orig  <= 57))
        {
            *tmp = *orig;
            tmp++;
        } 
        orig++;
    }
    *tmp = '\0';
    return dst; 
}

SvLocal int
get_vta_paq_response (const char* resp, size_t resp_len, char* url, char * txt)
{
    unsigned int idx=0;
    int init = 0;
    int result = PROXY_VTA_ERROR;
    
    if (resp == NULL) {
        return PROXY_ERROR;
    }

    if ( strncmp(resp, "OK:", 3) == 0) {
        init = 3;    
        result = PROXY_VTA_OK;
    }
    
     
    while (idx < resp_len)
    {
        if (resp[idx] == '|')
        {
            memcpy(url, &resp[init], idx);
            memcpy(txt, &resp[init+idx+1], (resp_len-idx));
            break;
        }
        idx++; 
    } 
    return result;
}

SvLocal int
get_response (const char* resp, size_t resp_len, char* url, char * tkn_txt)
{
    unsigned int idx=0;

    if (resp == NULL) {
        return PROXY_ERROR;
    }
    
   SvLogError("%s: RESP: %s",__func__, resp);
     
    if (strncmp(resp, "ERROR:", 6) == 0) 
    {
        SvLogError("%s: DENTRO DE ERROR",__func__);
        while (idx < resp_len)
        {
            if (resp[idx] == '|')
            {
                memcpy(url, &resp[6], idx-6);
                memcpy(tkn_txt, &resp[idx+1], (resp_len-idx));
                break;
            }
            idx++; 
        } 
        return PROXY_ERROR;
    } 
    else if (strncmp (resp, "VTA:", 4) == 0) 
    {
        SvLogError("%s: DENTRO DE VTA",__func__);
        memcpy(tkn_txt, &resp[5], (resp_len-5));
        return PROXY_VTA_PTE;
    } 
    else 
    {
        SvLogError("%s: DENTRO DE OK",__func__);
        while (idx < resp_len)
        {
            if (resp[idx] == '|')
            {
                memcpy(url, resp, idx);
                memcpy(tkn_txt, &resp[idx+1], (resp_len-idx));
                break;
            }
            idx++; 
        } 
        return PROXY_OK;
    }
}


SvLocal void
VtaPaqCallback (void * self_,  char* buff, size_t len)
{
    BoldTurnerApi self = (BoldTurnerApi) self_;
    char txt[192];
    char url[192];
    int resp = 0;
 
    memset(url, 0, 192);
    memset(txt, 0, 192);
    
    resp = get_vta_paq_response(buff, len, url, txt);
    
    SvLogError("%s: RESP: %d, url: %s, txt: %s",__func__, resp, url, txt);
     
    if (resp == PROXY_VTA_OK)
    {
            BoldVodVentaOkPopup(self->vod_api, "TURNER", txt);
    }
    else
    {
        if (url[0] != 0) {
            SvLogError("%s: url distinto cero",__func__);
            // Show custom video error
            SVTESTRELEASE(self->publicUri);
            self->publicUri = SvStringCreate(url, NULL);
        }
        else if (txt[0] != 0)
        {
            SvLogError("%s: txt distinto cero",__func__);
            BoldVodPopup(self->vod_api, "TURNER", txt);
        }
        else
        {
            BoldTurnerErrorPopup (self, "990001");
        }
    }
    self->callback(self->callbackData, NULL);
    self->callback = NULL;
}

SvLocal void
GetLinkTokenCallback (void * self_,  char* buff, size_t len)
{
    BoldTurnerApi self = (BoldTurnerApi) self_;
    char pr_params[1024];
    char url[512];
    int resp = 0;
    SvString params = SvStringCreate("T", NULL);
 
    memset(pr_params, 0, 1024);
    memset(url,0, 512);
    
    resp = get_response(buff, len, url, pr_params);
       
    if (resp == PROXY_OK)
    {
        // All is OK
        SVTESTRELEASE(self->pr_params);
        self->pr_params = SvStringCreate(pr_params, NULL);
        SVTESTRELEASE(self->publicUri);
        self->publicUri = SvStringCreate(url, NULL);
        SvLogNotice("%s: TURNER, PlayReady Params= %s, link= %s",__func__, pr_params, url);
        self->publicUri = SvStringCreate(url, NULL);
        params = SvStringCreate("T", NULL);
    } 
    else if (resp == PROXY_VTA_PTE)  
    {
        // Vta paquete.
        SvLogInfo("%s: Venta paquete.", __func__);
        BoldVodVentaPopup(self->vod_api, "COMPRA TURNER", pr_params);
        params = SvStringCreate("F", NULL);
    }
    else 
    {
        // ABOCessage.
        SvLogError("%s: Get ABOCOD fail, resp: %s", __func__, buff);
        if (url[0] != 0)
        {
            // Show custom video error
            SVTESTRELEASE(self->publicUri);
            self->publicUri = SvStringCreate(url, NULL);
            params = SvStringCreate("T", NULL);
        }
        else if (pr_params[0] != 0)
        {
            BoldVodPopup(self->vod_api, "TURNER", pr_params);
            params = SvStringCreate("F", NULL);
        }
        else
        {
            BoldTurnerErrorPopup (self, "790001");
            params = SvStringCreate("F", NULL);
        }
    }
    self->callback(self->callbackData, params);
    self->callback = NULL;
    SVRELEASE(params);
}

SvLocal void
GetLinkToken(BoldTurnerApi self)
{
    char url [512];
    char deviceId[30];
    
    sprintf(url, "%s/linktoken?smartCardId=%s&contentUrl=%s", SvStringCString(self->proxy), clean(SvStringCString(self->deviceId), deviceId), SvStringCString(self->publicUri));
    SvLogNotice("%s: (%s)",  __func__, url);
    BoldHttpApiProxyStartRequest(self->api, (const char*)url, (void*)self, &GetLinkTokenCallback, NULL);
}

SvLocal void
BoldTurnerErrorPopup (BoldTurnerApi self, const char*szcode)
{
    const char* title = "TURNER+";
    char message[512];

    sprintf (message, "        ERROR: %s \n\n" 
            "        Si el problema persiste comuníquese al 1715.\n\n", szcode);
    BoldVodPopup(self->vod_api, title, message);
}

SvLocal void
BoldTurnerApiDestroy(void *self_)
{
    BoldTurnerApi self = self_;
    SVTESTRELEASE(self->publicUri);
    SVTESTRELEASE(self->deviceId);
    SVTESTRELEASE(self->url_pattern);
    SVRELEASE(self->api);
    SVRELEASE(self->proxy);
    SVRELEASE(self->pr_params);
}

SvLocal void
BoldTurnerApiInit(BoldTurnerApi self, AppGlobals appGlobals)
{
    self->appGlobals = appGlobals;
    self->callback  = NULL;
    const char *value;

   
    if ((value = QBConfigGet("TURNER.PROXY")))
        self->proxy = SvStringCreate (value, NULL);
    else 
        self->proxy = SvStringCreate ("http://foxproxy.boldmss.com:3033/linktoken", NULL);

    if ((value = QBConfigGet("TURNER.URI_PATTERN")))
        self->url_pattern = SvStringCreate (value, NULL);
    else
        self->url_pattern = SvStringCreate ("default", NULL);
    
    SvLogNotice ("%s: TURNER API initialized.", __func__);
}

SvLocal bool
BoldCheck(const char * uri)
{
    regex_t regex;

    /* Compile regular expression */
    if (regcomp(&regex, "anifest", 0))
    {
        SvLogError ("%s:", __func__);
        return false;
    }

    /* Execute regular expression */
    if (regexec(&regex, uri, 0, NULL, 0))
    {
        regfree(&regex);
        return false;
    }
    SvLogNotice ("%s: Smooth content", __func__);
    regfree(&regex);
    return true;
}


SvLocal void
BoldTurnerApiSmartCardCallbackInfo(void* self_, const QBCASCmd cmd, int status, QBCASInfo info)
{
    BoldTurnerApi self = (BoldTurnerApi) self_;
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
        SVRETAIN(deviceId);
        SVTESTRELEASE(self->deviceId);
        self->deviceId = deviceId;   
        SvLogNotice("%s: Got card number as Device Id: %s", __func__, SvStringCString(self->deviceId));
    }
}


SvLocal void 
BoldTurnerApiSmartCardUpdate (BoldTurnerApi self)
{
    if (self->infoCmd)
        SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->infoCmd);

    SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, BoldTurnerApiSmartCardCallbackInfo, self, &self->infoCmd);
}

SvLocal void 
BoldTurnerApiSmartCardState(void *self_, QBCASSmartcardState state)
{
    BoldTurnerApi self = self_;
    BoldTurnerApiSmartCardUpdate(self);
}

SvLocal bool
BoldTurnerCheckUri(BoldTurnerApi self, const char * uri)
{
    regex_t regex;
    
    /* Compile regular expression */
    if (regcomp(&regex, SvStringCString(self->url_pattern), 0))
    {
        SvLogError ("%s:", __func__);
        return false;
    }

    /* Execute regular expression */
    if (regexec(&regex, uri, 0, NULL, 0))
    {
        regfree(&regex);
        return false;   
    }
    regfree(&regex);
    return true;
}

////////////////////////////////////////////////////// Public functions ////////////////////////////////////////////////////////
void
BoldTurnerVtaPaqResp(BoldTurnerApi self)
{
    //SvString params = SvStringCreate("T", NULL);
    //self->callback(self->callbackData, params); 
    //self->callback = NULL;
    //SVRELEASE(params);
}

void
BoldTurnerVtaPaqCancel(BoldTurnerApi self)
{
    //SvString params = SvStringCreate("F", NULL);
    //self->callback(self->callbackData, params); 
    //self->callback = NULL;
    //SVRELEASE(params);
}

void
BoldTurnerVtaPaq(BoldTurnerApi self, void * callbackData, BoldTurnerCallback callback)
{
    char url [512];
    char deviceId[30];
    
    self->callback = callback; 
    self->callbackData = callbackData;
    sprintf(url, "%s/vtapaq?smartCardId=%s&contentUrl=%s", SvStringCString(self->proxy), clean(SvStringCString(self->deviceId), deviceId), SvStringCString(self->publicUri));
    SvLogNotice("%s: (%s)",  __func__, url);
    BoldHttpApiProxyStartRequest(self->api, (const char*)url, (void*)self, &VtaPaqCallback, NULL);
}

void
BoldTurnerCancelHttpReq(BoldTurnerApi self)
{
    BoldHttpApiCleanHTTPRequest(self->api);
}

BoldVodServiceType 
BoldTurnerGetType(BoldTurnerApi self, const char * uri)
{
    if (BoldTurnerCheckUri(self, uri))
    {
        if (BoldCheck(uri))
        {
            return BoldVodServiceTurner;
        }
    }
    return BoldVodServiceNONE;
}

SvType
BoldTurnerApi_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = 
    {
      .destroy = BoldTurnerApiDestroy
    };
    
    static SvType myType = NULL;

    if (unlikely(!myType))
    {
        SvTypeCreateManaged("BoldTurnerApi",
                sizeof(struct BoldTurnerApi_),
                SvObject_getType(), &myType,
                SvObject_getType(), &objectVTable,
                NULL);
    }
    return myType;
}

BoldTurnerApi 
BoldTurnerApiCreate(AppGlobals appGlobals, BoldVodApi vod_api,  BoldHttpApi http_api) 
{
    BoldTurnerApi self = (BoldTurnerApi) SvTypeAllocateInstance(BoldTurnerApi_getType(), NULL);
    self->api = SVRETAIN(http_api);
    self->vod_api = SVRETAIN(vod_api);
    BoldTurnerApiInit(self, appGlobals);
    return self;
}

static const struct QBCASCallbacks_s s_smartcard_callbacks =
{
    .smartcard_state = BoldTurnerApiSmartCardState
};

int 
BoldTurnerApiStart(BoldTurnerApi self) 
{
    SvLogNotice ("%s: Starting ...", __func__);
    SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks)&s_smartcard_callbacks, self, NULL);
    BoldTurnerApiSmartCardUpdate(self);
    return 0;
}

void BoldTurnerSetDeviceId(BoldTurnerApi self, SvString deviceId)
{
    SVTESTRELEASE(self->deviceId);
    self->deviceId = SVRETAIN(deviceId);
}

SvString 
BoldTurnerGetLink(BoldTurnerApi self)
{
    return self->publicUri;
}

SvString 
BoldTurnerGetPlayreadyParams(BoldTurnerApi self)
{
    return self->pr_params;
}

void
BoldTurnerHandler(BoldTurnerApi self, SvString uri, void * callbackData, BoldTurnerCallback callback)
{
    self->callback = callback;
    self->callbackData = callbackData;

    SVTESTRELEASE(self->publicUri);
    self->publicUri = SVRETAIN(uri); 
    
    if (!self->publicUri)
    {
        goto fini;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////
    GetLinkToken(self);
    return;
    ///////////////////////////////////////////////////////////////////////////////////////////////

fini:
    self->callback(self->callbackData, NULL);
    self->callback = NULL;
}

