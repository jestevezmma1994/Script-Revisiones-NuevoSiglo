#include "VOD3FakeCategories.h"
#include <QBConf.h>
#include <string.h>
#include <Utils/QBMovieUtils.h>
#include <SvDataBucket2/SvDBRawObject.h>
#include <SvFoundation/SvType.h>

#define VOD3FakeCategoriesDefaultFakeURL "http://fake.empty.vod.3.category"

struct VOD3FakeCategories_ {
    struct SvObject_ super_;
    bool bEnable;
    SvString sFakeURL;
};

SvLocal void VOD3FakeCategoriesDestroy(void *self_)
{
    VOD3FakeCategories self = self_;
    SVTESTRELEASE(self->sFakeURL);
}

SvType VOD3FakeCategories_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
      .destroy = VOD3FakeCategoriesDestroy
    };
    
    static SvType myType = NULL;

    if (unlikely(!myType))
    {
        SvTypeCreateManaged("VOD3FakeCategories",
                sizeof(struct VOD3FakeCategories_),
                SvObject_getType(), &myType,
                SvObject_getType(), &objectVTable,
                NULL);
    }
    return myType;
}

VOD3FakeCategories VOD3FakeCategoriesCreate(AppGlobals appGlobals)
{
    VOD3FakeCategories self = (VOD3FakeCategories) SvTypeAllocateInstance(VOD3FakeCategories_getType(), NULL);
    self->bEnable = false; 
    const char * sFakeURL = QBConfigGet("VOD3FAKECATEGORIES.FAKESOURCE");
    self->sFakeURL = SvStringCreate (sFakeURL?sFakeURL:VOD3FakeCategoriesDefaultFakeURL, NULL);
    return self;
}

void VOD3FakeCategoriesStart(VOD3FakeCategories self){
    const char *sEnable = QBConfigGet("VOD3FAKECATEGORIES.ENABLED");
    if (sEnable && strcmp(sEnable,"enabled") == 0)
    {
        self->bEnable=true;
    }
    else
    {
        self->bEnable=false;
    }
}

bool VOD3FakeCategoriesApplyInThisMovie(VOD3FakeCategories self, SvDBRawObject movie){
    if (self->bEnable)
    {
        SvString url = QBMovieUtilsGetAttr(movie, "source", NULL);
    
        if (url)
        {
            return SvStringEqualToCString(url, SvStringCString(self->sFakeURL));
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

