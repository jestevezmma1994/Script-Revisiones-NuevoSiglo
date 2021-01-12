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

#ifndef QBBOOKMARKMANAGERPLUGIN_H_
#define QBBOOKMARKMANAGERPLUGIN_H_

#include <QBBookmarkService/QBBookmark.h>
#include <QBBookmarkService/QBBookmarkManager.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvInterface.h>
#include <SvCore/SvErrorInfo.h>
#include <fibers/c/fibers.h>

/**
 * Get runtime type identification object representing
 * QBBookmarkManagerPlugin interface.
 **/
extern SvInterface
QBBookmarkManagerPlugin_getInterface(void);

/**
 * QBBookmarkManagerPlugin interface.
 **/
typedef struct QBBookmarkManagerPlugin_t *QBBookmarkManagerPlugin;
struct QBBookmarkManagerPlugin_t {

   /**
    * Start plugin tasks.
    *
    * @param[in] self_      plugin handle
    * @param[in] scheduler  handle to the scheduler that should be used
    *                       by the plugin if needed
    * @return               zero on success, @c negative value on error
    **/
   int (*start)(SvObject self_,
                SvScheduler scheduler);

   /**
    * Stop plugin tasks.
    *
    * @param[in] self_     plugin handle
    * @return              zero on success, @c negative value on error
    **/
   int (*stop)(SvObject self_);

   /**
    * Setup plugin for work with bookmark manager.
    *
    * @param[in]  self_         plugin handles
    * @param[in]  storage       bookmark storage handle
    * @param[in]  handledTypes  array of bookmark types handled by this plugin
    * @param[in]  typesCnt      size of the handledTypes array
    * @return                   zero on success, @c negative value on error
    **/
   int (*setup)(SvObject self_,
                SvHashTable storage,
                QBBookmarkType *handledTypes,
                size_t typesCnt);

   /**
    * Perform action for adding bookmark
    *
    * @param[in] self_      plugin handle
    * @param[in] bookmark   bookmark
    * @return               zero on success, @c negative value on error
    **/
   int (*onAddBookmark)(SvObject self_,
                        QBBookmark bookmark);

   /**
    * Perform action for removing bookmark
    *
    * Plugins may need to perform special actions
    * when bookmarks are removed
    *
    * @param[in] self_      plugin handle
    * @param[in] bookmark   bookmark
    * @return               zero on success, @c negative value on error
    **/
   int (*onRemoveBookmark)(SvObject self_,
                           QBBookmark bookmark);

   /**
    * Perform action on bookmark change
    *
    * @param[in] self_      plugin handle
    * @param[in] bookmark   bookmark
    * @return               zero on success, @c negative value on error
    **/
   int (*onChangeBookmark)(SvObject self_,
                           QBBookmark bookmark);
};

bool
QBBookmarkManagerPluginIsTypeHandled(const QBBookmarkType *handledTypes,
                                     size_t typesCnt,
                                     QBBookmarkType bookmarkType);

#endif /* QBBOOKMARKMANAGERPLUGIN_H_ */
