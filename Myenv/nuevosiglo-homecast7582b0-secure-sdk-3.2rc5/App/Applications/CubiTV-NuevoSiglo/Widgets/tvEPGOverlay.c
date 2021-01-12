/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#include "tvEPGOverlay.h"
#include <Logic/EventsLogic.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvEPG/SvEPGOverlay.h>
#include <QBResourceManager/SvRBObject.h>
#include <settings.h>
#include <SWL/icon.h>
#include <Widgets/infoIcons.h>
#include <Widgets/QBRecordingIcon.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <main.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGManager.h>

typedef struct TVEPGOverlay_ *TVEPGOverlay;
struct TVEPGOverlay_ {
  struct SvObject_ super_;

  AppGlobals appGlobals;

  // used for channel header

  int iconsOffX, iconsOffY;
  int channelIDX, channelIDY;
  int channelNameX, channelNameY;

  SvRID emptyLogoRID;
  int logoWidth;
  int logoHeight;
  int logoOffX;
  int logoOffY;
  SvBitmap logoBG;

  SvGenericObject playlist;

  // used for event tags

  SvBitmap reminderIcon;
  int reminderIconXOffset;
  int reminderIconYOffset;

  SvBitmap cutvIconActive, cutvIconInactive;
  int cutvIconXOffset;
  int cutvIconYOffset;

  QBRecordingIconConstructor recordingIconConstructor;
  QBAsyncLabelConstructor channelIDConstructor, disabledChannelIDConstructor;
  QBAsyncLabelConstructor channelNameConstructor, disabledChannelNameConstructor;
};

struct tvEPGOverlayHeader_s {
    SvWidget channelID;
    SvWidget disabledChannelID;
    SvWidget channelName;
    SvWidget disabledChannelName;
    SvWidget logo;
    SvWidget icons;

    TVEPGOverlay overlay;
};
typedef struct tvEPGOverlayHeader_s* tvEPGOverlayHeader;

SvLocal SvURL
EPGGridHeaderURLFormatter(SvGenericObject obj)
{
    if (obj && SvObjectIsInstanceOf(obj, SvTVChannel_getType()))
    {
        //return (SvURL) SVTESTRETAIN(((SvTVChannel) obj)->logoURL);
        SvURL logoURL = SvURLCreateWithString(QBTVLogicGetChannelLogoURL(((SvTVChannel) obj)), NULL); // NR [#634] Logos de Canales en servidor
        return logoURL;
    }
    return NULL;
}

