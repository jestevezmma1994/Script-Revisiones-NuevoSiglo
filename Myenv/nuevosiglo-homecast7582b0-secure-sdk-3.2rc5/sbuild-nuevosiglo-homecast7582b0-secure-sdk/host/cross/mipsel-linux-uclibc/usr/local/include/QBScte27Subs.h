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

#ifndef QBSCTE27SUBS_H_
#define QBSCTE27SUBS_H_

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvPlayerManager/SvPlayerManager.h>
#include <QBSubsManager.h>
#include <dataformat/sv_data_format.h>

typedef struct displayStandard_s {
    int width, height, frames;
} *QBScte27SubsDisplayStandard;

typedef struct QBScte27SubsManager_t *QBScte27SubsManager;

/**
 * Create QBScte27SubsManager.
 *
 * @param[in] subsManager             subtitles manager handle
 * @param[in] playerTask              player task handle
 * @param[in] format                  data format
 * @return                            created QBScte27SubsManager, @c NULL in case of error
 **/
QBScte27SubsManager QBScte27SubsManagerCreate(QBSubsManager subsManager, SvPlayerTask playerTask, const struct svdataformat* format);
void QBScte27SubsManagerStop(QBScte27SubsManager self);
SvType QBScte27SubsTrack_getType(void);

void QBScte27SubsManagerEnableNTSCHack(QBScte27SubsManager self, bool enable);

void QBScte27SubsSetCustomDisplayStandardTab(QBScte27SubsDisplayStandard customDisplayStandardTab, int tabCount);

#endif /* QBSCTE27SUBS_H_ */
