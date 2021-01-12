#include "BoldContentTracking.h"

#include <main.h>
#include <main_decl.h>
#include <QBConf.h>
#include <QBICSmartcardInfo.h>
#include <QBICSmartcardSubscription.h>
#include <QBNetManager.h>
#include <regex.h>
#include <Services/QBAppVersion.h>

#define CONTENT_TRACKING_URL_SIZE 512
#define CONTENT_TRACKING_DATA_BUFFER 1024

static BoldHttpApi http_api;
static AppGlobals appGlobals;
static bool enabled = false;
static SvString destinationAddress;
static SvString scid;


void BoldContentTrackingUpdateIds(SvString sc_id)
{
    if(sc_id)
    {
        SVTESTRELEASE(scid);
        scid = SVRETAIN(sc_id);
        BoldContentTrackingSendMessageLogin();
    }
}

void BoldContentTrackingInitialize(AppGlobals app, BoldHttpApi api)
{
    enabled = false;
    http_api = api;
    appGlobals = app;
    
    const char *tmp;
    if((tmp = QBConfigGet("CONTENTTRACKING.ENABLED")) && strcmp(tmp, "enabled") == 0)
        enabled = true;
    
    if (enabled)
    {
        destinationAddress = QBConfigGet("CONTENTTRACKING.SERVER") ? SvStringCreate(QBConfigGet("CONTENTTRACKING.SERVER"), NULL) : SVSTRING("127.0.0.1");
    }
    
    SvLogNotice("%s: TRACKING %s\n", __func__, enabled ? "enabled" : "disabled");
}

// SvLocal bool isFoxContent(const char * uri)
// {
//     regex_t regex;
    
//     /* Compile regular expression */
//     if (regcomp(&regex, "anifest.*UNTY.*FOX", 0))
//     {
//         SvLogError ("%s:", __func__);
//         return false;
//     }

//     /* Execute regular expression */
//     if (regexec(&regex, uri, 0, NULL, 0))
//     {
//         regfree(&regex);
//         return false;
//     }
//     regfree(&regex);
//     return true;
// }

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

SvLocal void BoldContentTrackingGetLinkTokenCallback(void * self_,  char* buff, size_t len)
{
    // DO NOTHING
    if (buff)
    {
        SvLogNotice("%s: %s\n", __func__, buff);
    }
    else
    {
        SvLogNotice("%s: buff is null\n", __func__);
    }
}

void BoldContentTrackingSendMessageLogin(void)
{
    if (!enabled || !scid)
    {
        return;
    }
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    sprintf(url, "%s/stb", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"%s\", \"deviceId\": \"%s\", \"so\": \"%s\"}", 
            "login", 
            clean(SvStringCString(scid), deviceId),
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)));
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, NULL, &BoldContentTrackingGetLinkTokenCallback, NULL);
}

void BoldContentTrackingSendMessageKeepAlive(SvString uri, BoldHttpApiCallback keepAliveCallback, void* keepAliveData, SvString mwUri)
{
    if (!enabled || !scid || !uri)
    {
        return;
    }
    
    //if (!isFoxContent(SvStringCString(uri)))
    //{
    //    return;
    //}
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    sprintf(url, "%s/concurrency", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"%s\", \"deviceId\": \"%s\", \"contentUrl\": \"%s\", \"so\": \"%s\", \"mwContentUrl\": \"%s\"}", 
            "keepalive", 
            clean(SvStringCString(scid), deviceId), 
            SvStringCString(uri),
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)),
            mwUri ? SvStringCString(mwUri) : "");
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, keepAliveData, keepAliveCallback, NULL);
}