SvLocal SvWidget
TVEPGOverlaySetupHeader(SvGenericObject self_,
                        SvWidget header_,
                        SvTVChannel channel,
                        SvWidget prev)
{
    TVEPGOverlay self = (TVEPGOverlay) self_;

    if (!self->playlist || !self->emptyLogoRID) {
        svWidgetDestroy(prev);
        return NULL;
    }

    tvEPGOverlayHeader headerPrv = NULL;
    SvWidget header = NULL;

    if (!prev) {
        SvApplication app = header_->app;
        header = svWidgetCreateBitmap(app, header_->width, header_->height, NULL);
        headerPrv = calloc(1, sizeof(struct tvEPGOverlayHeader_s));
        header->prv = headerPrv;
        headerPrv->overlay = self;

        SvWidget channelID = QBAsyncLabelNewFromConstructor(app, self->channelIDConstructor);
        svWidgetAttach(header, channelID, self->channelIDX, self->channelIDY, 1);
        SvWidget disabledChannelID = NULL;
        if (svSettingsIsWidgetDefined("EPGWindow.EPGHeader.ChannelID.Disabled")) {
            disabledChannelID = QBAsyncLabelNewFromConstructor(app, self->disabledChannelIDConstructor);
            svWidgetAttach(header, disabledChannelID, self->channelIDX, self->channelIDY, 1);
        }

        headerPrv->channelID = channelID;
        headerPrv->disabledChannelID = disabledChannelID;

        SvWidget channelName = QBAsyncLabelNewFromConstructor(app, self->channelNameConstructor);
        svWidgetAttach(header, channelName, self->channelNameX, self->channelNameY, 1);
        SvWidget disabledChannelName = NULL;
        if (svSettingsIsWidgetDefined("EPGWindow.EPGHeader.ChannelName.Disabled")) {
            disabledChannelName = QBAsyncLabelNewFromConstructor(app, self->disabledChannelNameConstructor);
            svWidgetAttach(header, disabledChannelName, self->channelNameX, self->channelNameY, 1);
        }

        headerPrv->channelName = channelName;
        headerPrv->disabledChannelName = disabledChannelName;

        svSettingsPushTemporaryComponent("logo");
        svSettingsSetInteger("logo", "width", self->logoWidth);
        svSettingsSetInteger("logo", "height", self->logoHeight);
        SvWidget logo = svIconNew(header->app, "logo");
        svSettingsPopComponent();
        headerPrv->logo = logo;

        SvWidget logoBG = svWidgetCreateBitmap(app, self->logoBG->width, self->logoBG->height, self->logoBG);
        svWidgetAttach(header, logoBG, self->logoOffX, self->logoOffY, 1);
        svWidgetAttach(logoBG, logo, (logoBG->width - logo->width) / 2, (logoBG->height - logo->height) / 2, 1);

        SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                             SVSTRING("SvEPGManager"));
        SvWidget icons = QBInfoIconsCreate(self->appGlobals->res,
                                           "EPGWindow.EPGHeader.Icons",
                                           self->appGlobals->eventsLogic,
                                           epgManager,
                                           self->appGlobals->pc);
        svWidgetAttach(header, icons, self->iconsOffX, self->iconsOffY, 1);
        headerPrv->icons = icons;
    } else {
        header = prev;
        headerPrv = header->prv;
    }

    QBServiceRegistry registry = QBServiceRegistryGetInstance();
    QBPlaylistManager playlists = (QBPlaylistManager) QBServiceRegistryGetService(registry, SVSTRING("QBPlaylistManager"));
    SvString id = SvStringCreateWithFormat("%03u", QBPlaylistManagerGetNumberOfChannelInPlaylist(playlists, self->playlist, channel));
    QBTextRendererAddPersistentText(self->appGlobals->textRenderer, id);


    if (headerPrv->disabledChannelID && channel->isDisabled) {
        svWidgetSetHidden(headerPrv->channelID, true);
        svWidgetSetHidden(headerPrv->disabledChannelID, false);
        QBAsyncLabelClear(headerPrv->disabledChannelID);
        QBAsyncLabelSetText(headerPrv->disabledChannelID, id);
    } else {
        svWidgetSetHidden(headerPrv->channelID, false);
        if (headerPrv->disabledChannelID)
            svWidgetSetHidden(headerPrv->disabledChannelID, true);
        QBAsyncLabelClear(headerPrv->channelID);
        QBAsyncLabelSetText(headerPrv->channelID, id);
    }

    SVRELEASE(id);

    QBTextRendererAddPersistentText(self->appGlobals->textRenderer, channel->name);

    if (headerPrv->disabledChannelName && channel->isDisabled) {
        svWidgetSetHidden(headerPrv->channelName, true);
        svWidgetSetHidden(headerPrv->disabledChannelName, false);
        QBAsyncLabelClear(headerPrv->disabledChannelName);
        QBAsyncLabelSetText(headerPrv->disabledChannelName, channel->name);
    } else {
        svWidgetSetHidden(headerPrv->channelName, false);
        if (headerPrv->disabledChannelName)
            svWidgetSetHidden(headerPrv->disabledChannelName, true);
        QBAsyncLabelClear(headerPrv->channelName);
        QBAsyncLabelSetText(headerPrv->channelName, channel->name);
    }

    svIconSetBitmapFromRID(headerPrv->logo, 0, self->emptyLogoRID);
    SvURL bmpURL = EPGGridHeaderURLFormatter((SvGenericObject) channel);
    if (bmpURL) {
        svIconSetBitmapFromURI(headerPrv->logo, 1, SvStringCString(SvURLString(bmpURL)));
        svIconSwitch(headerPrv->logo, 1, 0, 0.0f);
        SVRELEASE(bmpURL);
    } else {
        svIconSwitch(headerPrv->logo, 0, 0, 0.0f);
    }
    QBInfoIconsSetByTVChannel(headerPrv->icons, channel);

    return header;
}

