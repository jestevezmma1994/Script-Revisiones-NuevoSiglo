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

#include "teleosd.h"

#include <libintl.h>
#include <CUIT/Core/widget.h>
#include <settings.h>
#include <Widgets/infoIcons.h>
#include <QBWidgets/QBAsyncLabel.h>
#include <Widgets/osd.h>
#include <Translations/QBTranslator.h>
#include <QBAppKit/QBServiceRegistry.h>
#include <SvEPGDataLayer/SvEPGManager.h>

#define MAX_LENGTH 255
#define LISTNAME_SUFFIX ".listName"
#define LOGO_SUFFIX ".customerLogo"
#define FRAME_SUFFIX ".frame"
#define INFOICONS_SUFFIX ".infoIcons"
#define NAME_SUFFIX ".name"


typedef struct SvTVOSD_ {
   struct SvObject_ super_;

   SvWidget listName;
   SvWidget frame, infoIcons;
   SvRID tmpRID;
   int iconsSpacing;
   QBPlaylistManager playlists;
   QBViewport viewport;
} *SvTVOSD;

SvLocal SvType SvTVOSD_getType(void)
{
   static SvType type = NULL;

   if (!type) {
       SvTypeCreateManaged("SvTVOSD",
                           sizeof(struct SvTVOSD_),
                           SvObject_getType(), &type,
                           NULL);
   }

   return type;
}

void svTVOsdSetTVChannel(SvWidget tvOSD, SvTVChannel channel, bool isFavorite)
{
   SvTVOSD tv = tvOSD->prv;
   QBTVChannelFrameSetChannel(tv->frame, channel);
   SvString listName = SvStringCreate(QBTranslatorGetPlaylistName(tv->playlists, QBPlaylistManagerGetCurrentId(tv->playlists)), NULL);
   if (!listName) {
      QBAsyncLabelSetCText(tv->listName, gettext("No list"));
   } else {
      QBAsyncLabelSetText(tv->listName, listName);
   }
   SVTESTRELEASE(listName);
}

void svTVOsdSetListName(SvWidget tvOSD, SvString listName)
{
    SvTVOSD tv = tvOSD->prv;
    QBAsyncLabelSetText(tv->listName, listName);
}

void svTVOsdSetInfoIconsByEvent(SvWidget tvOSD, SvEPGEvent event)
{
    SvTVOSD tv = tvOSD->prv;
    SvWidget w = tv->infoIcons;
    QBInfoIconsSetByEvent(w, event);
}

void svTVOsdSetInfoIconsBySubsManager(SvWidget tvOSD, QBSubsManager subsManager)
{
    SvTVOSD tv = tvOSD->prv;
    SvWidget w = tv->infoIcons;
    QBInfoIconsSetBySubsManager(w, subsManager);
}

void SvTVOsdSetInfoIconsByFormat(SvWidget tvOSD, struct svdataformat* format)
{
    SvTVOSD tv = (SvTVOSD) tvOSD->prv;
    SvWidget w = tv->infoIcons;
    QBInfoIconsSetByFormat(w, format);
}

void svTVOsdSetInfo(SvWidget tvOSD, SvTVChannel channel,  SvEPGEvent event, QBSubsManager subsManager)
{
    SvTVOSD tv = tvOSD->prv;
    SvWidget w = tv->infoIcons;
    QBInfoIconsSetByTVChannel(w, channel);
    QBInfoIconsSetByEvent(w, event);
    QBInfoIconsSetByViewport(w, tv->viewport);
    QBInfoIconsSetBySubsManager(w, subsManager);

    struct svdataformat* format = QBViewportGetOutputFormat(tv->viewport);
    QBInfoIconsSetByFormat(w, format);
}

void svTVOsdSetRecordingStatus(SvWidget tvOSD, bool isRecording)
{
   SvTVOSD tv = tvOSD->prv;
   QBInfoIconsSetState(tv->infoIcons, QBInfoIcons_Recording, isRecording);
}

extern void svTVOsdSetChannelName(SvWidget tvOSD, SvString name)
{
    SvTVOSD tv;
    tv = tvOSD->prv;
    QBTVChannelFrameSetChannelName(tv->frame, name);
}

SvLocal void
QBPlayListNameCallback(void *prv_, SvWidget label)
{
    SvTVOSD tv = prv_;
    tv->frame->off_x = tv->listName->off_x + QBAsyncLabelGetWidth(tv->listName) + tv->iconsSpacing;
}

SvLocal void svTVOSDFrameNameChangedCallback(void *prv)
{
    SvTVOSD tv = prv;
    tv->listName->off_x = tv->frame->off_x + tv->frame->width + tv->iconsSpacing;
}

SvLocal void svTVOsdVideoInfoChanged(void *self_, QBViewport qbv, const QBViewportVideoInfo *videoInfo)
{
    SvTVOSD tv = (SvTVOSD) self_;
    SvWidget w = tv->infoIcons;
    QBInfoIconsSetByViewport(w, tv->viewport);
}

