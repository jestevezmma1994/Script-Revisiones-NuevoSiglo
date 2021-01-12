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

#ifndef TV_OSD_INTERFACE
#define TV_OSD_INTERFACE

#include <stdbool.h>
#include <time.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <dataformat/sv_data_format.h>
#include <QBTextSubsManager.h>
#include <Windows/tv/QBTVChannelFrame.h>
#include <QBInput/QBInputEvent.h>
#include <CUIT/Core/widget.h>
#include <CAGE/Core/SvBitmap.h>
#include <QBResourceManager/SvRBBitmap.h>

typedef void (TVOSDEventSource)(void *, SvTVChannel, time_t, SvEPGEvent *, SvEPGEvent *);

typedef struct TVOSD_ {
    bool (*isShown)(SvObject self);
    void (*show)(SvObject self, bool immediate, bool force);
    void (*hide)(SvObject self, bool immediate, bool force);
    void (*update)(SvObject self);
    void (*lock)(SvObject self);
    void (*unlock)(SvObject self, bool force);
    bool (*isShowingMoreInfo)(SvObject self);
    void (*showMoreInfo)(SvObject self);
    void (*showLessInfo)(SvObject self);
    void (*setTVChannel)(SvObject self, const SvTVChannel channel, bool isFavorite);
    void (*setTVChannelNumber)(SvObject self, int number, int digits, int digitsMax);
    void (*setTVChannelName)(SvObject self, SvString name);
    void (*setTitle)(SvObject self, SvString title);
    void (*setDescription)(SvObject self, SvString description);
    void (*setCurrentEvent)(SvObject self, const SvEPGEvent event);
    void (*setTimeshiftedCurrentEvent)(SvObject self, const SvEPGEvent event);
    void (*setCurrentEventCaption)(SvObject self, const SvString caption);
    void (*setFollowingEvent)(SvObject self, const SvEPGEvent event);

    /**
     * Set data into osd's event frame of the given index.
     *
     * @param[in] self        TVOSD handle
     * @param[in] data        inserted data object
     * @param[in] frameIdx    index of the frame
     **/
    void (*setDataIntoEventFrame)(SvObject self, SvObject data, int frameIdx);
    void (*setTimeshiftedFollowingEvent)(SvObject self, const SvEPGEvent event);
    void (*setSubsManager)(SvObject self, const QBSubsManager subsManager);
    void (*setRecordingStatus)(SvObject self, bool isRecording);
    void (*setPlayState)(SvObject self, const SvPlayerTaskState *state);
    void (*setPosition)(SvObject self, double position, double end);
    void (*setEventSource)(SvObject self, TVOSDEventSource func, void *userdata);
    void (*checkForUpdates)(SvObject self, time_t currentTime, time_t timeshiftStart, time_t timeshiftEnd);
    void (*updateFormat)(SvObject self, struct svdataformat *format);
    void (*setCoverVisibility)(SvObject self, bool visible);
    void (*showSideMenu)(SvObject self, SvObject pane, SvString id);
    void (*hideSideMenu)(SvObject self);
    bool (*isSideMenuShown)(SvObject self);
    SvObject (*getSideMenuContext)(SvObject self);
    void (*setChannelSelectionCallback)(SvObject self, QBTVChannelFrameCallback callback, void *callbackData);
    bool (*handleInputEvent)(SvObject self, SvInputEvent ie);
    void (*attachNotificationContent)(SvObject self, SvWidget notificationContent, const char *notificationContentName);
    void (*showNotification)(SvObject self, bool immediate);
    void (*hideNotification)(SvObject self, bool immediate);
    /**
     * Set the advert to be displayed
     *
     * @param[in] self      TVOSD handle
     * @param[in] advert    the resource bundle bitmap with the advert
     **/
    void (*setAdvert)(SvObject self, SvRBBitmap advert);
    void (*clearAdvert)(SvObject self);
    /**
     * Set external plugin. Plugin can extend or change default functionality of OSD.
     *
     * @param[in] self      self handle
     * @param[in] plugin    plugin which will be added to OSD
     **/
    void (*setPlugin)(SvObject self, SvObject plugin);
} *TVOSD;

extern SvInterface TVOSD_getInterface(void);


#endif
