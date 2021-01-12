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

#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvAtomic.h>
#include <SvCore/SvLog.h>
#include <mpeg_common.h>
#include <mpeg_descriptors.h>
#include <mpeg_psi_parser.h>
#include <mpeg_descriptors/dvb/local_time_offset.h>
#include <Logic/DateTimeLogic.h>
#include <dvbTimeDate.h>
#include <Services/core/appState.h>
#include <QBConf.h>
#include <iso_639_table.h>
#include <main.h>
#include <assert.h>

struct DateTimeLogic_ {
    struct SvObject_ super_;
    AppGlobals appGlobals;
    time_t lastUTCTime;
};

SvLocal long int
DateTimeLogicGetCurrentTZOffset(DateTimeLogic self)
{
    struct tm local, UTC;
    long int offset;
    time_t now;

    time(&now);

    localtime_r(&now, &local);
    gmtime_r(&now, &UTC);

    offset = (local.tm_hour - UTC.tm_hour) * 3600 + (local.tm_min - UTC.tm_min) * 60;
    if (local.tm_yday != UTC.tm_yday) {
        if (local.tm_yday == UTC.tm_yday + 1 || local.tm_yday == 0) {
            offset += 24 * 3600;
        } else {
            offset -= 24 * 3600;
        }
    }

    return offset;
}

SvLocal void
DateTimeLogicDestroy(void *self_)
{
    DateTimeLogic self = self_;
    // unset dvbTimeDate callbacks
    if (self->appGlobals->lstvDVBTimeDateInitialized) {
        QBDVBTimeSetTablesCallbacks(NULL, NULL, NULL);
    }
}

SvLocal SvType
DateTimeLogic_getType(void)
{
    static const struct SvObjectVTable_ objectVTable = {
        .destroy = DateTimeLogicDestroy
    };
    static SvType t = NULL;
    if (!t) {
        t =        SvTypeCreateManaged("DateTimeLogic",
                            sizeof(struct DateTimeLogic_),
                            SvObject_getType(),
                            &t,
                            SvObject_getType(), &objectVTable,
                            NULL);
    }
    return t;
}

SvLocal void
DateTimeLogicTDTReceived(SvObject self_, const mpeg_tdt_parser *TDT, int tunerIdx)
{
    DateTimeLogic self = (DateTimeLogic) self_;

    time_t UTC = -1;
    mpeg_tdt_parser_get_utctime(TDT, &UTC);
    if (UTC > 0) {
        self->lastUTCTime = UTC;
        lstvDVBTimeDateUpdateTime(UTC);
    }
}

SvLocal void
DateTimeLogicApplyTimeZoneOffset(DateTimeLogic self, long int offset)
{
    const char *TZ = getenv("TZ");

    long int currentOffset = DateTimeLogicGetCurrentTZOffset(self);

    if (likely(offset == currentOffset)) {
        // current time zone setting is correct
        if (SvOnce()) {
            SvLogNotice("DateTimeLogic: time zone offset from TOT (%ld) matches current time zone setting '%s'", offset, TZ);
        }
        return;
    }

    SvLogNotice("DateTimeLogic: time zone offset from TOT (%ld) differs from current time zone offset (%ld)", offset, currentOffset);

    // try to get time zone name
    const char *timeZoneName = tzname[0];
    struct tm localTime;
    time_t now = time(NULL);
    localtime_r(&now, &localTime);
    if (localTime.tm_isdst > 0) {
        // we are currently using daylight saving time
        if (tzname[1])
            timeZoneName = tzname[1];
    }

    // generate ad-hoc time zone using current offset
    const size_t bufferSize = strlen(timeZoneName) + 9;
    char buffer[bufferSize];
    if (offset == 0) {
        snprintf(buffer, bufferSize, "%s0", timeZoneName);
    } else {
        long int hours = labs(offset) / 3600;
        long int minutes = (labs(offset) - hours * 3600) / 60;
        // the sign is reversed, because POSIX states that positive
        // offsets are west from UTC (see man tzset(3))
        snprintf(buffer, sizeof(buffer), "%s%c%ld:%02ld",
                 timeZoneName, (offset < 0) ? '+' : '-', hours, minutes);
    }

    // setup new time zone
    setenv("TZ", buffer, 1);
    tzset();
    SvLogNotice("DateTimeLogic: changed time zone from %s to %s", TZ, buffer);
    lstvDVBTimeDateNotify();
}

