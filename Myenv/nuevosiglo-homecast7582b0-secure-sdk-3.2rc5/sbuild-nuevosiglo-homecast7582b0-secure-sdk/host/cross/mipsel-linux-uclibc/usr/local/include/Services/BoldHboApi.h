#ifndef BoldHboApi_H_
#define BoldHboApi_H_
#include <main_decl.h>

#include <SvFoundation/SvGenericObject.h>
#include <Services/BoldHttpApi.h>
#include <Services/BoldVodApi.h>

typedef struct BoldHboApi_ * BoldHboApi;

typedef void (*BoldHboCallback) (void * self, SvString uri);

SvType
BoldHboApi_getType(void);

BoldHboApi BoldHboApiCreate(AppGlobals appGlobals, BoldVodApi vod_api, BoldHttpApi api);
void BoldHboHandler(BoldHboApi self, SvString uri, void * callbackData, BoldHboCallback callback);
BoldVodServiceType BoldHboGetType (BoldHboApi self, const char * uri);
void BoldHboSetDeviceId(BoldHboApi self, SvString deviceId);
SvString BoldHboGetCustomData(BoldHboApi self);
SvString BoldHboGetLink(BoldHboApi self);
void BoldHboCancelHttpReq(BoldHboApi self);
void BoldHboVtaPaq(BoldHboApi self, void * callbackData, BoldHboCallback callback);
void BoldHboVtaPaqResp(BoldHboApi self);
void BoldHboVtaPaqCancel(BoldHboApi self);
#endif

