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

#ifndef QBRECORDINGICON_H_
#define QBRECORDINGICON_H_

#include <CUIT/Core/types.h>
#include <SvPlayerKit/SvEPGEvent.h>
#include <QBPVRProvider.h>

typedef struct QBRecordingIconConstructor_t *QBRecordingIconConstructor;
typedef struct QBRecordingIcon_t *QBRecordingIcon;

/**
 * Create QBRecordingIcon widget.
 *
 * @param[in]  app           CUIT application handle
 * @param[in]  pvrProvider   PVR provider, may be @c NULL
 * @param[in]  nPVRProvider  nPVR provider, may be @c NULL
 * @param[in]  constructor   QBRecordingIconConstructor
 * @return                   created widget
 **/
SvWidget
QBRecordingIconNewFromConstructor(SvApplication app,
                                  QBPVRProvider pvrProvider,
                                  SvObject nPVRProvider,
                                  QBRecordingIconConstructor constructor);

QBRecordingIconConstructor
QBRecordingIconConstructorNew(const char *name);
void
QBRecordingIconSetByEPGEvent(SvWidget widget, SvEPGEvent event);

void
QBRecordingIconUpdaterStart(void);

void
QBRecordingIconUpdaterStop(void);
#endif /* QBRECORDINGICON_H_ */