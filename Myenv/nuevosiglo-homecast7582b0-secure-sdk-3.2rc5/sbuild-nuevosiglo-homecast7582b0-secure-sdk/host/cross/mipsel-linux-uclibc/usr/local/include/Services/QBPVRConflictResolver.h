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

#ifndef QBPVRCONFLICTRESOLVER_H_
#define QBPVRCONFLICTRESOLVER_H_


#include <ContextMenus/QBContextMenu.h>
#include <QBPVRTypes.h>
#include <SvFoundation/SvCoreTypes.h>
#include <main_decl.h>

typedef struct QBPVRConflictResolver_ *QBPVRConflictResolver;

typedef void (*QBPVRConflictResolverFinishCallback)(void *ptr, bool saved);
void QBPVRConflictResolverSetFinishCallback(QBPVRConflictResolver self, QBPVRConflictResolverFinishCallback callback, void *ptr);
typedef void (*QBPVRConflictResolverQuestionCallback)(void *ptr);
void QBPVRConflictResolverSetQuestionCallback(QBPVRConflictResolver self, QBPVRConflictResolverQuestionCallback callback, void *ptr);


void
QBPVRConflictResolverResolve(QBPVRConflictResolver self);

/**
 * Create PVR conflict resolver
 *
 * @param[in] appGlobals        application globals
 * @param[in] ctxMenu           context menu
 * @param[in] params            recording schedule parameters
 * @param[in] level             level for first conflict resolver pane
 * @return                      created conflict resolver, @c NULL in case of error
 */
QBPVRConflictResolver
QBPVRConflictResolverCreate(AppGlobals appGlobals, QBContextMenu ctxMenu, QBPVRRecordingSchedParams params, int level);

#endif /* QBPVRCONFLICTRESOLVER_H_ */
