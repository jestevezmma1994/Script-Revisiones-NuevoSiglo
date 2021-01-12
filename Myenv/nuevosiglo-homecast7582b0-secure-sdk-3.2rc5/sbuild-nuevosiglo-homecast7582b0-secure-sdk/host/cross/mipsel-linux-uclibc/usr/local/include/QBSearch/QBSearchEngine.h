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

#ifndef QB_SEARCH_ENGINE_H_
#define QB_SEARCH_ENGINE_H_

/**
 * @file QBSearchEngine.h Search Engine class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvCore/SvErrorInfo.h>

/**
 * Search engine class.
 *
 * @class QBSearchEngine QBSearchEngine.h <QBSearch/QBSearchEngine.h>
 * @extends QBListDataSource
 * @implements QBListModel
 **/
typedef struct QBSearchEngine_ *QBSearchEngine;

/**
 * Create QBSearchEngine.
 *
 * @memberof QBSearchEngine
 *
 * This is a method that allocates and initializes QBSearchEngine.
 *
 * @param[in] agents            agents array
 * @param[in] keywords          keywords to be searched
 * @param[out] *errorOut        error info
 * @return                      created search engine, @c NULL in case of error
 **/
SvObject
QBSearchEngineCreate(SvArray agents, SvString keywords, SvErrorInfo *errorOut);

/**
 * Get keywords array.
 *
 * @memberof QBSearchEngine
 *
 * @param[in] self              search engine handle
 * @return                      keywords array
 **/
SvArray
QBSearchEngineGetKeywords(SvObject self);

/**
 * Get keywords string.
 *
 * @memberof QBSearchEngine
 *
 * @param[in] self              search engine handle
 * @return                      keywords string
 **/
SvString
QBSearchEngineGetKeywordsString(SvObject self);

/**
 * Add object implementing QBSearchAgent interface to search engine.
 *
 * @memberof QBSearchEngine
 *
 * @param[in] self              search engine handle
 * @param[in] agent             agent to be added
 **/
void
QBSearchEngineAddAgent(SvObject self, SvObject agent);

/**
 * Set progress listener of search engine.
 *
 * @memberof QBSearchEngine
 *
 * @param[in] self              search engine handle
 * @param[in] listener          progress listener
 **/
void
QBSearchEngineSetProgressListener(SvObject self, SvObject listener);

/**
 * Start search engine.
 *
 * @memberof QBSearchEngine
 *
 * @param[in] self              search engine handle
 **/
void
QBSearchEngineSearchStart(SvObject self);

/**
 * Stop search engine.
 *
 * @memberof QBSearchEngine
 *
 * @param[in] self              search engine handle
 **/
void
QBSearchEngineSearchStop(SvObject self);

#endif /* QB_SEARCH_ENGINE_H_ */
