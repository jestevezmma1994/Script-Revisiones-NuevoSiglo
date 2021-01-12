/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SVEPGEVENTSLIST_H_
#define SVEPGEVENTSLIST_H_

#include <SvCore/SvErrorInfo.h>
#include <SvPlayerKit/SvTimeRange.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <QBDataModel3/QBSortedList.h>

typedef struct SvEPGEventsList_t *SvEPGEventsList;

extern SvType
SvEPGEventsList_getType(void);

extern void
SvEPGEventsListSetChannel(SvEPGEventsList self,
                          SvTVChannel channel,
                          SvErrorInfo *errorOut);

extern void
SvEPGEventsListSetTimeRange(SvEPGEventsList self,
                            SvTimeRange *timeRange,
                            SvErrorInfo *errorOut);

extern void
SvEPGEventsListUpdateEPG(SvEPGEventsList self,
                         const SvTimeRange *timeRange,
                         SvTVChannel channel);

extern bool
SvEPGEventsListIsCurrentlyOnChannel(SvEPGEventsList self,
                                    SvTVChannel tvCh);

extern SvTimeRange
SvEPGEventsListGetTimeRange(SvEPGEventsList self);

#endif /* SVEPGEVENTSLIST_H_ */