void BoldContentTrackingSendMessageCommand(SvString uri, SvString commandType, double position, SvString mwUri, int playbackTime)
{
    if (!enabled || !commandType || !uri || !http_api || !scid)
    {
        return;
    }
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    SvObject ipAddress = QBNetManagerGetAttribute(appGlobals->net_manager, SVSTRING("ra0"), QBNetAttr_IPv4_address, NULL);
    SvString connectionType = SVSTRING("wired");
    if (ipAddress)
    {
        if (SvObjectIsInstanceOf(ipAddress, SvString_getType()))
        {
            connectionType = SVSTRING("wifi");
        }
    }
    
    sprintf(url, "%s/stb", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"%s\", \"deviceId\": \"%s\", \"contentUrl\": \"%s\", \"position\": \"%f\", \"playbacktime\": \"%d\", \"mwContentUrl\": \"%s\", \"connectionType\": \"%s\", \"so\": \"%s\"}", 
            SvStringCString(commandType), 
            clean(SvStringCString(scid), deviceId), 
            SvStringCString(uri), 
            position,
            playbackTime,
            SvStringCString(mwUri),
            SvStringCString(connectionType),
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)));
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, NULL, &BoldContentTrackingGetLinkTokenCallback, NULL);
}

void BoldContentTrackingSendMessageStartResume(SvString uri, SvString commandType, double position, int playbackTime, SvString mwUri, SvString pathinfo)
{
    if (!enabled || !commandType || !uri || !http_api || !scid)
    {
        return;
    }
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    SvObject ipAddress = QBNetManagerGetAttribute(appGlobals->net_manager, SVSTRING("ra0"), QBNetAttr_IPv4_address, NULL);
    SvString connectionType = SVSTRING("wired");
    if (ipAddress)
    {
        if (SvObjectIsInstanceOf(ipAddress, SvString_getType()))
        {
            connectionType = SVSTRING("wifi");
        }
    }
    
    sprintf(url, "%s/stb", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"%s\", \"deviceId\": \"%s\", \"contentUrl\": \"%s\", \"position\": \"%f\", \"playbacktime\": \"%d\", \"connectionType\": \"%s\", \"so\": \"%s\", \"mwContentUrl\": \"%s\", \"pathInfo\": \"%s\"}", 
            SvStringCString(commandType), 
            clean(SvStringCString(scid), deviceId), 
            SvStringCString(uri), 
            position,
            playbackTime,
            SvStringCString(connectionType),
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)),
            SvStringCString(mwUri),
            SvStringCString(pathinfo));
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, NULL, &BoldContentTrackingGetLinkTokenCallback, NULL);
}

void BoldContentTrackingSendMessageStop(SvString uri, SvString commandType, double position, SvString mwUri, int playbackTime, double currentBitrate, SvString finishStop)
{
    if (!enabled || !commandType || !uri || !http_api || !scid)
    {
        return;
    }
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    sprintf(url, "%s/stb", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"%s\", \"deviceId\": \"%s\", \"contentUrl\": \"%s\", \"position\": \"%f\", \"mwContentUrl\": \"%s\", \"playbacktime\": \"%d\", \"bitrate\": \"%.1f\", \"finishStop\": \"%s\", \"so\": \"%s\"}", 
            SvStringCString(commandType), 
            clean(SvStringCString(scid), deviceId), 
            SvStringCString(uri), 
            position, 
            SvStringCString(mwUri), 
            playbackTime, 
            currentBitrate,
            SvStringCString(finishStop),
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)));
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, NULL, &BoldContentTrackingGetLinkTokenCallback, NULL);
}

void BoldContentTrackingSendMessageProgress(SvString uri, SvString commandType, double position, SvString mwUri, int playbackTime, int progress, double currentBitrate)
{
    if (!enabled || !commandType || !uri || !http_api)
    {
        return;
    }
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    sprintf(url, "%s/stb", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"%s\", \"deviceId\": \"%s\", \"contentUrl\": \"%s\", \"playbacktime\": \"%d\", \"position\": \"%f\", \"mwContentUrl\": \"%s\", \"mark\": \"%d\", \"bitrate\": \"%.1f\", \"so\": \"%s\"}", 
            SvStringCString(commandType), 
            clean(SvStringCString(scid), deviceId), 
            SvStringCString(uri), 
            playbackTime,
            position, 
            SvStringCString(mwUri), 
            progress, 
            currentBitrate,
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)));
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, NULL, &BoldContentTrackingGetLinkTokenCallback, NULL);
}

