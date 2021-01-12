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

#ifndef QB_PVR_TYPES_H_
#define QB_PVR_TYPES_H_

/**
 * @file QBPVRTypes.h QBPVRTypes set.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvTime.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup QBPVRTypes PVR common types.
 * @ingroup QBPVRProvider
 * @{
 **/

/**
 * PVR search provider recording test callback.
 *
 * This callback is used to test if recording matches search criteria.
 *
 * @param[in] prv           private field
 * @param[in] recording     recording which should be tested
 * @param[in] keywords      array of keywords
 * @return                  @c true if recording matches search criteria, @c false otherwise
 **/
typedef bool (*QBPVRSearchProviderTestCallback)(void *prv, SvObject recording, SvArray keywords);

/**
 * PVR Search provider test associated params.
 */
typedef struct QBPVRSearchProviderTest_ {
    QBPVRSearchProviderTestCallback callback;    ///< callback to be used for checking if recording matches search criteria
    void *prv;                                   ///< callback private field
} QBPVRSearchProviderTest;

/**
 * PVR provider type.
 **/
typedef enum {
    QBPVRProviderType_unknown,          ///< unknown PVR type
    QBPVRProviderType_disk,             ///< disk PVR type
    QBPVRProviderType_network,          ///< network PVR type
} QBPVRProviderType;

/**
 * Quota delete policy.
 **/
typedef enum {
    QBPVRQuotaPolicy_deleteOldest,      ///< delete oldest recordings
    QBPVRQuotaPolicy_deleteNone         ///< don't delete automatically, let user choose what should be deleted
} QBPVRQuotaPolicy;

/**
 * Quota units.
 */
typedef enum {
    QBPVRQuotaUnits_bytes,              ///< bytes
    QBPVRQuotaUnits_seconds,            ///< seconds
    QBPVRQuotaUnits_events,             ///< events
} QBPVRQuotaUnits;

/**
 * Quota.
 */
typedef struct {
    int64_t limit;     ///< quota limit, @c -1 if unknown
    int64_t used;      ///< quota used, @c -1 if unknown
} QBPVRQuota;

/**
 * QBPVRDirectory create params.
 **/
typedef struct QBPVRDirectoryCreateParams_ {
    struct SvObject_ super_;    ///< super type
    SvString name;              ///< directory name
} *QBPVRDirectoryCreateParams;

/**
 * Get runtime type identification object representing QBPVRDirectoryCreateParams class.
 *
 * @return QBPVRDirectoryCreateParams runtime type identification object
 **/
extern SvType
QBPVRDirectoryCreateParams_getType(void);

/**
 * QBPVRDirectory delete params.
 **/
typedef struct QBPVRDirectoryDeleteParams_ {
    struct SvObject_ super_;            ///< super type
    bool keepContents;                  ///< should content be kept
} *QBPVRDirectoryDeleteParams;

/**
 * Get runtime type identification object representing QBPVRDirectoryDeleteParams class.
 *
 * @return QBPVRDirectoryDeleteParams runtime type identification object
 **/
extern SvType
QBPVRDirectoryDeleteParams_getType(void);

/**
 * Repeated recordings mode.
 */
typedef enum {
    QBPVRRepeatedRecordings_None,       ///< repeated recordings mode none
    QBPVRRepeatedRecordings_Daily,      ///< repeated recordings mode repeat daily
    QBPVRRepeatedRecordings_Weekly,     ///< repeated recordings mode repeat weakly
    QBPVRRepeatedRecordings_Weekends,   ///< repeated recordings mode repeat weekends
    QBPVRRepeatedRecordings_Weekdays,   ///< repeated recordings mode repeat weekdays
} QBPVRRepeatedRecordingsMode;

/**
 * Get repeated recordings mode from name.
 *
 * @param[in] str           repeated recordings mode name
 * @return                  repeated recordings mode
 */
QBPVRRepeatedRecordingsMode
QBPVRRepeatedRecordingsModeFromName(const char *str);

/**
 * Create repeated recordings mode name string.
 *
 * @param[in] mode          repeated recordings mode
 * @param[in] translate     should mode name string be translated to current locales version
 * @return                  repeated recordings name string
 */
SvString
QBPVRRepeatedRecordingsModeCreateName(QBPVRRepeatedRecordingsMode mode, bool translate);

/**
 * QBPVR repeated recordings info.
 **/
typedef struct QBPVRRepeatedRecordingsInfo_ {
    int start;                          ///< start time in UTC, relative to day start
    int duration;                       ///< duration in seconds
    QBPVRRepeatedRecordingsMode mode;   ///< repeated recordings mode
} QBPVRRepeatedRecordingsInfo;

