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

#ifndef SUBTITLE_TRACK_LOGIC_H_
#define SUBTITLE_TRACK_LOGIC_H_

/**
 * @file SubtitleTrackLogic.h Subtitle track logic class API
 * @brief Subtitle track logic API
 **/

#include <QBSubsManager.h>
#include <SvFoundation/SvString.h>
#include <main_decl.h>
#include <SvPlayerControllers/SvPlayerTaskControllers.h>

/**
 * @defgroup SubtitleTrackLogic Subtitle track logic class
 * @ingroup CubiTV_logic
 * @{
 **/

typedef struct SubtitleTrackLogic_ *SubtitleTrackLogic;

/**
 * Create SubtitleTrackLogic Object
 *
 * @param[in] appGlobals pointer to appGlobals object
 * @return SubtitleTrackLogic object
 */
extern SubtitleTrackLogic SubtitleTrackLogicCreate(AppGlobals appGlobals) __attribute__((weak));
extern void SubtitleTrackLogicSetPreferredLang(SubtitleTrackLogic self, SvString lang) __attribute__((weak));
extern SvString SubtitleTrackLogicGetPreferredLang(SubtitleTrackLogic self) __attribute__((weak));

extern const char * SubtitleTrackLogicCCGetPreferredChannel(void);
extern const char * SubtitleTrackLogicCCGetPreferredLang(void);

extern void SubtitleTrackLogicSubsTrackChanged(SubtitleTrackLogic self, QBSubsTrack track) __attribute__((weak));

/**
 * Set PlayerTask controllers.
 *
 * @param[in] self SubtitleTrackLogic handle
 * @param[in] controllers track controllers to be set
 */
void SubtitleTrackLogicSetPlayerTaskControllers(SubtitleTrackLogic self, SvPlayerTaskControllers controllers);

/**
 * Get associated PlayerTask controllers.
 *
 * @param[in] self SubtitleTrackLogic handle
 * @return PlayerTask controllers
 */
SvPlayerTaskControllers SubtitleTrackLogicGetPlayerTaskControllers(SubtitleTrackLogic self);

/**
 * @}
 **/

#endif
