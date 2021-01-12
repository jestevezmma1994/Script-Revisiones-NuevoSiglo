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

#ifndef QBNPVRDIRECTORY_H
#define QBNPVRDIRECTORY_H

/**
 * @file QBnPVRDirectory.h QBnPVRDirectory interface file.
 **/

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>
#include <stdbool.h>

#include <NPvr/QBnPVRQuota.h>
#include <NPvr/QBnPVRRecording.h>
#include <NPvr/QBnPVRRecordingSchedule.h>

/**
 * @defgroup QBnPVRDirectory The QBnPVRDirectory class.
 * @ingroup NPvr
 * @{
 **/


typedef enum {
    QBnPVRDirectoryListenerState_Added,
    QBnPVRDirectoryListenerState_Changed,
    QBnPVRDirectoryListenerState_Removed
} QBnPVRDirectoryListenerState;

typedef enum {
    QBnPVRDirectoryType_Normal,
    QBnPVRDirectoryType_Keyword,
    QBnPVRDirectoryType_Series
} QBnPVRDirectoryType;

typedef struct QBnPVRDirectory_ {
    struct SvObject_ super_;

    SvString id;
    SvString name;
    QBnPVRDirectoryType type;
    QBnPVRRecordingSeries seriesId;
    QBnPVRRecordingKeyword keywordId;
    QBnPVRRecordingSpaceRecovery scheduleRecoveryOption;
    QBnPVRRecordingSchedulePremiere schedulePremiereOption;
    QBnPVRQuota quota;
} *QBnPVRDirectory;

/**
 * Get runtime type identification object representing QBnPVRDirectory class.
**/
extern SvType
QBnPVRDirectory_getType(void);

typedef const struct QBnPVRProviderDirectoryListener_ {
    void (*added)(SvObject self, QBnPVRDirectory directory);
    void (*changed)(SvObject self, QBnPVRDirectory directory);
    void (*removed)(SvObject self, QBnPVRDirectory directory);
} *QBnPVRProviderDirectoryListener;

/**
 * Get runtime type identification object representing
 * QBnPVRProviderDirectoryListener interface.
 **/
extern SvInterface
QBnPVRProviderDirectoryListener_getInterface(void);

/**
 * QBnPVRDirectory create params.
 **/
typedef struct {
    SvString name;
} QBnPVRDirectoryCreateParams_;
typedef QBnPVRDirectoryCreateParams_* QBnPVRDirectoryCreateParams;

/**
 * Create empty params that can be used to create QBnPVRDirectory.
 * @return empty-initialized QBnPVRDirectoryCreateParams
 */
QBnPVRDirectoryCreateParams_
QBnPVRDirectoryGetEmptyCreateParams(void);

/**
 * QBnPVRDirectory delete params.
 **/
typedef struct {
    SvString id;
    bool keepContents;
} QBnPVRDirectoryDeleteParams_;
typedef QBnPVRDirectoryDeleteParams_* QBnPVRDirectoryDeleteParams;

/**
 * Create empty params that can be used to delete QBnPVRDirectory.
 * @return empty-initialized QBnPVRDirectoryDeleteParams_
 */
QBnPVRDirectoryDeleteParams_
QBnPVRDirectoryGetEmptyDeleteParams(void);

/**
 * QBnPVRDirectory update params.
 **/
typedef struct {
    SvString id;
    SvString name;
    bool updateQuota;
    QBnPVRProviderQuotaUpdate quota;
    bool updatePolicy;
    QBnPVRQuotaPolicy policy;
} QBnPVRDirectoryUpdateParams_;
typedef QBnPVRDirectoryUpdateParams_* QBnPVRDirectoryUpdateParams;

/**
 * Create empty params that can be used to update QBnPVRDirectory.
 * @return empty-initialized QBnPVRDirectoryUpdateParams_
 */
QBnPVRDirectoryUpdateParams_
QBnPVRDirectoryGetEmptyUpdateParams(void);

/**
 * Tell if the QBnPVRDirectory was created for keyword NPvr recordings
 * @param[in] dir   directory to be checked
 * @return          true if the directory is keyword type
 */
bool
QBnPVRDirectoryIsKeyword(QBnPVRDirectory dir);

/**
 * Tell if the QBnPVRDirectory was created for series NPvr recordings
 * @param[in] dir   directory to be checked
 * @return          true if the directory is series type
 */
bool
QBnPVRDirectoryIsSeries(QBnPVRDirectory dir);

/** @} */

#endif // QBNPVRDIRECTORY_H
