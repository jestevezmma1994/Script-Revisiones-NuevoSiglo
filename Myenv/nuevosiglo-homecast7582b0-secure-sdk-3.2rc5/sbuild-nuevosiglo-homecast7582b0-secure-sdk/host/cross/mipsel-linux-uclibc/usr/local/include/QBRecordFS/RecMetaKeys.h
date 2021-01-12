/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef REC_META_KEYS_H
#define REC_META_KEYS_H


// about state
#define REC_META_KEY__STATE             "rec:state"         // string,  one of REC_META_STATE_*
//
#define REC_META_STATE__UNCOMPLETED  "Uncompleted" // in progress, or interrupted suddenly.
#define REC_META_STATE__COMPLETED    "Completed"   // stopped due to end of input stream.
#define REC_META_STATE__STOPPED      "Stopped"     // stopped : see "STOP_REASON".

#define REC_META_KEY__ENCRYPTION            "rec:encryption" // string,  one of REC_META_ENCRYPTION_*
#define REC_META_ENCRYPTION__REENCRYPTED    "Reencrypted"    // content recorded as reencrypted

#define REC_META_KEY__REENCRYPTION_STATE        "rec:reencryption_state" // string,  one of REC_META_REENCRYPTION_STATE_*
#define REC_META_REENCRYPTION_STATE__STARTED    "Started"  // converting to reencrypted content started
#define REC_META_REENCRYPTION_STATE__DONE       "Done"     // converting to reencrypted content done
#define REC_META_REENCRYPTION_STATE__BROADCASTER_LIMITATION "BroadcasterLimitation" // converting to reencrypted content finished with 'broadcaster limitation' error
#define REC_META_REENCRYPTION_STATE__REENCRYPTION_FAILED    "ReencryptionFailed"    // converting to reencrypted content finished with 'reencryption failed' error

//
#define REC_META_KEY__STOP_REASON       "rec:stop_reason"   // string,  one of REC_META_STOP_REASON_*
//
#define REC_META_STOP_REASON__MANUAL    "Manual"  // manually.
#define REC_META_STOP_REASON__NO_SPACE  "NoSpace" // due to lack of space on storage
#define REC_META_STOP_REASON__SOURCE    "Source"  // due to some source-related error, e.g. server connection is lost.
// ...
//


// about storage
#define REC_META_KEY__DATAFILE          "rec:datafile"      // string,  "device_id/file_id", e.g. "2389ad18b5cf4/17", or "0/12"

//about meta
#define REC_META_IS_META_AUTHENTICATED  "rec:is_meta_authenticated" // bool

// about recording process
#define REC_META_KEY__START_TIME            "rec:start_time"            // time_t
#define REC_META_KEY__START_TIME_TZ         "rec:start_time_tz"         // int, in seconds
#define REC_META_KEY__STOP_TIME             "rec:stop_time"             // time_t
#define REC_META_KEY__STOP_TIME_TZ          "rec:stop_time_tz"          // int, in seconds
#define REC_META_KEY__EXPIRATION_TIME       "rec:expiration_time"       // time_t
#define REC_META_KEY__EXPIRATION_TIME_TZ    "rec:expiration_time_tz"    // int, in seconds
#define REC_META_KEY__PROGRESS              "rec:progress"              // double, fraction [0..1], how much of the content is already processed.
#define REC_META_KEY__EVENTS                "rec:events"                // events

// about original content
#define REC_META_KEY__SOURCE_URL        "rec:source_url"      // string,  "rtsp://bla/bla/bla"
#define REC_META_KEY__SOURCE_DURATION   "rec:source_duration" // double,  in seconds of content
// DRM?

// Block encryption
#define REC_META_KEY__ENCRYPTION_RECORD_METADATA "rec:encryption_record_metadata"
#define REC_META_KEY__ENCRYPTION_BLOCK_METADATA "rec:encryption_block_metadata"

// about recorded content
#define REC_META_KEY__DURATION          "rec:duration"       // double,  in seconds of content

#define REC_META_KEY_SKIP_FIRST_4KB     "rec:skip_first_4kB" // bool

#define REC_META_KEY__EVENTS_SOURCE     "rec:events_source"
#define REC_META_KEY__EVENTS_SOURCE_PRESENT_FOLLOWING   "present_following"
#define REC_META_KEY__EVENTS_SOURCE_EPG_MANAGER         "epg_manager"

#endif // #ifndef REC_META_KEYS_H