SvLocal SvWidget
TVEPGOverlaySetupEvent(SvGenericObject self_,
                     SvWidget widget, SvTVChannel channel, SvEPGEvent event,
                     int spacing, int visibleWidth)
{
    TVEPGOverlay self = (TVEPGOverlay) self_;

    SvWidget rec = NULL;
    QBPVRProvider pvrProvider = (QBPVRProvider)
        QBServiceRegistryGetService(QBServiceRegistryGetInstance(), SVSTRING("QBPVRProvider"));
    if (self->recordingIconConstructor) {
        rec = QBRecordingIconNewFromConstructor(self->appGlobals->res, pvrProvider, self->appGlobals->nPVRProvider,
                                                self->recordingIconConstructor);
        QBRecordingIconSetByEPGEvent(rec, event);
    }

    SvWidget reminder = NULL;
    if (channel && self->reminderIcon && QBReminderIsScheduledFor(self->appGlobals->reminder, channel, event)) {
        reminder = svWidgetCreateBitmap(widget->app, self->reminderIcon->width, self->reminderIcon->height, self->reminderIcon);
    }

    SvWidget cutv = NULL;
    if (self->appGlobals->cutvManager) {
        time_t cutvStart, cutvEnd;
        time_t now = SvTimeNow();
        if (CutvManagerGetEventAvailableTimes(self->appGlobals->cutvManager, event, &cutvStart, &cutvEnd)) {
            if (EventsLogicIsEventFeatureAllowed(self->appGlobals->eventsLogic, event, EventsLogicAllowedEventFeatureType_CUTV)) {
                if (cutvEnd > now) {
                    if (cutvStart <= now && self->cutvIconActive) {
                        if (event->startTime < now) // AMERELES Sacar CatchUp a programas posteriores al actual
                        {
                            cutv = svWidgetCreateBitmap(widget->app, self->cutvIconActive->width, self->cutvIconActive->height, self->cutvIconActive);
                        }
                    }/* else if (self->cutvIconInactive) {
                        cutv = svWidgetCreateBitmap(widget->app, self->cutvIconInactive->width, self->cutvIconInactive->height, self->cutvIconInactive);
                    }*/
                }
            }
        }
        
        // BEGIN AMERELES Mostrar ícono de CatchUp para Start-Over
        bool eventCanBeRecorded = event && SvEPGEventCanBeRemotelyStartedOver(event);
        if (!cutv && eventCanBeRecorded && event->startTime < now && now < event->endTime)
        {
            cutv = svWidgetCreateBitmap(widget->app, self->cutvIconActive->width, self->cutvIconActive->height, self->cutvIconActive);
        }
        // END AMERELES Mostrar ícono de CatchUp para Start-Over
    }

    if (!rec && !reminder && !cutv)
        return NULL;

    SvWidget overlay = svWidgetCreateBitmap(widget->app, widget->width, widget->height, NULL);
    int xOffset = overlay->width;
    if (visibleWidth >= 0) {
        xOffset = visibleWidth;
    }
    if (cutv) {
        xOffset -= self->cutvIconXOffset;
        svWidgetAttach(overlay, cutv, xOffset, self->cutvIconYOffset, 1);
    }
    if (reminder) {
        xOffset -= self->reminderIconXOffset;
        svWidgetAttach(overlay, reminder, xOffset, self->reminderIconYOffset, 1);
    }
    if (rec) {
        xOffset -= rec->width;
        svWidgetAttach(overlay, rec, xOffset, 0, 1);
    }
    return overlay;
}

SvLocal void
TVEPGOverlay__dtor__(void *self_)
{
   TVEPGOverlay self = self_;
   SVTESTRELEASE(self->playlist);
   SVTESTRELEASE(self->reminderIcon);
   SVTESTRELEASE(self->cutvIconActive);
   SVTESTRELEASE(self->cutvIconInactive);
   SVTESTRELEASE(self->recordingIconConstructor);
   SVTESTRELEASE(self->channelIDConstructor);
   SVTESTRELEASE(self->channelNameConstructor);
   SVTESTRELEASE(self->disabledChannelIDConstructor);
   SVTESTRELEASE(self->disabledChannelNameConstructor);
}

SvLocal SvType
TVEPGOverlay_getType(void)
{
   static const struct SvObjectVTable_ objectVTable = {
      .destroy = TVEPGOverlay__dtor__
   };
   static const struct SvEPGOverlay_t methods = {
      .setupHeader = TVEPGOverlaySetupHeader,
      .setupEvent = TVEPGOverlaySetupEvent
   };
   static SvType type = NULL;

   if (unlikely(!type)) {
      SvTypeCreateManaged("TVEPGOverlay",
                          sizeof(struct TVEPGOverlay_),
                          SvObject_getType(),
                          &type,
                          SvObject_getType(), &objectVTable,
                          SvEPGOverlay_getInterface(), &methods,
                          NULL);
   }

   return type;
}

