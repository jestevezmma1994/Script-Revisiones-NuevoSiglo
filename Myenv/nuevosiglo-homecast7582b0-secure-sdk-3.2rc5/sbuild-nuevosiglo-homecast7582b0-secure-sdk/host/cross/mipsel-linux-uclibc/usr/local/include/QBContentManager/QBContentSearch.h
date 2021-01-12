/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

/* App/Libraries/QBContentManager/QBContentCategory.h */

#ifndef QB_CONTENT_SEARCH_H_
#define QB_CONTENT_SEARCH_H_

#include <SvCore/SvErrorInfo.h>
#include <QBContentManager/QBContentCategory.h>

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>

#include <fibers/c/fibers.h>


typedef struct QBContentSearch_t *QBContentSearch;


extern SvType
QBContentSearch_getType(void);

extern SvString
QBContentSearchGetKeyword(QBContentSearch self);

extern int
QBContentSearchStartNewSearch(QBContentSearch self, SvString keyword, SvScheduler sched);

/**
 * Create new search with a specific category as search root.
 *
 * @param[in] self      search category handle
 * @param[in] keyword   search keyword
 * @param[in] sched     scheduler
 * @param[in] category  root category
 * @return              0 for succeess, error code otherwise
 */
extern int
QBContentSearchStartNewSearchWithCategory(QBContentSearch self, SvString keyword, SvScheduler sched, QBContentCategory category);

/**
 * Get root search category
 *
 * @param[in] self search category handle
 * @return         root category handle
 */
extern QBContentCategory
QBContentSearchGetSearchRootCategory(QBContentSearch self);

bool
QBContentSearchIsActive(QBContentSearch self);

/**
 * Create instance of QBContentSearch class.
 *
 * @param[in] name          name of search
 * @param[in] parent        handle to parent search, NULL if the search should be root
 * @param[in] treeRef       reference to the tree in which the search will be available
 * @param[out] errorOut     error info
 * @return                  created provider, @c NULL in case of error
 **/
QBContentSearch
QBContentSearchCreate(SvString name, QBContentCategory parent, SvWeakReference treeRef, SvErrorInfo *errorOut);


#endif
