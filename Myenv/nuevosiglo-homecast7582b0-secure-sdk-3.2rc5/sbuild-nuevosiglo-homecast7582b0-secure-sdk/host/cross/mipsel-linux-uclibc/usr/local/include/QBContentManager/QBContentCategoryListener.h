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

#ifndef QB_CONTENT_CATEGORY_LISTENER_H_
#define QB_CONTENT_CATEGORY_LISTENER_H_

/**
 * @file QBContentCategoryListener.h
 * @brief Content category listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <QBContentManager/QBContentCategory.h>


/**
 * @defgroup QBContentCategoryListener Content category listener
 * @ingroup QBContentManagerCore
 * @{
 **/

/**
 * Content category listener.
 **/
typedef const struct QBContentCategoryListener_ {
    /**
     * Notify that state of a category have changed.
     *
     * @param[in] self_     handle to a @ref QBContentCategoryListener
     * @param[in] category  category with recently changed loading state
     * @param[in] previousState previous state of a category
     * @param[in] currentState current state of a category
     **/
    void (*loadingStateChanged)(SvObject self_,
                                QBContentCategory category,
                                QBContentCategoryLoadingState previousState,
                                QBContentCategoryLoadingState currentState);
} *QBContentCategoryListener;


/**
 * Get runtime type identification object representing
 * QBContentCategoryListener interface.
 *
 * @return QBContentCategoryListener interface object
 **/
extern SvInterface
QBContentCategoryListener_getInterface(void);

/**
 * @}
 **/


#endif
