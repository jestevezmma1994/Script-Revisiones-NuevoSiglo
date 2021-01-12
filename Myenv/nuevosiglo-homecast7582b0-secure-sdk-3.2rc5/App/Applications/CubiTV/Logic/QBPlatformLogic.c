/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#include <Utils/QBCacheSize.h>
#include <QBInput/QBInputUtils.h>
#include <QBPlatformHAL/QBPlatformMemory.h>

size_t QBPlatformLogicGetEPGTextRendererCacheSize(void)
{
    return QB_EPG_TEXT_RENDERER_CACHE_SIZE;
}

size_t QBPlatformLogicGetTextRendererCacheSize(void)
{
    return QB_TEXT_RENDERER_CACHE_SIZE;
}

size_t QBPlatformLogicGetTextRendererPersistentCacheSize(void)
{
    return QB_TEXT_RENDERER_PERSISTENT_CACHE_SIZE;
}

size_t QBPlatformLogicGetCacheMarginSize(void)
{
    // 10% of memory is used as margin (but not less than MIN cache) and align to megabytes
    const size_t size = (QBPlatformGetAvailableMemory() / 10) & ~(size_t) 0xFFFFF;
    return size > QB_MARGIN_MIN_CACHE_SIZE ? size : QB_MARGIN_MIN_CACHE_SIZE;
}

void QBPlatformLogicSetupInputAutoRepeat(void)
{
    // auto repeat keys 8 times a second after initial 500 ms delay
    QBInputSetupAutoRepeat(500000, 125000, NULL);
}