void BoldContentTrackingSendMessageJoinTime(SvString uri, SvString commandType, int playbackTime, SvString mwUri)
{
    if (!enabled || !uri || !http_api)
    {
        return;
    }
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    sprintf(url, "%s/stb", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"%s\", \"deviceId\": \"%s\", \"contentUrl\": \"%s\", \"playbacktime\": \"%d\", \"mwContentUrl\": \"%s\", \"so\": \"%s\"}", 
            SvStringCString(commandType), 
            clean(SvStringCString(scid), deviceId), 
            SvStringCString(uri), 
            playbackTime,
            SvStringCString(mwUri), 
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)));
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, NULL, &BoldContentTrackingGetLinkTokenCallback, NULL);
}

void BoldContentTrackingSendMessageBufferUnderrun(SvString uri, SvString commandType, double position, SvString mwUri, int duration)
{
    if (!enabled || !uri || !http_api)
    {
        return;
    }
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    sprintf(url, "%s/stb", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"%s\", \"deviceId\": \"%s\", \"contentUrl\": \"%s\", \"position\": \"%f\", \"mwContentUrl\": \"%s\", \"duration\": \"%d\", \"so\": \"%s\"}", 
            SvStringCString(commandType), 
            clean(SvStringCString(scid), deviceId), 
            SvStringCString(uri), 
            position, 
            SvStringCString(mwUri), 
            duration,
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)));
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, NULL, &BoldContentTrackingGetLinkTokenCallback, NULL);
}

void BoldContentTrackingSendMessageError(SvString uri, SvString errorCode, SvString message, double position, SvString mwUri, int playbackTime)
{
    if (!enabled || !uri || !http_api || !message)
    {
        return;
    }
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    sprintf(url, "%s/stb", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"error\", \"deviceId\": \"%s\", \"contentUrl\": \"%s\", \"errorCode\": \"%s\", \"message\": \"%s\", \"playbacktime\": \"%d\", \"position\": \"%f\", \"mwContentUrl\": \"%s\", \"so\": \"%s\"}", 
            clean(SvStringCString(scid), deviceId), 
            SvStringCString(uri), 
            SvStringCString(errorCode), 
            SvStringCString(message),
            playbackTime, 
            position,
            SvStringCString(mwUri), 
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)));
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, NULL, &BoldContentTrackingGetLinkTokenCallback, NULL);
}

void BoldContentTrackingSendMessageSeek(SvString uri, int seekTime, SvString mwUri)
{
    if (!enabled || !uri || !http_api)
    {
        return;
    }
    
    char url [CONTENT_TRACKING_URL_SIZE];
    char data [CONTENT_TRACKING_DATA_BUFFER];
    char deviceId[30];
    
    sprintf(url, "%s/stb", SvStringCString(destinationAddress));

    sprintf(data, "{ \"event\": \"seek\", \"deviceId\": \"%s\", \"contentUrl\": \"%s\", \"seektime\": \"%d\", \"mwContentUrl\": \"%s\", \"so\": \"%s\"}", 
            clean(SvStringCString(scid), deviceId), 
            SvStringCString(uri), 
            seekTime,
            SvStringCString(mwUri), 
            SvStringCString(QBAppVersionGetFullVersion(appGlobals->appVersion)));
    SvLogNotice("%s: (%s)\n%s",  __func__, url, data);
    BoldHttpApiPost(http_api, (const char*)url, (const char*)data, NULL, &BoldContentTrackingGetLinkTokenCallback, NULL);
    
}
