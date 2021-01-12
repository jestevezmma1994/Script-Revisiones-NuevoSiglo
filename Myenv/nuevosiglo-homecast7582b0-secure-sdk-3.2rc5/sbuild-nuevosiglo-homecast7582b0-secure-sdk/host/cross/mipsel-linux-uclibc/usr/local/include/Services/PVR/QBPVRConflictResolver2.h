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

#ifndef QB_PVR_CONFLICT_RESOLVER_2_H_
#define QB_PVR_CONFLICT_RESOLVER_2_H_

/**
 * @file QBPVRConflictResolver2.h PVR Conflicts resolver 2
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBPVRTypes.h>
#include <QBPVRProvider.h>
#include <SvEPGDataLayer/SvEPGManager.h>

/**
 * Wrapper for pvr providers returning possible conflicts array and conflict reason.
 *
 * @class QBPVRConflictResolver2 QBPVRConflictResolver2.h <Services/PVR/QBPVRConflictResolver2.h>
 * @extends SvObject
 *
 * It is responsible for handling pvr conflicts resolving. It asks PVR Provider if any conflicts exist.
 * If a conflicts array is obtained, it filters out keyword and series recordings and returns the array
 * in final form.
 **/
typedef struct QBPVRConflictResolver2_ * QBPVRConflictResolver2;

typedef struct QBPVRConflictResolver2Result_ {
    struct SvObject_ super_;            ///< base class handle

    QBPVRProviderConflictReason reason; ///< Conflict reason
    SvArray conflicts;                  ///< array of conflicts
} *QBPVRConflictResolver2Result;

/**
 * Create QBPVRConflictResolver2.
 *
 * @param[in] pvrProvider       QBPVRProvider handle
 * @param[in] params            Parameters of newly scheduled recording
 * @param[in] ignoredRecordings array of recordings to ignore when checking for conflicts and delete if no conflicts are found
 * @param[out] errorOut         SvErrorInfo handle
 * @return                      QBPVRConflictResolver2 handle, @c NULL if error occured
 */
extern QBPVRConflictResolver2
QBPVRConflictResolver2Create(QBPVRProvider pvrProvider, QBPVRRecordingSchedParams params,
                             SvArray ignoredRecordings, SvErrorInfo *errorOut);

/**
 * Performs conflict resolving. If no conflicts are found, schedules recording. If there are conflicts,
 * returns conflict array and reason of conflict.
 *
 * @param[in] self          QBPVRConflictResolver2 handle
 * @param[out] errorOut     errorhandle
 * @return                  QBPVRConflictResolver2Result containing reason of conflicts and array of conflicts, @c NULL if no conflicts.
 */
extern QBPVRConflictResolver2Result
QBPVRConflictResolver2GetResult(QBPVRConflictResolver2 self, SvErrorInfo *errorOut);

#endif /* QB_PVR_CONFLICT_RESOLVER_2_H_ */
