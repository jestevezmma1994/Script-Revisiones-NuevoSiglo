/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_CAT_MANAGER_H
#define QB_CAT_MANAGER_H

#include <fibers/c/fibers.h>
#include <QBTunerTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBCatManager_s;
typedef struct QBCatManager_s  QBCatManager;

QBCatManager* QBCatManagerCreate(SvScheduler sched);
void QBCatManagerDestroy(QBCatManager* man);

void QBCatManagerAddTuner(QBCatManager* man, int tunerNum, struct QBTunerMuxId muxid);
void QBCatManagerSetPlaybackTuner(QBCatManager* man, int tunerNum);

void QBCatManagerEmitFakeCAT(QBCatManager* man, int tunerNum,
                             int catLen, int version, bool goodCrc);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_CAT_MANAGER_H
