#ifndef BoldTurnerApi_H_
#define BoldTurnerApi_H_

#include <main_decl.h>

#include <Services/BoldHttpApi.h>
#include <Services/BoldVodApi.h>

typedef struct BoldTurnerApi_ * BoldTurnerApi;

typedef void (*BoldTurnerCallback) (void * self, SvString uri);

SvType
BoldTurnerApi_getType(void);

BoldTurnerApi BoldTurnerApiCreate(AppGlobals appGlobals, BoldVodApi vod_api,  BoldHttpApi http_api); 
int BoldTurnerApiStart(BoldTurnerApi self); 
void BoldTurnerHandler(BoldTurnerApi self, SvString uri, void * callbackData, BoldTurnerCallback callback);
BoldVodServiceType BoldTurnerGetType (BoldTurnerApi self, const char * uri);
void BoldTurnerSetDeviceId(BoldTurnerApi self, SvString deviceId);
void BoldTurnerCancelHttpReq(BoldTurnerApi self);
SvString BoldTurnerGetPlayreadyParams(BoldTurnerApi self);
SvString BoldTurnerGetLink(BoldTurnerApi self);
void BoldTurnerVtaPaq(BoldTurnerApi self, void * callbackData, BoldTurnerCallback callback);
void BoldTurnerVtaPaqResp(BoldTurnerApi self);
void BoldTurnerVtaPaqCancel(BoldTurnerApi self);

#endif



    
