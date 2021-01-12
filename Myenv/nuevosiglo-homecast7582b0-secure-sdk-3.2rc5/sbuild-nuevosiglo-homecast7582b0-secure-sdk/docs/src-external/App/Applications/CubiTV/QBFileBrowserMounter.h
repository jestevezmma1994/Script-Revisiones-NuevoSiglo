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

#ifndef QB_FILE_BROWSER_MOUNTER_H_
#define QB_FILE_BROWSER_MOUNTER_H_

/**
 * @file QBFileBrowserMounter.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief QBFileBrowser mounter service
 **/

#include <SvCore/SvErrorInfo.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataModel3/QBActiveTree.h>
#include <QBDataModel3/QBTreePathMap.h>
#include <main_decl.h>

/**
 * @defgroup QBFileBrowserMounter QBFileBrowser mounter service
 * @ingroup CubiTV_services
 * @{
 **/

/**
 * QBFileBrowserMounterHandler interface.
 *
 * QBFileBrowserMounterHandler is interface for objects handling file browsing menu registering.
 **/
typedef const struct QBFileBrowserMounterHandler_ {
    /**
     * Register new file browsing menu in menu.
     *
     * @param[in] handler    handler handle
     * @param[in] menu       menu handle
     * @param[in] menuTree   menu data source handle
     * @param[in] pathMap    global path map
     * @param[in] path       path where to register the menu
     * @param[in] fsPath     file system path of an external device to be registered
     **/
    void (*menuRegister)(SvObject handler, SvWidget menu, QBActiveTree menuTree,
                         QBTreePathMap pathMap, SvObject path,
                         SvString fsPath);

    /**
     * Unregister file browsing menu from a menu.
     *
     * @param[in] handler    handler handle
     * @param[in] menu       menu handle
     * @param[in] menuTree   menu data source handle
     * @param[in] pathMap    global path map
     * @param[in] path       path for which the menu is unregistered
     **/
    void (*menuUnregister)(SvObject handler, SvWidget menu, QBActiveTree menuTree,
                           QBTreePathMap pathMap, SvObject path);
} *QBFileBrowserMounterHandler;

/**
 * Get runtime type identification object representing
 * QBFileBrowserMounterHandler interface.
 *
 * @return QBFileBrowserMounterHandler interface object
**/
SvInterface
QBFileBrowserMounterHandler_getInterface(void);

/**
 * QBFileBrowserMounter service class.
 *
 * @class QBFileBrowserMounter QBFileBrowserMounter.h <Services/QBFileBrowserMounter.h>
 * @extends SvObject
 * @implements QBHotplugMountAgentListener
 **/
typedef struct QBFileBrowserMounter_t* QBFileBrowserMounter;

/**
 * Create QBFileBrowserMounter service.
 *
 * @public @memberof QBFileBrowserMounter
 *
 * @return    New QBFileBrowser mounter service, @c NULL in case of error
 **/
QBFileBrowserMounter QBFileBrowserMounterCreate(void);

/**
 * Start QBFileBrowserMounter service.
 *
 * @public @memberof QBFileBrowserMounter
 *
 * @param[in] self          QBFileBrowserMounter service handle
 * @param[in] appGlobals    application globals
 **/
void QBFileBrowserMounterStart(QBFileBrowserMounter self, AppGlobals appGlobals);

/**
 * Stop QBFileBrowserMounter service.
 *
 * @public @memberof QBFileBrowserMounter
 *
 * @param[in] self          QBFileBrowserMounter service handle
 **/
void QBFileBrowserMounterStop(QBFileBrowserMounter self);

/**
 * Register file browser mounter handler.
 *
 * @public @memberof QBFileBrowserMounter
 *
 * @param[in] self          QBFileBrowserMounter service handle
 * @param[in] handler       handler handle
 * @param[out] *errorOut    error info
 **/
void QBFileBrowserMounterRegisterHandler(QBFileBrowserMounter self, SvObject handler, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_FILE_BROWSER_MOUNTER_H_ */
