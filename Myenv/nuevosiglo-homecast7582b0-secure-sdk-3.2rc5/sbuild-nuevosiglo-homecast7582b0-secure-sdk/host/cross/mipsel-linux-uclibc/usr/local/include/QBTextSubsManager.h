/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TEXT_SUBS_MANAGER_H
#define QB_TEXT_SUBS_MANAGER_H

/**
 * @file QBTextSubsManager.h
 * @brief Text subtitles manager library API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvFoundation/SvCoreTypes.h>
#include <SvPlayerManager/SvPlayerTask.h>
#include <QBSubsManager.h>
#include <CUIT/Core/types.h>

#ifdef __cplusplus
extern "C" {
#endif


// forward declaration
struct svdataformat;


/**
 * @defgroup QBTextSubsManager QBTextSubsManager: text subtitles manager library
 * @ingroup DataLayer
 * @{
 **/

/**
 * Text subtitles manager class.
 * @class QBTextSubsManager
 * @extends SvObject
 **/
typedef struct QBTextSubsManager_ *QBTextSubsManager;

/**
 * Create text subtitles manager.
 *
 * @memberof QBTextSubsManager
 *
 * @param[in] app           CUIT application handle
 * @param[in] playerTask    player task handle
 * @param[in] format        description of the current stream carrying text subtitles
 * @param[in] subsManager   subtitles manager handle
 * @return                  created text subtitles manager, @c NULL in case of error
 **/
extern QBTextSubsManager
QBTextSubsManagerCreate(SvApplication app, SvPlayerTask playerTask, const struct svdataformat *format, QBSubsManager subsManager);

/**
 * Stop asynchronous tasks of the text subtitles manager.
 *
 * @memberof QBTextSubsManager
 *
 * @param[in] self          text subtitles manager handle
 **/
extern void
QBTextSubsManagerStop(QBTextSubsManager self);


/**
 * Text subtitles track class.
 * @class QBTextSubsTrack
 * @extends QBSubsTrack
 **/
typedef struct QBTextSubsTrack_ *QBTextSubsTrack;

/**
 * Get runtime type identification object representing QBTextSubsTrack class.
 *
 * @return text subtitles track class
 **/
extern SvType
QBTextSubsTrack_getType(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif //QB_TEXT_SUBS_MANAGER_H
