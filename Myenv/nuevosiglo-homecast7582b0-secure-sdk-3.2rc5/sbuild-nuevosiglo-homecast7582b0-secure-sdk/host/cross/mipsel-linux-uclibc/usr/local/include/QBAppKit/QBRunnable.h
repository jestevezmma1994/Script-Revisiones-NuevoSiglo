/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_RUNNABLE_H_
#define QB_RUNNABLE_H_

/**
 * @file QBRunnable.h
 * @brief Abstract interface for a runnable operation
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBRunnable Runnable operation
 * @ingroup QBAppKit
 *
 * @{
 **/

/**
 * Runnable operation interface.
 **/
typedef const struct QBRunnable_ {
    /**
     * Perform your work.
     *
     * @param[in] self_     handle to an object implementing @ref QBRunnable
     * @param[in] caller    owner of this operation that called this method
     **/
    void (*run)(SvObject self_,
                SvObject caller);
} *QBRunnable;

/**
 * Get runtime type identification object representing
 * QBRunnable interface.
 *
 * @return QBRunnable interface object
 **/
extern SvInterface
QBRunnable_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
