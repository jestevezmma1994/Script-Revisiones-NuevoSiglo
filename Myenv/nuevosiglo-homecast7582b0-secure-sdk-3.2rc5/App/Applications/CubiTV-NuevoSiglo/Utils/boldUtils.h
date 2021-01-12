

#ifndef __BOLD_UTILS_H__
#define __BOLD_UTILS_H__


#include <CUIT/Core/widget.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <CUIT/Core/types.h>

// Search a son of 'parent' with the name 'widgetName'
SvWidget getSvWidgetChild(SvWidget parent, const char* widgetName);

// Return matadata's Package Offer name
SvString getPackageOfferType(SvDBRawObject metaData);

// Change Widget Position to newPositionSettingsName key in .settings
void BoldWidgetMove(SvWidget toMove, SvString newPositionSettingsName);

// Change Widget Properties to newPositionSettingsName key in .settings
void BoldWidgetSetProperties(SvWidget toMove, SvString newSettingsName);

// Hide toHide Widget if hide
void BoldWidgetHide(SvWidget toHide, bool hide);

// Return true if channelSourceURL is an IP URL
bool BoldIsChannelSourceIp(const char* channelSourceURL);

// Return true if service is served by the proxy
bool BoldIsMWProxyService(SvString serviceID);

// Return true if service has Mi Lista category
//bool BoldIsServiceWithMiLista(SvString serviceID, SvString serviceName);

// Return Service ID for displaying Serch in all contents
SvString getBoldSearchServiceID(void);

#endif // __BOLD_UTILS_H__
