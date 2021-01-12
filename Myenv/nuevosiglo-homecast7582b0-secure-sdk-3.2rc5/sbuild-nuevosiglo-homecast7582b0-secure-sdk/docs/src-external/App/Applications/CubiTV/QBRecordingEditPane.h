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

#ifndef QBRECORDINGEDITPANE_H_
#define QBRECORDINGEDITPANE_H_

/**
 * @file QBRecordingEditPane.h Recording data edit pane
 **/

#include <ContextMenus/QBContextMenu.h>
#include <Logic/PVRLogic.h>

#include <SvPlayerKit/SvEPGEvent.h>
#include <NPvr/QBnPVRRecording.h>
#include <QBPVRRecording.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvLocalTime.h>
#include <main_decl.h>

/**
 * @defgroup QBRecordingEditPane Recording edit pane
 * @ingroup CubiTV
 * @{
 **/

/**
 * Edition pane for the Pvr/NPvr recordings
 *
 * @class QBRecordingEditPane
 **/
typedef struct QBRecordingEditPane_t* QBRecordingEditPane;

/**
 * Get runtime type identification object representing QBRecordingEditPane class.
 *
 * @return QBRecordingEditPane class
 **/
SvType QBRecordingEditPane_getType(void);

/**
 * Pane type for the appropriate recording type: manual or keyword
 **/
typedef enum {
    QBRecordingEditPane_Manual,
    QBRecordingEditPane_Keyword
} QBRecordingEditPaneType;

/**
 * Common constuctor function parameters.
 */
struct QBRecordingEditPaneCreationSettings_ {
    AppGlobals appGlobals;              /**< AppGlobals handle */
    const char *settingsFileName;       /**< settings file name to be used for the pane construction */
    const char *optionsFileName;        /**< options file name to be used for the pane construction */
    QBContextMenu contextMenu;          /**< context menu that is owner of the pane */
    int level;                          /**< level at which pane widget should be attached */
};
typedef struct QBRecordingEditPaneCreationSettings_* QBRecordingEditPaneCreationSettings;

/**
 * Create PVR recording edit pane basing on recording data.
 * @param[in] creationSettings  basic constructor parameters struct
 * @param[in] recording         PVR recording object handle
 * @param[in] recordingType     PVR recording type: QBRecordingEditPane_Manual or QBRecordingEditPane_Keyword
 * @param[out] errorOut         error info
 * @return QBRecordingEditPane object or NULL
 */
QBRecordingEditPane
QBRecordingEditPaneCreatePvrWithRec(QBRecordingEditPaneCreationSettings creationSettings,
                                    QBPVRRecording recording,
                                    QBRecordingEditPaneType recordingType,
                                    SvErrorInfo* errorOut);

/**
 * Create PVR recording edit pane basing on channel data.
 * @param[in] creationSettings  basic constructor parameters struct
 * @param[in] channel           SvTVChannel of the recording
 * @param[in] recordingType     PVR recording type: QBRecordingEditPane_Manual or QBRecordingEditPane_Keyword
 * @param[out] errorOut         error info
 * @return QBRecordingEditPane object or NULL
 */
QBRecordingEditPane
QBRecordingEditPaneCreatePvrWithChannel(QBRecordingEditPaneCreationSettings creationSettings,
                                        SvTVChannel channel,
                                        QBRecordingEditPaneType recordingType,
                                        SvErrorInfo* errorOut);

/**
 * Create keyword NPVR recording edit pane basing on event data.
 * @param[in] creationSettings  basic constructor parameters struct
 * @param[in] event             event data
 * @param[out] errorOut         error info
 * @return QBRecordingEditPane object or NULL
 */
QBRecordingEditPane
QBRecordingEditPaneCreateNPvrKeywordWithEvent(QBRecordingEditPaneCreationSettings creationSettings,
                                              SvEPGEvent event,
                                              SvErrorInfo *errorOut);

/**
 * Set initial keyword that will be presented in the pane.
 * @param[in] self      QBRecordingEditPane handle
 * @param[in] keyword   keyword to be displayed
 */
void QBRecordingEditPaneSetKeyword(QBRecordingEditPane self, SvString keyword);

/**
 * Set pane title.
 * @param[in] self      QBRecordingEditPane handle
 * @param[in] name      pane title
 */
void QBRecordingEditPaneSetName(QBRecordingEditPane self, SvString name);

/**
 * Set recording time range.
 * @param[in] self      QBRecordingEditPane handle
 * @param[in] start     edited recording start time
 * @param[in] stop      edited recording end time
 */
void QBRecordingEditPaneSetTimeRange(QBRecordingEditPane self, SvLocalTime start, SvLocalTime stop);

/**
 * Set repetition mode for the edited PVR recording type.
 * @param[in] self      QBRecordingEditPane handle
 * @param[in] mode      repetiton mode
 */
void QBRecordingEditPaneSetRepeatMode(QBRecordingEditPane self, QBPVRRepeatedRecordingsMode mode);

/**
 * @}
**/

#endif /* QBRECORDINGEDITPANE_H_ */
