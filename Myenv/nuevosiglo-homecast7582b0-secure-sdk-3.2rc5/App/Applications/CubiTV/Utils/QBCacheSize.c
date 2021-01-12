/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#include "QBCacheSize.h"
#include <QBConfig.h>
#include <QBPlatformHAL/QBPlatformMemory.h>
#include <assert.h>

typedef size_t (*cacheFunc)(void);

size_t QBCacheSizeGetResourceBundleHardSize(void)
{
    size_t size = QBPlatformGetAvailableMemory();

    static const cacheFunc caches[] = {
        QBPlatformLogicGetEPGTextRendererCacheSize,
        QBPlatformLogicGetTextRendererCacheSize,
        QBPlatformLogicGetTextRendererPersistentCacheSize,
        QBPlatformLogicGetCacheMarginSize
    };

    unsigned int i = 0;
    for (i = 0; i < sizeof(caches)/sizeof(caches[0]); ++i) {
        size_t c = caches[i]();
        assert(c < size);
        size -= c;
    }

    const char* maxResourceBundleSizeStr = QBConfigGet("UI.RESOURCE_BUNDLE.MAX_MEM_USAGE_MB");
    if (maxResourceBundleSizeStr) {
        SvLogNotice("Limiting resource bundle size to: %s MB", maxResourceBundleSizeStr);
        unsigned long maxResourceBundleSize = strtoul(maxResourceBundleSizeStr, NULL, 10) * 1024 * 1024;
        if (maxResourceBundleSize > 0 && size > maxResourceBundleSize) {
            size = maxResourceBundleSize;
        }
    }

    return size;
}

size_t QBCacheSizeGetResourceBundleSoftSize(void)
{
    size_t size = QBCacheSizeGetResourceBundleHardSize() / 4 * 3;
    assert(size > 0);
    return size;
}