static struct qb_viewport_callbacks viewportCallbacks = {
    .video_info_changed = svTVOsdVideoInfoChanged
};

SvLocal void svTVOsdClean(SvApplication app, void *tv_)
{
    SvTVOSD tv = tv_;
    QBViewportRemoveCallbacks(tv->viewport, &viewportCallbacks, tv);
    SVRELEASE(tv);
}

bool svTVOSDInputEventHandler(SvWidget tvOSD, QBInputEvent ie)
{
    SvTVOSD tv = tvOSD->prv;
    return QBTVChannelFrameInputEventHandler(tv->frame, &ie);
}

void svTVOSDSetChannelFrameCallback(SvWidget tvOSD, QBTVChannelFrameCallback callback, void *callbackData)
{
    SvTVOSD tv = tvOSD->prv;
    QBTVChannelFrameSetCallback(tv->frame, callback, callbackData);
}

SvWidget
svTVOSDNew(SvApplication app,
           const char *widgetName,
           QBTextRenderer textRenderer,
           QBPlaylistManager playlists,
           QBViewport viewport,
           EventsLogic eventsLogic,
           QBTVLogic tvLogic,
           QBParentalControl pc)
{

  // TODO: SQA team comment: ugly code - it require refactorization.
  // This code assume that we have loaded settings.
  // Usage of this code always assume that this function never fail - noboady check that
  // return value might be NULL.

   const int width = svSettingsGetInteger(widgetName, "width", -1);
   const int height = svSettingsGetInteger(widgetName, "height", -1);

   if ((width < 0) || (height < 0))
      return NULL;

   SvWidget tvOSD = svWidgetCreateBitmap(app, width, height, NULL);
   if (tvOSD == NULL)
      return NULL;

   char childName[MAX_LENGTH];
   SvWidget nameLabel;

   SvTVOSD tv = (SvTVOSD) SvTypeAllocateInstance(SvTVOSD_getType(), NULL);
   tv->playlists = playlists;
   tv->viewport = viewport;
   tvOSD->prv = tv;
   tvOSD->clean = svTVOsdClean;
   strncpy(childName, widgetName, MAX_LENGTH);

   strncpy(childName + strlen(widgetName), FRAME_SUFFIX, MAX_LENGTH - strlen(widgetName));

   if (!(tv->frame = QBTVChannelFrameNew(app, childName, textRenderer, playlists, tvLogic))) {
       svWidgetDestroy(tvOSD);
       return NULL;
   }

   if(!svSettingsWidgetAttach(tvOSD, tv->frame, childName, 1)){
       svWidgetDestroy(tv->frame);
       tv->frame = NULL;
       svWidgetDestroy(tvOSD);
       return NULL;
   }
   strncpy(childName + strlen(widgetName), LISTNAME_SUFFIX, MAX_LENGTH - strlen(widgetName));

   if (!(nameLabel = QBAsyncLabelNew(app, childName, textRenderer))){
      svWidgetDestroy(tvOSD);
      return NULL;
   }
   if (!(svSettingsWidgetAttach(tvOSD, nameLabel, childName, 10))){
      svWidgetDestroy(nameLabel);
      svWidgetDestroy(tvOSD);
      return NULL;
   }
   QBViewportAddCallbacks(viewport, &viewportCallbacks, tv);
   if (nameLabel->off_x <= tv->frame->off_x) {
       QBAsyncLabelSetCallback(nameLabel, QBPlayListNameCallback, tv);
   } else {
       QBTVChannelFrameNameChangedSetCallback(tv->frame, svTVOSDFrameNameChangedCallback, tv);
   }
   tv->listName = nameLabel;

   strncpy(childName + strlen(widgetName), INFOICONS_SUFFIX, MAX_LENGTH - strlen(widgetName));
   SvEPGManager epgManager = (SvEPGManager) QBServiceRegistryGetService(QBServiceRegistryGetInstance(),
                                                                        SVSTRING("SvEPGManager"));
   SvWidget icons = QBInfoIconsCreate(app, childName, eventsLogic, epgManager, pc);
   if (!icons){
      svWidgetDestroy(tvOSD);
      return NULL;
   }
   svSettingsWidgetAttach(tvOSD, icons, childName, 1);
   tv->infoIcons = icons;
   tv->iconsSpacing = svSettingsGetInteger(childName, "spacing", 0);

   strncpy(childName + strlen(widgetName), LOGO_SUFFIX, MAX_LENGTH - strlen(widgetName));
   if (svSettingsIsWidgetDefined(childName)) {
       SvWidget logo = svSettingsWidgetCreate(app, childName);
       if (logo) svSettingsWidgetAttach(tvOSD, logo, childName, 1);
   }

   return tvOSD;
}
