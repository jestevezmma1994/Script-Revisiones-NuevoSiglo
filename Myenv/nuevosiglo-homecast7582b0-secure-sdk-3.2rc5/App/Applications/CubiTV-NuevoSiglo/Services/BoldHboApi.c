
#include "BoldHboApi.h"

#include <strings.h>
#include <QBConf.h>
#include <Utils/dbobject.h>
#include <QBWidgets/QBDialog.h>
#include <Widgets/confirmationDialog.h>
#include <QBContentManager/QBContentMgrTransaction.h>
#include <SvFoundation/SvType.h>
#include <regex.h>

#define PROXY_OK        0
#define PROXY_ERROR     1
#define PROXY_VTA_PTE   2
#define PROXY_VTA_OK    3
#define PROXY_VTA_ERROR 4

struct BoldHboApi_
{
    struct SvObject_ super_;
    BoldHttpApi api;
    BoldVodApi vod_api;
    AppGlobals appGlobals;
    BoldHboCallback callback;
    void * callbackData;
    bool response_ok;
    
    SvString public_ip;
    SvString token;
    SvString publicUri;
    SvString deviceId;
    SvString url_pattern;
    SvString proxy;
};

SvLocal void
BoldHboErrorPopup (BoldHboApi self, const char*szcode)
{
    const char* title = "HBO GO";
    char message[512];

    sprintf (message, "        ERROR: %s \n\n" 
                "        Si el problema persiste comuníquese al 1715.\n\n", szcode);
    BoldVodPopup(self->vod_api, title, message);
}

SvLocal void
BoldHboApiDestroy(void *self_)
{
    BoldHboApi self = self_;
    SVTESTRELEASE(self->public_ip);
    SVTESTRELEASE(self->token);
    SVTESTRELEASE(self->publicUri);
    SVTESTRELEASE(self->deviceId);
    SVTESTRELEASE(self->url_pattern);
    SVTESTRELEASE(self->proxy);
    SVTESTRELEASE(self->api);
}


SvLocal void
BoldHboApiInit(BoldHboApi self, AppGlobals appGlobals)
{
    self->appGlobals = appGlobals;
    self->callback  = NULL;
    const char *url_pattern;
    const char *proxy;
    
    if ((url_pattern = QBConfigGet("HBO.URI_PATTERN")))
       self->url_pattern = SvStringCreate (url_pattern, NULL);
    else
        self->url_pattern = SvStringCreate ("default", NULL);
    
    if ((proxy = QBConfigGet("HBO.PROXY")))
       self->proxy = SvStringCreate (proxy, NULL);
    else
        self->proxy = SvStringCreate ("http://hboproxy.boldmss.com", NULL);

    SvLogNotice("%s : Uri Pattern: %s, Proxy: %s", __func__, url_pattern, proxy);
}