/**
 * QBPVRDirectory update params.
 **/
typedef struct QBPVRDirectoryUpdateParams_ {
    struct SvObject_ super_;                    ///< super type
    bool updateName;                            ///< should name be updated
    SvString name;                              ///< directory name
    bool updateQuota;                           ///< should quota be updated
    int64_t quotaLimit;                         ///< quota update
    bool updateRepeatedInfo;                    ///< should repeated info be updated
    QBPVRRepeatedRecordingsInfo repeatedInfo;   ///< repeated info
    bool updateKeyword;                         ///< should keyword be updated
    SvString keyword;                           ///< keyword
    bool updateSeriesID;                        ///< should series ID be updated
    SvString seriesID;                          ///< series ID
} *QBPVRDirectoryUpdateParams;

/**
 * Get runtime type identification object representing QBPVRDirectoryUpdateParams class.
 *
 * @return QBPVRDirectoryUpdateParams runtime type identification object
 **/
extern SvType
QBPVRDirectoryUpdateParams_getType(void);

/**
 * QBPVRRecording space recovery mode.
 **/
typedef enum {
    QBPVRRecordingSpaceRecovery_unknown,                ///< unknown
    QBPVRRecordingSpaceRecovery_DeleteOldest,           ///< delete oldest
    QBPVRRecordingSpaceRecovery_Manual,                 ///< delete manual
} QBPVRRecordingSpaceRecovery;

/**
 * Type of the PVR recording.
 */
typedef enum {
    QBPVRRecordingType_none,            ///< none recording type
    QBPVRRecordingType_keyword,         ///< keyword recording type
    QBPVRRecordingType_series,          ///< series recording type
    QBPVRRecordingType_repeated,        ///< repeated recording type
    QBPVRRecordingType_event,           ///< event recording type
    QBPVRRecordingType_OTR,             ///< one time recording type
    QBPVRRecordingType_manual,          ///< manual recording type
} QBPVRRecordingType;

/**
 * Get recording type from name.
 *
 * @param[in] str           recording type name
 * @return                  recording type
 */
QBPVRRecordingType
QBPVRRecordingTypeFromName(const char *str);

/**
 * Create recording type name string.
 *
 * @param[in] type          recording type
 * @return                  recording type name string
 */
SvString
QBPVRRecordingTypeCreateName(QBPVRRecordingType type);

/**
 * QBPVRRecording schedule command parameters.
 **/
typedef struct QBPVRRecordingSchedParams_ {
    struct SvObject_ super_;                    ///< super type
    SvTVChannel channel;                        ///< channel handle
    QBPVRRecordingType type;                    ///< recording type
    SvString name;                              ///< name
    SvEPGEvent event;                           ///< event
    int startMargin;                            ///< start margin
    int endMargin;                              ///< end margin
    SvString keyword;                           ///< keyword
    QBPVRRecordingSpaceRecovery recovery;       ///< space recovery
    SvString seriesID;                          ///< series id
    bool recordPremieresOnly;                   ///< record premieres only
    struct {
        SvTime start;                       ///< start time in UTC
        int duration;                       ///< duration in seconds
        QBPVRRepeatedRecordingsMode mode;   ///< repeated recordings mode
    } repeatedInfo;   ///< repeated info;

    SvTime  startTime;                          ///< start time in UTC
    SvTime  stopTime;                           ///< stop time in UTC
    /// duration, if >= 0, use real-time-clock to measure when to stop this recording, don't use "stopTime"
    int duration;
} *QBPVRRecordingSchedParams;

/**
 * Get runtime type identification object representing QBPVRRecordingSchedParams class.
 *
 * @return QBPVRRecordingSchedParams runtime type identification object
 **/
extern SvType
QBPVRRecordingSchedParams_getType(void);

/**
 * QBPVRRecording update command parameters.
 **/
typedef struct QBPVRRecordingUpdateParams_ {
    struct SvObject_ super_;                    ///< super type
    bool updateTime;                            ///< should recording time be updated
    SvTime startTime;                           ///< new start time
    SvTime endTime;                             ///< new end time
    bool updateRecovery;                        ///< should space recovery mode be updated
    QBPVRRecordingSpaceRecovery recovery;       ///< new space recovery mode
} *QBPVRRecordingUpdateParams;

/**
 * Get runtime type identification object representing QBPVRRecordingUpdateParams class.
 *
 * @return QBPVRRecordingUpdateParams runtime type identification object
 **/
extern SvType
QBPVRRecordingUpdateParams_getType(void);

/**
 * @}
 **/

#endif /* QB_PVR_TYPES_H_ */
