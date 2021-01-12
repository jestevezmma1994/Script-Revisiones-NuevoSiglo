/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBVBITELETEXT_H
#define QBVBITELETEXT_H

/**
 * @file QBVBITeletext.h
 * @brief Service for displaying teletext through VBI on analog outputs.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include <QBTeletextLineReceiver.h>
#include <fibers/c/fibers.h>

/**
 * @defgroup QBVBITeletext  QBVBITeletext service
 * @{
 **/

/**
 * QBVBITeletext service
 *
 * There are at least two ways for displaying teletext on TV. In the first one
 * STB decodes and renders teletext by hand (as done in QBTeletextDisplayer).
 * The second one provides teletext data to the TV and the TV itself will
 * decode and render teletext.
 *
 * QBVBITeletext allows to use the second method by providing teletext data
 * through VBI. In this solution teletext is encoded in invisible (to the user)
 * video output, but interpreted by the TV. Due to technical reasons this
 * solution only works on SD outputs with PAL resolution.
 *
 * Teletext data is obtained from DVB via QBTeletextLineReceiver and then pushed
 * to appropriate video outputs using QBPlatformHAL APIs. Data from
 * QBTeletextLineReceiver must be transformed into data that is understood by
 * QBPlatformHAL APIs. It is done by following EN 300 472 - V1.3.1 spec.
 *
 * @class QBVBITeletext
**/
typedef struct QBVBITeletext_* QBVBITeletext;

/**
 * Create an instance of the service. Upon start it will listen to the
 * lineReceiver for data.
 * @param[in] lineReceiver instance of QBTeletextLineReceiver, may be @c NULL
 * @return created service handle or @c NULL in case of error
 */
QBVBITeletext QBVBITeletextCreate(QBTeletextLineReceiver lineReceiver);

/**
 * Starts the service, it will now start listening to lineReceiver
 * and push data through VBI using QBPlatformHAL.h API.
 * @param[in] self service handle
 * @param[in] scheduler instance of SvScheduler on which the service should run
 **/
void QBVBITeletextStart(QBVBITeletext self, SvScheduler scheduler);

/**
 * Stops the service.
 * @param[in] self service handle
 */
void QBVBITeletextStop(QBVBITeletext self);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* QBVBITELETEXT_H */

