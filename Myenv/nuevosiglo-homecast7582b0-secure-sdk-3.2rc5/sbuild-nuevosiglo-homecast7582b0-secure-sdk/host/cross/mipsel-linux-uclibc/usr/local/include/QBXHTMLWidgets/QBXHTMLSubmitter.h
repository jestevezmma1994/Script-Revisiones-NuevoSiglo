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

#ifndef QB_XHTML_SUBMITTER_H
#define QB_XHTML_SUBMITTER_H

#include <QBXHTMLWidgets/QBXHTMLNodes.h>
#include <QBXHTMLWidgets/QBXHTMLWindowsClient.h>
#include <SvFoundation/SvHashTable.h>
#include <SvCore/SvErrorInfo.h>
#include <stdbool.h>

/**
 * @file QBXHTMLSubmitter.h XHTML tree submitter
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/


/**
 * QBXHTMLSubmitter collects and sends data to url from form.
 *
 * @class QBXHTMLSubmitter
 * @extends SvObject
 **/
typedef struct QBXHTMLSubmitter_ *QBXHTMLSubmitter;

/**
 * Function that will handle response received after data submission.
 *
 * @param [in] self         handle to object which has invoked function QBXHTMLSubmitterSubmit
 * @param [in] response     response from server
 * @param [in] state        state of a request created in order to submit data
 **/
typedef void (*QBXHTMLSubmitterCallback)(SvObject self, SvHashTable response, QBXHTMLWindowsClientState state);

/**
 * Create submitter.
 *
 * @param [in] client       QBXHTMLWindowsClient handle
 * @param [out] errorOut    error output
 * @return                  created subbmiter or @c NULL in case of error
 **/
QBXHTMLSubmitter
QBXHTMLSubmitterCreate(QBXHTMLWindowsClient client, SvErrorInfo *errorOut);

/**
 * Submitter visits ancestors of buttonNode and remember first founded form.
 * In second step function collects data form form subtree and sends them to form url.
 *
 * @param [in] self         QBXHTMLSubmitter handle
 * @param [in] widgetsInfo  map: nodes -> widgets
 * @param [in] id           id of window from which buttonNode come from
 * @param [in] buttonNode   node related to 'submit' button
 * @param [in] callback     callback function that will handle the response
 * @param [in] callbackArg  object passed back to callback function
 * @return                  true when submiterr starts correctly or false when error occurred
 **/
bool
QBXHTMLSubmitterSubmit(QBXHTMLSubmitter self, SvHashTable widgetsInfo, SvString id, QBXHTMLVisitableNode buttonNode, QBXHTMLSubmitterCallback callback, SvObject callbackArg);

#endif //QB_XHTML_SUBMITTER_H
