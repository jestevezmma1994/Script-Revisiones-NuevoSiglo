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

#include "QBEventUtils.h"

#include <main.h>
#include <Logic/EventsLogic.h>

SvEPGEventDesc
QBEventUtilsGetDescFromEvent(EventsLogic logic,
                             QBLangPreferences langPreferences,
                             SvEPGEvent event)
{
    if (!logic || !langPreferences || !event) {
        return NULL;
    }

    SvEPGEventDesc desc = NULL;

    if (EventsLogicGetDescFromEvent(logic, event, &desc)) {
        return desc;
    }

    return QBLangPreferencesGetDescFromEvent(langPreferences, event);
}

SvEPGEventDesc
QBEventUtilsGetDescFromEvent_(void *appGlobals_,
                              SvEPGEvent event)
{
    AppGlobals appGlobals = appGlobals_;
    return QBEventUtilsGetDescFromEvent(appGlobals->eventsLogic,
                                        appGlobals->langPreferences,
                                        event);
}

SvEPGEventDesc
QBEventUtilsGetSearchableDescFromEvent(EventsLogic logic,
                                       QBLangPreferences langPreferences,
                                       SvEPGEvent event)
{
    if (!logic || !langPreferences || !event) {
        return NULL;
    }

    SvEPGEventDesc desc = NULL;

    if (EventsLogicGetSearchableDescFromEvent(logic, event, &desc)) {
        return desc;
    }

    return QBLangPreferencesGetDescFromEvent(langPreferences, event);
}

SvString
QBEventUtilsGetTitleFromEvent(EventsLogic logic,
                              QBLangPreferences langPreferences,
                              SvEPGEvent event)
{
    if (!logic || !langPreferences || !event) {
        return NULL;
    }

    SvString title = NULL;
    if (EventsLogicGetTitleFromEvent(logic, event, &title)) {
        return title;
    }

    SvEPGEventDesc desc = QBEventUtilsGetDescFromEvent(logic, langPreferences, event);
    return ( (desc && desc->title) ? desc->title : SVSTRING("") );
}

SvString
QBEventUtilsGetRawTitleFromEvent(QBLangPreferences langPreferences,
                                 SvEPGEvent event)
{
    if (!langPreferences || !event) {
        return NULL;
    }

    return QBLangPreferencesGetTitleFromEvent(langPreferences, event);
}

QBPCList
QBEventUtilsCreateEventRating(EventsLogic logic,
                              SvEPGManager epgManager,
                              SvEPGEvent event)
{
    if (!logic || !epgManager || !event) {
        return NULL;
    }

    SvErrorInfo error = NULL;
    if (event->PC && SvStringLength(event->PC) != 0) {
        QBPCList rating = QBPCListCreate(&error);
        if (error)
           goto err;

        QBPCListLockRatingsFromString(rating, event->PC, &error);
        if (error) {
            SVTESTRELEASE(rating);
            goto err;
        }

        return rating;
    }

    SvValue channelId = event->channelID;
    SvTVChannel channel = SvEPGManagerFindTVChannel(epgManager, channelId, NULL);

    QBPCList rating = NULL;
    if (EventsLogicCreateNotRatedEventRating(logic, channel, &rating)) {
        return rating;
    }

    return NULL;

err:
    SvErrorInfoWriteLogMessage(error);
    SvErrorInfoDestroy(error);
    return NULL;
}
