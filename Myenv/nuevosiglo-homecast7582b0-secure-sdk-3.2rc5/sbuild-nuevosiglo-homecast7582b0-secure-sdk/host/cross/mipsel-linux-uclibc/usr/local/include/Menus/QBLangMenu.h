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

#ifndef QB_LANG_MENU_H
#define QB_LANG_MENU_H

#include <SvPlayerControllers/SvPlayerAudioTrack.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvString.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <QBDvbSubsManager.h>
#include <Services/langPreferences.h>
#include <main_decl.h>


typedef struct QBLangMenu_t *QBLangMenu;

/**
 * Called when new track is set by user.
 *
 * @param[in] ptr pointer registered with QBLangMenuSetAudioCallback function
 * @param[in] audioTrack current track
 */
typedef void (*QBLangMenuAudioCallback)(void *ptr, SvPlayerAudioTrack audioTrack);
typedef void (*QBLangMenuSubsCallback)(void *ptr, QBSubsTrack subsTrack);

typedef enum{
    QBLangMenuSubMenu_Audio,
    QBLangMenuSubMenu_Subtitles,
} QBLangMenuSubMenu;

void QBLangMenuSetSubsLang(QBLangMenu menu, SvString subsLang);
QBLangMenu QBLangMenuNew(AppGlobals appGlobals, QBSubsManager subsManager);
void QBLangMenuShow(QBLangMenu self, QBLangMenuSubMenu submenu);
void QBLangMenuHide(QBLangMenu self);
bool QBLangMenuIsVisible(QBLangMenu self);
void QBLangMenuUpdateAudioTracks(QBLangMenu self);
void QBLangMenuSetAudioCallback(QBLangMenu menu, QBLangMenuAudioCallback callback, void *callbackData);
void QBLangMenuSetSubsCallback(QBLangMenu menu, QBLangMenuSubsCallback callback, void *callbackData);

#endif
