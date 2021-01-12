/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBXHTML_WINDOWS_CREATOR_H
#define QBXHTML_WINDOWS_CREATOR_H

/**
 * @file QBXHTMLWindowsCreator.h XHTML windows asynchronous creator
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBXHTMLWidgets/QBXHTMLWindowsClient.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvString.h>
#include <QBDataModel3/QBActiveTree.h>
#include <main_decl.h>

/**
 * @defgroup QBXHTMLWindowsCreator XHTML windows creator
 * @ingroup GUI
 * @{
 *
 * Class for creating XHTML windows
 **/

/**
 * QBXHTMLWindow Class containing all the data needed to display and use XHTML window
 *
 * @class QBXHTMLWindow
 * @extends SvObject
 **/
typedef struct QBXHTMLWindow_ *QBXHTMLWindow;

/**
 * Set the id of the window
 *
 * @param [in] self     QBXHTMLWindow handle
 * @param [in] id       new id of XHTML window
 **/
void
QBXHTMLWindowSetId(QBXHTMLWindow self, SvString id);

/**
 * Get the title of the window
 *
 * @param [in] self     QBXHTMLWindow handle
 * @return              title of XHTML window or @c NULL if not present
 **/
SvString
QBXHTMLWindowGetTitle(QBXHTMLWindow self);

/**
 * Get the id of the window
 *
 * @param [in] self     QBXHTMLWindow handle
 * @return              id of XHTML window or @c NULL if not present
 **/
SvString
QBXHTMLWindowGetId(QBXHTMLWindow self);

/**
 * Get refreshTime (in seconds) associated with the window.
 * Refresh time defines interval after which xhtml data should be downloaded again
 * and xhtmlWindow updated.
 *
 * @param [in] self     QBXHTMLWindow handle
 * @return              time after which window shoud be updated
 **/
int
QBXHTMLWindowGetRefreshTime(QBXHTMLWindow self);

/**
 * Get the widget representing the XHTML document
 *
 * @param [in] self     QBXHTMLWindow handle
 * @return              widget that needs to be displayed as a result of XHTML window construction
 **/
SvWidget
QBXHTMLWindowGetWidget(QBXHTMLWindow self);

/**
 * Get the XHTML document tree.
 * The tree returned from this function can be used to perform operations on XHTML window
 *
 * @param [in] self     QBXHTMLWindow handle
 * @return              tree representing XHTML document.
 **/
QBActiveTree
QBXHTMLWindowGetXHTMLTree(QBXHTMLWindow self);

/**
 * Get the widgets information hash table.
 * The hashtable contains @c QBXHTMLWidgetInfo objects with @c QBXHTMLVisitableNode nodes as keys.
 * To make use of this data you should create a visitor that will visit the document tree in some way
 * and obtain @c QBXHTMLWidgetInfo from this table using visited node as a key.
 *
 * @param [in] self     QBXHTMLWindow handle
 * @return              hash table with widgets info.
 **/
SvHashTable
QBXHTMLWindowGetWidgetsInfo(QBXHTMLWindow self);

/**
 * Get the XHTML document used to create a window.
 *
 * @param [in] self     QBXHTMLWindow handle
 * @return              XHTML document as string.
 **/
SvString
QBXHTMLWindowGetXHTMLDocument(QBXHTMLWindow self);

/**
 * QBXHTMLWindowsCreator Class responsible for asynchronous XHTML widget creation.
 *
 * @class QBXHTMLWindowsCreator
 * @extends SvObject
 **/
typedef struct QBXHTMLWindowsCreator_* QBXHTMLWindowsCreator;

/**
* Creates QBXHTMLWindowsCreator
*
* @param [in] appGlobals            application data handle
* @param [in] xhtmlWindowsClient    XHTMLWindows client handle, or @c NULL if windows creator will not be used
*                                   to download widgets using http. You can use @c QBXHTMLWindowsCreatorCreateWindowFromXHTML only
*                                   if you do not pprovide xhtmlWindowsClient.
* @return                           newly created windows creator
**/
QBXHTMLWindowsCreator
QBXHTMLWindowsCreatorCreate(AppGlobals appGlobals, QBXHTMLWindowsClient xhtmlWindowsClient);

/**
* Possible widget creation outcomes
**/
typedef enum {
    QBXHTMLWindowsCreatorErrorCode_ok,                  //!< window creation finished
    QBXHTMLWindowsCreatorErrorCode_dataNotAvailable,    //!< could not download XHTML document
    QBXHTMLWindowsCreatorErrorCode_parsingError,        //!< error while parsing XHTML document
    QBXHTMLWindowsCreatorErrorCode_transferCanceled     //!< XHTML document transfer canceled
} QBXHTMLWindowsCreatorErrorCode;

/**
* Callback called when window construction have been finished, or has failed
*
* @param [in] prv           callback private data passed to QBXHTMLWindowsCreator upon request
* @param [in] window        created window data or @c NULL in case of error
* @param [in] errorCode     error information
**/
typedef void (*QBXHTMLWindowsCreatorCallback)(
    void *prv,
    QBXHTMLWindow window,
    QBXHTMLWindowsCreatorErrorCode errorCode
    );

/**
 * Creates a widget from an URL. This is an asynchronous function.
 * After the operation is finished, a callback will be called.
 * Steps performed by the function:
 * (1) sends a SvHTTPRequest (GET) to the url
 * (2) parses received XHTML data with the expat XML parser and creates the SvWidget
 * (3) calls callback to provide created SvWidget (NULL in case of an error + errorCode)
 *
 * @param [in] self         QBXHTMLWindowsCreator handle
 * @param [in] url          url to the XHTML document
 * @param [in] id           id of the requested window
 * @param [in] callback     callback to be called when the widget is ready or request has failed
 * @param [in] target       callback private data
 **/
void QBXHTMLWindowsCreatorCreateWindowFromURL(
    QBXHTMLWindowsCreator self,
    SvURL url,
    SvString id,
    QBXHTMLWindowsCreatorCallback callback,
    void *target
    );

/**
 * Creates a widget from XHTML. This is an asynchronous function.
 * After the operation is finished, a callback will be called.
 * Steps performed by the function:
 * (1) parses received XHTML data with the expat XML parser and creates the SvWidget
 * (2) downloads any images if needed
 * (3) calls callback to provide created widget (NULL in case of an error + errorCode)
 *
 * @param [in] self         QBXHTMLWindowsCreator handle
 * @param [in] xhtml        xhtml describing widget
 * @param [in] callback     callback to be called when the widget is ready or request has failed
 * @param [in] target       callback private data
 **/
void QBXHTMLWindowsCreatorCreateWindowFromXHTML(
    QBXHTMLWindowsCreator self,
    SvString xhtml,
    QBXHTMLWindowsCreatorCallback callback,
    void *target
    );

/**
 * @}
 **/

#endif //QBXHTML_WINDOWS_CREATOR_H