SvLocal bool
BoldHboCheckUri(BoldHboApi self, const char * uri)
{
    regex_t regex;
    
    /* Compile regular expression */
    if (regcomp(&regex, SvStringCString(self->url_pattern), 0))
    {
        SvLogError ("%s :", __func__);
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

    if (resp == NULL)
    {
        return PROXY_ERROR;
    }
    
   SvLogError("%s : RESP: %s",__func__, resp);
     
    if (strncmp(resp, "ERROR:", 6) == 0) 
    {
        SvLogError("%s : DENTRO DE ERROR",__func__);
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
        SvLogError("%s : DENTRO DE VTA",__func__);
        memcpy(tkn_txt, &resp[5], (resp_len-5));
        return PROXY_VTA_PTE;
    } 
    else 
    {
        SvLogError("%s : DENTRO DE OK",__func__);
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
    BoldHboApi self = (BoldHboApi) self_;
    char txt[192];
    char url[192];
    int resp = 0;
 
    memset(url, 0, 192);
    memset(txt, 0, 192);
    
    resp = get_vta_paq_response(buff, len, url, txt);
    
    SvLogError("%s : RESP: %d, url: %s, txt: %s",__func__, resp, url, txt);
     
    if (resp == PROXY_VTA_OK)
    { 
            BoldVodVentaOkPopup(self->vod_api, "HBO GO", txt);
    }
    else
    {
        if (url[0] != 0)
        {
            SvLogError("%s : url distinto cero",__func__);
            // Show custom video error
            SVTESTRELEASE(self->publicUri);
            self->publicUri = SvStringCreate(url, NULL);
        }
        else if (txt[0] != 0)
        {
            SvLogError("%s : txt distinto cero",__func__);
            BoldVodPopup(self->vod_api, "HBO GO", txt);
        }
        else
        {
            BoldHboErrorPopup (self, "990001");
        }
    }
    self->callback(self->callbackData, NULL); 
    self->callback = NULL;
}


SvLocal void
GetLinkTokenCallback (void * self_,  char* buff, size_t len)
{
    BoldHboApi self = (BoldHboApi) self_;
    char tkn_txt[192];
    char url[512];
    int resp = 0;
    SvString params = SvStringCreate("T", NULL);
 
    memset(tkn_txt, 0, 192);
    memset(url,0, 512);
    
    resp = get_response(buff, len, url, tkn_txt);
       
    if (resp == PROXY_OK)
    {
        // All is OK
        SVTESTRELEASE(self->token);
        self->token = SvStringCreate(tkn_txt, NULL);
        SVTESTRELEASE(self->publicUri);
        self->publicUri = SvStringCreate(url, NULL);
        SvLogNotice("%s : HBO, token= %s, link= %s",__func__, tkn_txt, url);
        
        params = SvStringCreate("T", NULL);
    } 
    else if (resp == PROXY_VTA_PTE)  
    {
        // Vta paquete.
        SvLogInfo("%s : Venta paquete.", __func__);
        BoldVodVentaPopup(self->vod_api, "COMPRA HBO GO", tkn_txt);
        params = SvStringCreate("F", NULL);
    }
    else 
    {
        // ABOCessage.
        SvLogError("%s : Get ABOCOD fail, resp: %s", __func__, buff);
        if (url[0] != 0)
        {
            // Show custom video error
            SVTESTRELEASE(self->publicUri);
            self->publicUri = SvStringCreate(url, NULL);
            params = SvStringCreate("T", NULL);
        }
        else if (tkn_txt[0] != 0)
        {
            BoldVodPopup(self->vod_api, "HBO GO", tkn_txt);
            params = SvStringCreate("F", NULL);
        }
        else
        {
            BoldHboErrorPopup (self, "990001");
            params = SvStringCreate("F", NULL);
        }
    }
    self->callback(self->callbackData, params); 
    self->callback = NULL;
    SVRELEASE(params);
}

SvLocal void
GetLinkToken(BoldHboApi self)
{
    char url [512];
    char deviceId[30];

    sprintf(url, "%s/linktoken?smartCardId=%s&contentUrl=%s", SvStringCString(self->proxy), clean(SvStringCString(self->deviceId), deviceId), SvStringCString(self->publicUri));
    SvLogNotice("%s : (%s)", __func__, url);
    BoldHttpApiProxyStartRequest(self->api, (const char*)url, (void*)self, &GetLinkTokenCallback, NULL);
}

////////////////////////////////////////////////////// Public functions ////////////////////////////////////////////////////////
void
BoldHboVtaPaqResp(BoldHboApi self)
{
    //SvString params = SvStringCreate("T", NULL);
    //self->callback(self->callbackData, params); 
    //self->callback = NULL;
    //SVRELEASE(params);
}
void
BoldHboVtaPaqCancel(BoldHboApi self)
{
    //SvString params = SvStringCreate("F", NULL);
    //self->callback(self->callbackData, params); 
    //self->callback = NULL;
    //SVRELEASE(params);
}

void
BoldHboVtaPaq(BoldHboApi self, void * callbackData, BoldHboCallback callback)
{
    char url [512];
    char deviceId[30];

    self->callback = callback; 
    self->callbackData = callbackData;
    sprintf(url, "%s/vtapaq?smartCardId=%s&contentUrl=%s", SvStringCString(self->proxy), clean(SvStringCString(self->deviceId), deviceId), SvStringCString(self->publicUri));
    SvLogNotice("%s : (%s)",  __func__, url);
    BoldHttpApiProxyStartRequest(self->api, (const char*)url, (void*)self, &VtaPaqCallback, NULL);
}
void
BoldHboCancelHttpReq(BoldHboApi self)
{
    BoldHttpApiCleanHTTPRequest(self->api);
}

BoldVodServiceType 
BoldHboGetType(BoldHboApi self, const char * uri)
{
    if (BoldHboCheckUri(self, uri))
    {
        return BoldVodServiceHBO;
    }
    return BoldVodServiceNONE;
}

SvType
BoldHboApi_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = 
    {
      .destroy = BoldHboApiDestroy
    };
    
    static SvType myType = NULL;

    if (unlikely(!myType))
    {
        SvTypeCreateManaged("BoldHboApi",
                sizeof(struct BoldHboApi_),
                SvObject_getType(), &myType,
                SvObject_getType(), &objectVTable,
                NULL);
    }
    return myType;
}

BoldHboApi 
BoldHboApiCreate(AppGlobals appGlobals, BoldVodApi vod_api, BoldHttpApi http_api)
{
    SvLogNotice ("%s :", __func__);
    BoldHboApi self = (BoldHboApi) SvTypeAllocateInstance(BoldHboApi_getType(), NULL);
    self->api = http_api;
    self->vod_api = vod_api;
    BoldHboApiInit(self, appGlobals);
    return self;
}

void 
BoldHboSetDeviceId(BoldHboApi self, SvString deviceId)
{
    SVTESTRELEASE(self->deviceId);
    self->deviceId = SVRETAIN(deviceId);
}

SvString 
BoldHboGetLink(BoldHboApi self)
{
    return self->publicUri;
}

SvString 
BoldHboGetCustomData(BoldHboApi self)
{
    return self->token;
}

void
BoldHboHandler(BoldHboApi self, SvString uri, void * callbackData, BoldHboCallback callback)
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



