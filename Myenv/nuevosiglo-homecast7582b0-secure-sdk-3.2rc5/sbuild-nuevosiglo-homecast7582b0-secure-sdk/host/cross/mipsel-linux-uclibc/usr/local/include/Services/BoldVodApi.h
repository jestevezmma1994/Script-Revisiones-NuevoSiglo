#ifndef BoldVodApi_H_
#define BoldVodApi_H_
#include <main_decl.h>

#include <SvDataBucket2/SvDBRawObject.h>

typedef struct BoldVodApi_ * BoldVodApi;

typedef enum {
    BoldVodServiceNONE,
    BoldVodServiceHBO,
    BoldVodServiceFOX_hls,
    BoldVodServiceFOX_ss,
    BoldVodServiceFOX_ss_v3,
    BoldVodServiceTurner
} BoldVodServiceType;

typedef void (*BoldVodCallback) (void * self, SvString uri);


SvType
BoldVodApi_getType(void);

BoldVodApi BoldVodApiCreate(AppGlobals appGlobals);
int BoldVodApiStart(BoldVodApi self); 
void BoldVodProductHandler(BoldVodApi self, SvDBRawObject product, void * callbackData, BoldVodCallback callback);
BoldVodServiceType BoldVodGetServiceType(BoldVodApi self);
SvString BoldVodHboGetCustomData(BoldVodApi self);
SvString BoldVodHboGetLink(BoldVodApi self);
SvString BoldVodFoxGetLink(BoldVodApi self);
SvString BoldVodFoxGetPlayreadyParams(BoldVodApi self);
SvString BoldVodTurnerGetLink(BoldVodApi self);
SvString BoldVodTurnerGetPlayreadyParams(BoldVodApi self);
void BoldVodPopup (BoldVodApi self, const char * title, const char*message);
void BoldVodVentaPopup (BoldVodApi self, const char * title, const char*message);
void BoldVodVentaOkPopup (BoldVodApi self, const char * title, const char*message);
#endif

