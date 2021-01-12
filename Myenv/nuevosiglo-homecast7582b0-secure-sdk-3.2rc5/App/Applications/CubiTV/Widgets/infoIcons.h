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

#ifndef INFOICONS_H_
#define INFOICONS_H_

#include <CUIT/Core/types.h>
#include <settings.h>
#include <QBViewport.h>
#include <QBSubsManager.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvEPGDataLayer/SvEPGManager.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <QBPCRatings/QBPCList.h>
#include <Services/QBParentalControl/QBParentalControl.h>
#include <Logic/EventsLogic.h>

typedef enum{
            QBInfoIcons_AC3,
            QBInfoIcons_16_9,
            QBInfoIcons_Parental,
            QBInfoIcons_Subtitles,
            QBInfoIcons_HD,
            QBInfoIcons_Radio,
            QBInfoIcons_Audio,
            QBInfoIcons_Rating,
            QBInfoIcons_Teletext,
            QBInfoIcons_Recording,
            QBInfoIcons_Premium,

            QBInfoIcons_Count,
}QBInfoIconsSelected;

/**
 * Create new QBInfoIcons widget.
 *
 * @param[in] app         CUIT application handle
 * @param[in] widgetName  widget name
 * @param[in] eventsLogic events logic
 * @param[in] epgManager  epg manager
 * @param[in] pc          parental control
 * @return                new QBInfoIcons widget or @c NULL in case of error
 **/
SvWidget
QBInfoIconsCreate(SvApplication app,
                  const char *widgetName,
                  EventsLogic eventsLogic,
                  SvEPGManager epgManager,
                  QBParentalControl pc);

void
QBInfoIconsSetState(SvWidget w, QBInfoIconsSelected icon, int value);

/**
 * Set bitmap for chosen icon using tag. Tags are loaded form settings file
 * along with bitmaps.
 *
 * @param[in]   w       widget handle
 * @param[in]   icon    chosen icon to set
 * @param[in]   tag     name of required bitmap
 */
void
QBInfoIconsSetStateByTag(SvWidget w, QBInfoIconsSelected icon, SvString tag);


void QBInfoIconsSetByViewport(SvWidget w, QBViewport viewport);
void QBInfoIconsSetByEvent(SvWidget w, SvEPGEvent event);
void QBInfoIconsSetByTVChannel(SvWidget w, SvTVChannel channel);
void QBInfoIconsSetByFormat(SvWidget w, struct svdataformat* format);
void QBInfoIconsSetBySubsManager(SvWidget w, QBSubsManager subsManager);
void QBInfoIconsSetByMovie(SvWidget w, SvDBRawObject movie);
void QBInfoIconsSetByRating(SvWidget w, QBPCRating rating);
void QBInfoIconsClear(SvWidget w);


typedef struct QBInfoIconsConstructor_t *QBInfoIconsConstructor;

/**
 * Create new QBInfoIconsConstructor.
 *
 * @param[in] app         CUIT application handle
 * @param[in] widgetName  widget name
 * @param[in] eventsLogic events logic
 * @param[in] epgManager  epg manager
 * @param[in] pc          parental control
 * @return                new QBInfoIconsConstructor or @c NULL in case of error
 **/
QBInfoIconsConstructor
QBInfoIconsConstructorCreate(SvApplication app,
                             const char *widgetName,
                             EventsLogic eventsLogic,
                             SvEPGManager epgManager,
                             QBParentalControl pc);

SvWidget
QBInfoIconsCreateFromConstructor(QBInfoIconsConstructor constructor);


#endif /* INFOICONS_H_ */
