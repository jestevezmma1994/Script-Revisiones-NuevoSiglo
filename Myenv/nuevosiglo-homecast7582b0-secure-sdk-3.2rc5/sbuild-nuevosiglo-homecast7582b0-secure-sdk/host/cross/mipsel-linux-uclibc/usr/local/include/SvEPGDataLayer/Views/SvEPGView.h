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

/* App/Libraries/SvEPGDataLayer/SvEPGView.h */

#ifndef SV_EPG_VIEW_H
#define SV_EPG_VIEW_H

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvWeakList.h>
#include <QBDataModel3/QBDataSource.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>

/**
 * Generic EPG channel view class.
 *
 * Generic EPG channel view class implements tasks common to all
 * channel views: adding and removing channel view listeners and
 * notifying them about changes. It is of no use as a real channel
 * view, because it will be always empty.
 **/

/**
 * Notification type.
 **/
typedef enum {
   SvEPGViewNotification_channelsAdded,
   SvEPGViewNotification_channelsRemoved,
   SvEPGViewNotification_channelsChanged,
   SvEPGViewNotification_channelsReordered,
   SvEPGViewNotification_channelsReplaced
} SvEPGViewNotificationType;

/**
 * Channel ordering mode.
 */
typedef enum {
   SvEPGViewChannelNumbering_byIndex,
   SvEPGViewChannelNumbering_byNumber,
   SvEPGViewChannelNumbering_byName,
} SvEPGViewChannelNumbering;

/**
 * SvEPGView class.
 **/
typedef struct SvEPGView_ {
   struct QBDataSource_t super_;
   SvEPGViewChannelNumbering numbering;

   SvWeakList listeners;
} *SvEPGView;


extern SvType
SvEPGView_getType(void);

extern SvEPGView
SvEPGViewInit(SvEPGView self,
              SvEPGViewChannelNumbering numbering,
              SvErrorInfo *errorOut);

//deprecated, we have SvEPGViewNotify now
extern void
SvEPGViewPropagateChannelsChange(SvEPGView self,
                                 unsigned int first,
                                 unsigned int cnt);

extern void
SvEPGViewNotify(SvEPGView self,
                SvEPGViewNotificationType type,
                unsigned int idx1,
                unsigned int idx2);

extern void
SvEPGViewNotifyReplaced(SvEPGView self,
                        unsigned int idx);

extern void
SvEPGViewSetNumbering(SvEPGView self,
                      SvEPGViewChannelNumbering numbering,
                      SvErrorInfo *errorOut);

extern SvEPGViewChannelNumbering
SvEPGViewGetNumbering(SvEPGView self,
                      SvErrorInfo *errorOut);

/**
 * Notify channel list listeners about changes in channel list.
 *
 * @param self          SvEPGView handle
 * @param eventType     type of event
 * @param channel       handle to channel that caused the notification
 **/
extern void
SvEPGViewNotifyChannelListListeners(SvEPGView self,
                                    SvEPGViewNotificationType eventType,
                                    SvTVChannel channel);


#endif
