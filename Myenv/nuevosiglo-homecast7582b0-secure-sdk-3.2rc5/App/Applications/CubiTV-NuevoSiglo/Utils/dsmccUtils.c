/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#include "dsmccUtils.h"

#include <CUIT/Core/widget.h>
#include <settings.h>
#include <SvFoundation/SvString.h>
#include <QBResourceManager/SvRBBitmap.h>
#include <SWL/icon.h>


SvHidden void DSMCCUtilsEPGSetAds(SvApplication app,
                                  SvWindow window,
                                  int settingsCtx,
                                  SvWidget *adPreview,
                                  SvWidget *adBottom,
                                  SvHashTable images)
{
    svSettingsRestoreContext(settingsCtx);

    const char* const epgPreviewName = "TVPreview";
    if (adPreview && svSettingsIsWidgetDefined(epgPreviewName)) {
        SvRBBitmap adRB = NULL;
        if (images) {
            adRB = (SvRBBitmap) SvHashTableFind(images, (SvObject) SVSTRING("epg_1"));
        }

        if (adRB) {
            if (!*adPreview) {
                const int widthBorder = svSettingsGetInteger(epgPreviewName, "border", 0);
                int width = svSettingsGetInteger(epgPreviewName, "width", 0);
                int height = svSettingsGetInteger(epgPreviewName, "height", 0);
                const int heightBorder = widthBorder * height / width;
                width -= 2 * widthBorder;
                height -= 2 * heightBorder;
                const int xOffset = svSettingsGetInteger(epgPreviewName, "xOffset", 0) + widthBorder;
                const int yOffset = svSettingsGetInteger(epgPreviewName, "yOffset", 0) + heightBorder;
                *adPreview = svIconNewWithSize(app, width, height, false);
                svWidgetAttach(window, *adPreview, xOffset, yOffset, 10);
            }
            svIconSetBitmapFromRID(*adPreview, 0, SvRBObjectGetID((SvRBObject) adRB));
        } else if (*adPreview) {
            svWidgetDestroy(*adPreview);
            *adPreview = NULL;
        }
    }

    const char* const bottomAdName = "BottomAd";
    if (adBottom && svSettingsIsWidgetDefined(bottomAdName)) {
        SvRBBitmap adRB = NULL;
        if (images) {
            adRB = (SvRBBitmap) SvHashTableFind(images, (SvObject) SVSTRING("epg_2"));
        }

        if (adRB) {
            if (!*adBottom) {
                *adBottom = svIconNew(app, bottomAdName);
                svSettingsWidgetAttach(window, *adBottom, bottomAdName, 10);
            }
            svIconSetBitmapFromRID(*adBottom, 0, SvRBObjectGetID((SvRBObject) adRB));
        } else if (*adBottom) {
            svWidgetDestroy(*adBottom);
            *adBottom = NULL;
        }
    }

    svSettingsPopComponent();
}
