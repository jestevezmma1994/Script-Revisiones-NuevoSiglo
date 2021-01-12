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

/* App/Libraries/QBDSMCC/Headers/QBDSMCC/QBDSMCCFeedProvider.h */

#ifndef QB_DSMCC_FEED_PROVIDER_H_
#define QB_DSMCC_FEED_PROVIDER_H_

/**
 * @file QBDSMCCFeedProvider.h
 * @brief DSM-CC Feed Provider API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDSMCCClient.h>
#include <QBContentManager/QBContentTree.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup QBDSMCCFeedProvider DSM-CC Feed Provider
 * @ingroup QBDSMCC
 * @{
 **/

/**
 * QBDSMCCFeedProvider class.
 *
 * Feed provider creates two level tree:
 * - first level contains a QBContentCategory (and associated Feed provider
 *   slave) for every newsFeed and imageFeed in order as in JSON description.
 * - each slave creates an array of SvDBRawObjects. Each has the following content:
 *
 *       {
 *           "presentation_scheme": "TEXTIMAGE",
 *           "title": "...",
 *           "thumbnail": "SvRBObject URI", // it may be missing if there was any error
 *           "description": "..." // only in newsFeed
 *       }
 *
 * @class QBDSMCCFeedProvider QBDSMCCFeedProvider.h <QBDSMCC/QBDSMCCFeedProvider.h>
 * @extends QBContentProvider
 * @implements QBDSMCClientListener
 * @implements QBDSMCCDataParserListener
 **/
typedef struct QBDSMCCFeedProvider_s *QBDSMCCFeedProvider;

/**
 * Create QBDSMCCFeedProvider
 *
 * @param[in] dsmccClient   QBDSMCCClient handle
 * @param[in] contentTree   handle to a content tree to put content in
 * @return                  QBDSMCCFeedProvider handle or NULL on error
 **/
QBDSMCCFeedProvider QBDSMCCFeedProviderCreate(QBDSMCCClient dsmccClient, QBContentTree contentTree);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* QB_DSMCC_FEED_PROVIDER_H_ */
