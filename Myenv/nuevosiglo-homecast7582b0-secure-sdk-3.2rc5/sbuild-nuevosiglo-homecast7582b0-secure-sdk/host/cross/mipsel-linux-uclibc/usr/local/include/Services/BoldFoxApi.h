#ifndef BoldFoxApi_H_
#define BoldFoxApi_H_

#include <main_decl.h>
#include <SvFoundation/SvGenericObject.h>
#include <Services/BoldHttpApi.h>
#include <Services/BoldVodApi.h>

typedef struct BoldFoxApi_ * BoldFoxApi;

typedef void (*BoldFoxCallback) (void * self, SvString uri);

SvType
BoldFoxApi_getType(void);

BoldFoxApi BoldFoxApiCreate(AppGlobals appGlobals, BoldVodApi vod_api,  BoldHttpApi http_api); 
int BoldFoxApiStart(BoldFoxApi self); 
void BoldFoxHandler(BoldFoxApi self, SvString uri, void * callbackData, BoldFoxCallback callback);
BoldVodServiceType BoldFoxGetType (BoldFoxApi self, const char * uri);
void BoldFoxSetDeviceId(BoldFoxApi self, SvString deviceId);
void BoldFoxCancelHttpReq(BoldFoxApi self);
SvString BoldFoxGetPlayreadyParams(BoldFoxApi self);
SvString BoldFoxGetLink(BoldFoxApi self);
void BoldFoxVtaPaq(BoldFoxApi self, void * callbackData, BoldFoxCallback callback);
void BoldFoxVtaPaqResp(BoldFoxApi self);
void BoldFoxVtaPaqCancel(BoldFoxApi self);

#endif

