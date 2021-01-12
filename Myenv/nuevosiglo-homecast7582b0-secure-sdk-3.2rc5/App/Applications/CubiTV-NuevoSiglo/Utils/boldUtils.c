
#include "boldUtils.h"


#include <string.h>
#include <SvDataBucket2/SvDBReference.h>
#include <SvFoundation/SvStringBuffer.h>
#include <settings.h>
#include <QBConf.h>

SvWidget getSvWidgetChild(SvWidget parent, const char* widgetName)
{
    if (!parent)
    {
        return NULL;
    }

   SvWidget res = parent->first;
    while (res)
    {
        if (strcmp(svWidgetGetName(res), widgetName) == 0)
        {
            return res;
        }
        res = res->next;
    }
    
    return NULL;
}

void BoldWidgetMove(SvWidget toMove, SvString newPositionSettingsName)
{
    if (toMove)
    {
        int offsetX = toMove->off_x;
        int offsetY = toMove->off_y;
        
        toMove->off_x = svSettingsGetInteger(SvStringCString(newPositionSettingsName), "xOffset", offsetX);
        toMove->off_y = svSettingsGetInteger(SvStringCString(newPositionSettingsName), "yOffset", offsetY);
    }
}

void BoldWidgetSetProperties(SvWidget toMove, SvString newSettingsName)
{
    if (toMove)
    {
        int offsetX = toMove->off_x;
        int offsetY = toMove->off_y;
        int width = toMove->width;
        int height = toMove->height;
        
        toMove->off_x = svSettingsGetInteger(SvStringCString(newSettingsName), "xOffset", offsetX);
        toMove->off_y = svSettingsGetInteger(SvStringCString(newSettingsName), "yOffset", offsetY);
        toMove->width = svSettingsGetInteger(SvStringCString(newSettingsName), "width", width);
        toMove->height = svSettingsGetInteger(SvStringCString(newSettingsName), "height", height);
    }
}

void BoldWidgetHide(SvWidget toHide, bool hide)
{
    if (toHide)
    {
        svWidgetSetHidden(toHide, hide);
    }
}

SvString getPackageOfferType(SvDBRawObject metaData)
{
    SvArray offers = (SvArray) SvDBRawObjectGetAttrValue(metaData, "offers");
    SvString type = SvStringCreate("", NULL);
    if (offers && SvObjectIsInstanceOf((SvObject) offers, SvArray_getType()) && SvArrayCount(offers) == 1)
    {
        SvDBRawObject offer = (SvDBRawObject) SvDBReferenceGetReferredObject((SvDBReference)SvArrayObjectAtIndex(offers, 0));
        if (offer)
        {
            SvValue typeV = (SvValue) SvDBRawObjectGetAttrValue((SvDBRawObject) offer, "type");
            if (likely(typeV && SvObjectIsInstanceOf((SvObject) typeV, SvValue_getType()) && SvValueIsString(typeV) ))
            {
                type = SvValueGetString(typeV);
            }
        }
    }
    
    return type;
}

bool BoldIsChannelSourceIp(const char* channelSourceURL)
{
    return strncmp(channelSourceURL, "udp://", 6) == 0 || 
           strncmp(channelSourceURL, "http://", 7) == 0 || 
           strncmp(channelSourceURL, "https://", 8) == 0;
}

bool BoldIsMWProxyService(SvString serviceID)
{
    const char *mwPROXYenabled = QBConfigGet("mwPROXY.ENABLED");
    const char *mwPROXYurl = QBConfigGet("mwPROXY.URL");
    const char *mwPROXYservices = QBConfigGet("mwPROXY.SERVICES");
    bool ismwPROXYenabled = mwPROXYenabled && mwPROXYurl && strcmp(mwPROXYenabled, "enabled") == 0;
    
    bool isServiceProxy = SvStringEqualToCString(serviceID, "-100");
    char str[128];
    char *ptr = NULL;
    strcpy(str, mwPROXYservices);
    char *rest = str;
    int no_loop_infinito = 20;
    while (ismwPROXYenabled && !isServiceProxy && (ptr = strtok_r (rest, ",", &rest)) && no_loop_infinito > 0)
    {
        isServiceProxy = SvStringEqualToCString(serviceID, ptr);
        no_loop_infinito--;
    }
    
    return isServiceProxy;
}

//bool BoldIsServiceWithMiLista(SvString serviceID, SvString serviceName)
//{
//    bool res =  serviceName && (SvStringEqualToCString(serviceName, "Mi NsNow") || SvStringEqualToCString(serviceName, "Nitro"));
//    return res && BoldIsMWProxyService(serviceID);
//}

SvString getBoldSearchServiceID(void)
{
    return SVSTRING("6");
}
