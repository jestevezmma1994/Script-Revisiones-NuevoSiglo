/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#include "Widgets/QBTraxisStateIndicator.h"

#include <SWL/icon.h>
#include <settings.h>
#include <CUIT/Core/widget.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvGenericObject.h>
#include <SvCore/SvLog.h>
#include <SvCore/SvErrorInfo.h>
#include <SvCore/SvCoreErrorDomain.h>

#define log_debug(fmt, ...)  do { if (0) SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_info(fmt, ...)   do { if (1) SvLogNotice(COLBEG() fmt COLEND_COL(cyan), ##__VA_ARGS__); } while (0)
#define log_error(fmt, ...)  do { if (1) SvLogError( COLBEG() fmt COLEND_COL(red),  ##__VA_ARGS__); } while (0)

typedef struct QBTraxisStateIndicator_t {
    struct SvObject_ super_;
    SvWidget traxisStateIcon;

    TraxisWebSessionManager traxisManager; //different than NULL indicates, that Indicator is already registered to pointed out traxisManager
} *QBTraxisStateIndicator;

enum QBTraxisStateIndicatorImage {QBTraxisStateIndicatorImage_not_OK=0,
                                  QBTraxisStateIndicatorImage_OK};

SvLocal void
QBTraxisStateIndicatorStateChanged(SvGenericObject self_, TraxisWebSessionState state)
{
}

SvLocal void
QBTraxisStateIndicatorAuthLevelChanged(SvGenericObject self_, TraxisWebAuthLevel authLevel)
{
    QBTraxisStateIndicator self = (QBTraxisStateIndicator)self_;

    TraxisWebSessionState sessionState = TraxisWebSessionManagerGetState(self->traxisManager);
    if(sessionState == TraxisWebSessionState_active || sessionState == TraxisWebSessionState_anonymous)
        if (authLevel == TraxisWebAuthLevel_authenticated) {
            svIconSwitch(self->traxisStateIcon, QBTraxisStateIndicatorImage_OK, QBTraxisStateIndicatorImage_OK, -1.0);
            return;
        }

    svIconSwitch(self->traxisStateIcon, QBTraxisStateIndicatorImage_not_OK, QBTraxisStateIndicatorImage_not_OK, -1.0);
}

SvLocal void
QBTraxisStateIndicatorLanguageChanged(SvGenericObject self_, SvString language)
{
    ///Nothing to do
}

SvLocal int
QBTraxisStateIndicatorRegisterWithTraxisManager(QBTraxisStateIndicator self, TraxisWebSessionManager traxisManager)
{
    SvErrorInfo error = NULL;

    if (!self || !traxisManager) {
        error = SvErrorInfoCreate(SvCoreErrorDomain, SvCoreError_invalidArgument,
                                  "NULL argument passed");
        goto err;
    }


    //If already registered to some traxis manager
    if (self->traxisManager) {
        TraxisWebSessionManagerRemoveListener(self->traxisManager,(SvGenericObject) self, &error);
        if (error)
            goto err;
    }

    TraxisWebSessionManagerAddListener(traxisManager, (SvGenericObject) self, &error);
    if (error)
        goto err;

    self->traxisManager = traxisManager;

    return 0;

err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return -1;
}

SvLocal void
QBTraxisStateIndicatorClean(SvApplication app, void *ptr)
{
    QBTraxisStateIndicator self = ptr;
    SVRELEASE(self);
}

SvLocal void
QBTraxisStateIndicator__dtor__(void *self_)
{
    ///Nothing to do
    ///traxisManager wasn't retained
    ///traxisStateIcon is released by parent window
}

SvLocal SvType
QBTraxisStateIndicator_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = QBTraxisStateIndicator__dtor__
    };
    static SvType type = NULL;

    static const struct TraxisWebSessionStateListener_ traxisMethods = {
        .stateChanged = QBTraxisStateIndicatorStateChanged,
        .languageChanged = QBTraxisStateIndicatorLanguageChanged,
        .authLevelChanged = QBTraxisStateIndicatorAuthLevelChanged,
    };

    if (unlikely(!type)) {
        SvTypeCreateManaged("QBTraxisStateIndicator",
                            sizeof(struct QBTraxisStateIndicator_t),
                            SvObject_getType(),
                            &type,
                            SvObject_getType(), &objectVTable,
                            TraxisWebSessionStateListener_getInterface(), &traxisMethods,
                            NULL);
    }

    return type;
}

SvWidget
QBTraxisStateIndicatorCreate(SvApplication app, const char* widgetName, TraxisWebSessionManager traxisManager)
{
    int status = 0;
    QBTraxisStateIndicator self = NULL;

    if (!app || !widgetName || !traxisManager) {
        log_error("%s (%d): Bad arguments", __FUNCTION__, __LINE__);
        return NULL;
    }
    if (!svSettingsIsWidgetDefined(widgetName))
        return NULL;

    self = (QBTraxisStateIndicator) SvTypeAllocateInstance(QBTraxisStateIndicator_getType(), NULL);

    if (unlikely(!self)) {
        log_error("%s (%d): Can't allocate QBTraxisStateIndicator", __FUNCTION__, __LINE__);
        return NULL;
    }

    self->traxisStateIcon = NULL;
    self->traxisManager = NULL;

    //Widgets creation
    SvWidget w = svSettingsWidgetCreate(app, widgetName); //Outer widget

    if (!w) {
        log_info("%s: Can't create widget", __FUNCTION__); //Probably there were no settings (on purpose)
        SVRELEASE(self);
        return NULL;
    }

    //Widgets construction
    w->prv = self; //widget stores pointer to created GenObj
    w->clean = QBTraxisStateIndicatorClean; //function which would be called after widget destroy to cleanup prv
    svWidgetSetFocusable(w, false);

    char buf[128];
    snprintf(buf, 128, "%s.Icon", widgetName);

    self->traxisStateIcon = svIconNew(app, buf); //Inner widget (icon)

    if (!self->traxisStateIcon) {
        log_error("%s (%d): Can't create icon", __FUNCTION__, __LINE__);
        goto err;
    }

    svSettingsWidgetAttach(w, self->traxisStateIcon, buf, 1);

    const char *bgName[] = {"bgNo", "bgYes", NULL};
    enum QBTraxisStateIndicatorImage bgFeature[] = {QBTraxisStateIndicatorImage_not_OK,
                                                    QBTraxisStateIndicatorImage_OK};

    SvBitmap bitmap;
    for (int i=0; ; i++) {
        if(!bgName[i])
            break;

        bitmap = svSettingsGetBitmap(buf, bgName[i]);

        if(unlikely(!bitmap)) {
            log_error("%s (%d): Can't load bitmaps", __FUNCTION__, __LINE__);
            goto err;
        }

        svIconSetBitmap(self->traxisStateIcon, bgFeature[i], bitmap);
    }

    svIconSwitch(self->traxisStateIcon, QBTraxisStateIndicatorImage_not_OK,
                 QBTraxisStateIndicatorImage_not_OK, -1.0);


    status = QBTraxisStateIndicatorRegisterWithTraxisManager(self, traxisManager);

    if (!status)
        QBTraxisStateIndicatorStateChanged((SvGenericObject)self, TraxisWebSessionManagerGetState(traxisManager)); //Indicator initialization
    else {
        log_error("%s (%d): Can't register with Traxis Manager", __FUNCTION__, __LINE__);
        goto err;
    }


    return w; //outer widget with private GenObj

err:
    svWidgetDestroy(w);
    return NULL;
}



