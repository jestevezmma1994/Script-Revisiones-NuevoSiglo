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

#ifndef QB_OBSERVER_H_
#define QB_OBSERVER_H_

/**
 * @file QBObserver.h
 * @brief Object observer interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBObserver Object observer interface
 * @ingroup QBAppKit
 * @{
 *
 * @link QBObserver @endlink is a generic interface for receiving notifications about any kind
 * of changes to an observed object; it is commonly used together with
 * classes that inherit from @ref QBObservable.
 **/

/**
 * Object observer interface.
 **/
typedef const struct QBObserver_ {
    /**
     * Notify that observed object has changed.
     *
     * @param[in] self_     observer handle
     * @param[in] observedObject observed object handle
     * @param[in] arg       optional argument passed from observed object
     **/
    void (*observedObjectUpdated)(SvObject self_,
                                  SvObject observedObject,
                                  SvObject arg);
} *QBObserver;


/**
 * Get runtime type identification object representing
 * QBObserver interface.
 *
 * @return QBObserver interface object
 **/
extern SvInterface
QBObserver_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
