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

#ifndef QB_DIALOG_H_
#define QB_DIALOG_H_

#include <stdbool.h>
#include <SvFoundation/SvString.h>
#include <CAGE/Core/SvBitmap.h>
#include <CUIT/Core/types.h>
#include <QBApplicationController.h>
#include <QBApplicationControllerTypes.h>

/**
 * @file QBDialog.h API for dialog window
 **/

typedef void (*QBDialogCallback)(void *ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode);
typedef bool (*QBDialogInputEventHandlerCallback)(SvWidget ptr, SvInputEvent ev);

typedef bool (*QBDialogNotificationCallback)(void * ptr, SvWidget dialog, SvString buttonTag, unsigned keyCode);

/**
 * QBDialogParameters is used to create QBDialog widget
 **/
typedef struct QBDialogParameters_ {
    SvApplication app;                  //!< CUIT application handle
    QBApplicationController controller; //!< application controller handle
    const char *widgetName;             //!< widget name defined in settings
    SvWidgetId ownerId;                 //!< id
} QBDialogParameters;

/**
 * QBDialogLocalType defines dialog type.
 **/
typedef enum QBDialogLocalType_e {
    QBDialogLocalTypeFocusable = 0, //!< dialog can be focused
    QBDialogLocalTypeNonFocusable,  //!< dialog can't be focused
    QBDialogLocalTypeOnTop          //!< dialog is always on top
} QBDialogLocalType;

/**
 * QBDialogInputPolicy defines which keys that are not consumed by the dialog itself are passed on or blocked.
 **/
typedef enum QBDialogInputPolicy_e {
    QBDialogInputPolicy_nonModal = 0,       //!< do not block any keys
    QBDialogInputPolicy_lenientModal,       //!< block only arrow keys
    QBDialogInputPolicy_strictModal,        //!< block all besides some functional, like power and volume
} QBDialogInputPolicy;

/**
 * QBDialogGlobalNew creates dialog that is attached globally.
 * Parameter 'name' must be unique at the moment
 * of using QBDialogShow() or QBDialogRun().
 *
 * @param[in] params    parameters handle
 * @param[in] name      dialog indentification name
 * @return              @c dialog widget, if creation ends with success, @c NULL, otherwise
 **/
SvWidget
QBDialogGlobalNew(QBDialogParameters *params,
                  SvString name);

/**
 * QBDialogLocalNew creates dialog that is attached locally.
 * @see QBDialogLocalType to read about 'type' parameter.
 *
 * @param[in] params    parameters handle
 * @param[in] type      local dialog type
 * @return              @c dialog widget, if creation ends with success, @c NULL, otherwise
 **/
SvWidget
QBDialogLocalNew(QBDialogParameters *params,
                 QBDialogLocalType type);

/**
 * QBDialogSetKeysToCatch can set up QBKEYs to consume it.
 *
 * @param[in] dlg      widget handle
 * @param[in] keyCodes array with QBKEY codes; "0" element at the last position is mandatory;
 *                     set @c NULL when don't need to catch keys anymore
 **/
extern void
QBDialogSetKeysToCatch(SvWidget dlg,
                       const unsigned int *keyCodes);

extern void
QBDialogSetGlobalKeys(SvWidget dlg,
                      const unsigned int *keyCodes);

extern void
QBDialogSetTitle(SvWidget dlg,
                 const char *title);

extern void
QBDialogSetIcon(SvWidget dlg,
                SvBitmap icon);

extern void
QBDialogShow(SvWidget dlg,
             bool immediate);

extern void
QBDialogHide(SvWidget dlg,
             bool immediate,
             bool destroy);

extern void
QBDialogSetNotificationCallback(SvWidget dlg,
                                void * callbackData,
                                QBDialogNotificationCallback callback);
extern void
QBDialogRun(SvWidget dlg,
            void *callbackData,
            QBDialogCallback callback);

extern SvWidget
QBDialogAddButton(SvWidget dlg,
                  SvString tag,
                  const char *caption,
                  unsigned int position);

extern SvWidget
QBDialogAddPanel(SvWidget dlg,
                 SvString tag,
                 const char *caption,
                 unsigned int position);

extern SvWidget
QBDialogAddLabel(SvWidget dlg,
                 SvString panelTag,
                 const char *text,
                 SvString tag,
                 unsigned int position);

extern SvWidget
QBDialogAddWidget(SvWidget dlg,
                  SvString panelTag,
                  SvWidget wgt,
                  SvString tag,
                  unsigned int position,
                  const char *widgetName);

extern SvWidget
QBDialogGetWidget(SvWidget dlg,
                  SvString tag);

extern void
QBDialogRemoveItem(SvWidget dlg,
                   SvString tag);

extern void
QBDialogBreak(SvWidget dlg);

extern void
QBDialogAlignButtons(SvWidget dlg);

extern void
QBDialogRealign(SvWidget dlg);

/**
 * Create dialog with animated icon.
 * @param[in] app           CUIT application handle
 * @param appController     application controller handle
 * @return                  dialog widget with animated icon
 */
SvWidget
QBAnimationDialogCreate(SvApplication app,
                        QBApplicationController appController);

extern void QBDialogSetInputEventHandlerCallback(SvWidget dlg,
                                                 QBDialogInputEventHandlerCallback callback);

/*
 * Hack
 */
void QBDialogSetExitValue(SvWidget dlg,
                                  SvString exitValue,
                                  unsigned int exitKeyCode);


#endif
