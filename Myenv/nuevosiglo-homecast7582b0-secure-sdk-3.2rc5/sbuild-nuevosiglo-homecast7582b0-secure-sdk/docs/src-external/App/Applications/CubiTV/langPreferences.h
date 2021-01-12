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

#ifndef QBLANGPREFERENCES_H_
#define QBLANGPREFERENCES_H_

#include <SvPlayerKit/SvEPGEvent.h>
#include <QBViewport.h>
#include <dataformat/subtitle.h>
#include <main_decl.h>
#include <QBSubsManager.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * Language preferences
 *
 * @class QBLangPreferences langPreferences.h <Services/langPreferences.h>
 * @extends SvObject
 **/
typedef struct QBLangPreferences_t* QBLangPreferences;

/**
 * Get runtime type identification object representing QBLangPreferences class.
 *
 * @relates QBLangPreferences
 *
 * @return QBLangPreferences runtime type identification object
 **/
SvType
QBLangPreferences_getType(void);

QBLangPreferences QBLangPreferencesCreate(AppGlobals appGlobals);

void QBLangPreferencesLoadClosedCaptioningConf(QBLangPreferences self, SvString config);

void QBLangPreferencesSetPreferedSubtitlesTrack(void *target, QBSubsManager manager);

SvEPGEventDesc
QBLangPreferencesGetDescFromEvent(QBLangPreferences self, SvEPGEvent event);
SvString
QBLangPreferencesGetTitleFromEvent(QBLangPreferences self, SvEPGEvent ev);
SvEPGEventDesc
QBLangPreferencesGetDescFromEvent_(SvGenericObject self_, SvEPGEvent event);
void
QBLangPreferencesAddPreferredSubtitlesTrackListener(QBLangPreferences self, QBSubsManager manager);
SvGenericObject
QBLangPreferencesGetClosedCaptioningParam(QBLangPreferences self, SvString param);

/**
 * Primary EPG language type enum.
 */
typedef enum {
    QBLangPreferencesEPGLang_default,   /**< default one (subtitle language) */
    QBLangPreferencesEPGLang_menuLang   /**< menu language as primary EPG lang */
} QBLangPreferencesEPGLang;

/**
 * Set default EPG language that will be used for displaying events.
 *
 * This function is used to set different primary EPG lang according to client requests.
 *
 * @param[in] self self object
 * @param[in] defaultEPGLang default language to be set
 */
void
QBLangPreferencesSetDefaultEPGLang(QBLangPreferences self, QBLangPreferencesEPGLang defaultEPGLang);

#endif
