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

#ifndef QB_SEARCH_PROGRESS_LISTENER_H_
#define QB_SEARCH_PROGRESS_LISTENER_H_

/**
 * @file QBSearchProgressListener.h Abstract search progress listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBSearchProgressListener QBSearchProgressListener: search progress listener
 * @ingroup QBSearch
 **/

/**
  * Search progress percent special values
  **/
typedef enum {
    QBSearchProgress_noProgress = 0,  ///< no progress
    QBSearchProgress_finished = 100,  ///< progress finished
} QBSearchProgress;

/**
 * QBSearchProgressListener interface.
 *
 * QBSearchProgressListener is interface for listening search progress
 **/
typedef const struct QBSearchProgressListener_ {
    /**
     * Progress updated.
     *
     * @param[in] self_         listener handle
     * @param[in] progress      current progress
     **/
    void (*updated)(SvObject self_, int progress);
} *QBSearchProgressListener;

/**
 * Get runtime type identification object representing QBSearchProgressListener interface.
 *
 * @return QBSearchProgressListener interface object
 **/
SvInterface
QBSearchProgressListener_getInterface(void);

/**
 * @}
 **/

#endif /* QB_SEARCH_PROGRESS_LISTENER_H_ */