SvLocal void
DateTimeLogicUseTimeZone(DateTimeLogic self, const struct _dvb_local_time_offset_desc_item *offset)
{
    time_t nextChange =   offset->time_of_change.days + offset->time_of_change.hour * 3600
                        + offset->time_of_change.min * 60 + offset->time_of_change.sec;
    long int diff;
    if (nextChange > 0 && self->lastUTCTime >= nextChange) {
        // apply next offset
        diff = offset->next_time_offset.hour * 60 + offset->next_time_offset.min;
    } else {
        // apply current offset
        diff = offset->local_time_offset.hour * 60 + offset->local_time_offset.min;
    }
    if (offset->local_time_offset.polarity != 0)
        diff *= -60;
    else
        diff *= 60;
    DateTimeLogicApplyTimeZoneOffset(self, diff);
}

SvLocal void
DateTimeLogicTOTReceived(SvObject self_, const mpeg_tot_parser *TOT, int tunerIdx)
{
    DateTimeLogic self = (DateTimeLogic) self_;

    const char *autoTimeZone = QBConfigGet("AUTOTZ");
    if (autoTimeZone && !strcmp(autoTimeZone, "NO"))
        return;

    mpeg_descriptor_map *map = mpeg_tot_parser_get_descriptor_map(TOT);
    list_t *list = mpeg_descriptor_map_get_list(map, DVB_DESC_TAG_LOCAL_TIME_OFFSET);
    if (!list || list__empty(list))
        return;

    mpeg_descriptor *iter = NULL;
    list_linkage_t *aux;
    dvb_local_time_offset_desc desc;
    bool timeZoneApplied = false;
    char languageShortcut[3];
    const char *language = QBConfigGet("LANG");
    strncpy(languageShortcut, language, 2);
    languageShortcut[2] = '\0';
    const char *countryCode = iso639_1to3(languageShortcut);

    list__for_each(list, iter, aux) {
        dvb_local_time_offset_desc_init(&desc, &(iter->data));
        for (int i = 0; i < desc.cnt; i++) {
            struct _dvb_local_time_offset_desc_item *offset = &(desc.offsets[i]);
            assert(offset != NULL);
            // in generic usage app checks if there's only one time zone. If this condition is fulfilled then app is looking for
            // country code that matches chosen menu language. In the last resort the first time zone from stream is applied.
            if (desc.cnt == 1 || (offset->country_code && !strcasecmp(offset->country_code, countryCode))) {
                DateTimeLogicUseTimeZone(self, offset);
                timeZoneApplied = true;
                break;
            }
        }

        if (!timeZoneApplied && desc.cnt) {
            // Suitable time zone not found. Apply the first one...
            struct _dvb_local_time_offset_desc_item *offset = &(desc.offsets[0]);
            DateTimeLogicUseTimeZone(self, offset);
        }
        dvb_local_time_offset_desc_destroy(&desc);
    }
}

DateTimeLogic DateTimeLogicCreate(AppGlobals appGlobals)
{
    DateTimeLogic self = (DateTimeLogic) SvTypeAllocateInstance(DateTimeLogic_getType(), NULL);

    self->appGlobals = appGlobals;
    self->lastUTCTime = -1;

    if (!getenv("TZ")) {
        setenv("TZ", "UTC0", 1);
        tzset();
    }

    QBDVBTimeSetTablesCallbacks(DateTimeLogicTDTReceived, DateTimeLogicTOTReceived, (SvObject) self);

    return self;
}
