#ifndef VOD3FakeCategories_H_
#define VOD3FakeCategories_H_
#include <SvDataBucket2/SvDBRawObject.h>
#include <main_decl.h>
#include <SvFoundation/SvGenericObject.h>

typedef struct VOD3FakeCategories_ * VOD3FakeCategories;
SvType VOD3FakeCategories_getType(void);
VOD3FakeCategories VOD3FakeCategoriesCreate(AppGlobals appGlobals);
void VOD3FakeCategoriesStart(VOD3FakeCategories self);
bool VOD3FakeCategoriesApplyInThisMovie(VOD3FakeCategories self,SvDBRawObject movie);
#endif