static const char* concatWidgetName(const char* prefix, const char* suffix, char* tmp, int maxlen)
{
    if (!prefix || !*prefix)
        return suffix;
    snprintf(tmp, maxlen, "%s.%s", prefix, suffix);
    return tmp;
}
#define CONCAT_NAME(_varname_, _prefix_, _suffix_) \
  const char* _varname_ = concatWidgetName(_prefix_, _suffix_, tmp, 128)

SvGenericObject tvEPGOverlayCreate(AppGlobals appGlobals,
                                   const char* logoName, const char* headerName, const char* eventTagsName,
                                   SvGenericObject playlist)
{
    TVEPGOverlay self = (TVEPGOverlay) SvTypeAllocateInstance(TVEPGOverlay_getType(), NULL);
    self->appGlobals = appGlobals;
    self->playlist = SVTESTRETAIN(playlist);

    if (logoName) // "EPGWindow.EPGGrid.StripeHeader"
    {
        self->emptyLogoRID = svSettingsGetResourceID(logoName, "emptyLogo");
        self->logoWidth = svSettingsGetInteger(logoName, "logoWidth", -1);
        self->logoHeight = svSettingsGetInteger(logoName, "logoHeight", -1);
        self->logoOffX = svSettingsGetInteger(logoName, "logoOffX", 0);
        self->logoOffY = svSettingsGetInteger(logoName, "logoOffY", 0);
        self->logoBG = svSettingsGetBitmap(logoName, "logoBG");
    }

    if (headerName) // "EPGWindow.EPGHeader"
    {
        char tmp[128];

        CONCAT_NAME(iconsName, headerName, "Icons");
        self->iconsOffX = svSettingsGetInteger(iconsName, "xOffset", 0);
        self->iconsOffY = svSettingsGetInteger(iconsName, "yOffset", 0);

        CONCAT_NAME(channelIDName, headerName, "ChannelID");
        self->channelIDX = svSettingsGetInteger(channelIDName, "xOffset", 0);
        self->channelIDY = svSettingsGetInteger(channelIDName, "yOffset", 0);
        self->channelIDConstructor = QBAsyncLabelConstructorCreate(appGlobals->res, channelIDName, self->appGlobals->textRenderer, NULL);

        CONCAT_NAME(disabledChannelIDName, headerName, "ChannelID.Disabled");
        self->disabledChannelIDConstructor = QBAsyncLabelConstructorCreate(appGlobals->res, disabledChannelIDName, self->appGlobals->textRenderer, NULL);

        CONCAT_NAME(channelNameName, headerName, "ChannelName");
        self->channelNameX = svSettingsGetInteger(channelNameName, "xOffset", 0);
        self->channelNameY = svSettingsGetInteger(channelNameName, "yOffset", 0);
        self->channelNameConstructor = QBAsyncLabelConstructorCreate(appGlobals->res, channelNameName, self->appGlobals->textRenderer, NULL);

        CONCAT_NAME(disabledChannelNameName, headerName, "ChannelName.Disabled");
        self->disabledChannelNameConstructor = QBAsyncLabelConstructorCreate(appGlobals->res, disabledChannelNameName, self->appGlobals->textRenderer, NULL);
    }

    if (eventTagsName) // "EPGWindow.EPGGrid.StripeHeader"
    {
        char tmp[128];

        CONCAT_NAME(reminderName, eventTagsName, "Reminder");
        if (svSettingsIsWidgetDefined(reminderName)) {
            self->reminderIcon = SVRETAIN(svSettingsGetBitmap(reminderName, "tag"));
            self->reminderIconXOffset = svSettingsGetInteger(reminderName, "xOffset", 0);
            self->reminderIconYOffset = svSettingsGetInteger(reminderName, "yOffset", 0);
        }

        CONCAT_NAME(recordingName, eventTagsName, "Recording");
        if (svSettingsIsWidgetDefined(recordingName)) {
            self->recordingIconConstructor = QBRecordingIconConstructorNew(recordingName);
        }

        CONCAT_NAME(cutvName, eventTagsName, "CUTV");
        if (svSettingsIsWidgetDefined(cutvName)) {
            self->cutvIconInactive = svSettingsGetBitmap(cutvName, "tag");
            SVTESTRETAIN(self->cutvIconInactive);
            self->cutvIconActive = svSettingsGetBitmap(cutvName, "tagActive");
            SVTESTRETAIN(self->cutvIconActive);
            self->cutvIconXOffset = svSettingsGetInteger(cutvName, "xOffset", 0);
            self->cutvIconYOffset = svSettingsGetInteger(cutvName, "yOffset", 0);
        }
    }

    return (SvGenericObject)self;
}
