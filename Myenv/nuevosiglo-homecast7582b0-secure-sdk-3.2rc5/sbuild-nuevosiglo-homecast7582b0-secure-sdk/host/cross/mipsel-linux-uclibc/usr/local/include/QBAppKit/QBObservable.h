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

#ifndef QB_OBSERVABLE_H_
#define QB_OBSERVABLE_H_

/**
 * @file QBObservable.h
 * @brief Observable object class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvWeakList.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBObservable Observable object class
 * @ingroup QBAppKit
 * @{
 *
 * @link QBObservable @endlink represents an observable object; it is usually used as a base
 * class for implementing objects that can be observed by other modules.
 **/

/**
 * Observable object class.
 * @class QBObservable
 * @extends SvObject
 **/
typedef struct QBObservable_ *QBObservable;

/**
 * Observable object class internals.
 **/
struct QBObservable_ {
    /** super class */
    struct SvObject_ super_;
    /**
     * changed flag
     *
     * @note Don't access this attribute directly, use QBObservableHasChanged(),
     * QBObservableSetChanged() and QBObservableClearChanged() methods.
     **/
    bool changed;
    /**
     * container of weak references to observers,
     * can be @c NULL if there are no observers
     *
     * @note Don't access this attribute directly!
     **/
    SvWeakList observers;
};


/**
 * Get runtime type identification object representing QBObservable class.
 *
 * @return observable object class
 **/
extern SvType
QBObservable_getType(void);

/**
 * Create new observable instance.
 *
 * @memberof QBObservable
 *
 * @param[out] errorOut error info
 * @return              created observable, @c NULL in case of error
 **/
extern QBObservable
QBObservableCreate(SvErrorInfo *errorOut);

/**
 * Register an observer for notifications propagated by QBObservableNotifyObservers().
 *
 * @memberof QBObservable
 *
 * @param[in] self      observable object handle
 * @param[in] observer  handle to an object implementing @ref QBObserver
 * @param[out] errorOut error info
 **/
extern void
QBObservableAddObserver(QBObservable self,
                        SvObject observer,
                        SvErrorInfo *errorOut);

/**
 * Unregister one of previously registered observers.
 *
 * @memberof QBObservable
 *
 * @param[in] self      observable object handle
 * @param[in] observer  handle to a previously registered observer
 * @param[out] errorOut error info
 **/
extern void
QBObservableRemoveObserver(QBObservable self,
                           SvObject observer,
                           SvErrorInfo *errorOut);

/**
 * Unregister all observers.
 *
 * @memberof QBObservable
 *
 * @param[in] self      observable object handle
 * @param[out] errorOut error info
 **/
extern void
QBObservableRemoveAllObservers(QBObservable self,
                               SvErrorInfo *errorOut);

/**
 * Get number of observers currently registered in the observable object.
 *
 * @memberof QBObservable
 *
 * @param[in] self      observable object handle
 * @return              number of registered observers or @c -1 in case of error
 **/
extern ssize_t
QBObservableGetObserversCount(QBObservable self);

/**
 * Check if observable object is marked as changed.
 *
 * @memberof QBObservable
 *
 * @param[in] self      observable object handle
 * @return              @c true if observable object is marked as changed, otherwise @c false
 **/
static inline bool
QBObservableHasChanged(QBObservable self)
{
    return self->changed;
}

/**
 * Set QBObservable_::changed attribute marking observable object as changed.
 *
 * @memberof QBObservable
 *
 * @param[in] self      observable object handle
 * @param[out] errorOut error info
 **/
extern void
QBObservableSetChanged(QBObservable self,
                       SvErrorInfo *errorOut);

/**
 * Clear QBObservable_::changed attribute marking observable object as unchanged.
 *
 * @memberof QBObservable
 *
 * @param[in] self      observable object handle
 * @param[out] errorOut error info
 **/
extern void
QBObservableClearChanged(QBObservable self,
                         SvErrorInfo *errorOut);

/**
 * Notify all registered observers if observable object has changed.
 *
 * If this (observable) object has changed, this method notifies all registered
 * observers and clears QBObservable_::changed flag.
 *
 * The order in which observers will be notified is unspecified. Observers must
 * be implemented in a way that guarantees proper behaviour in any case.
 *
 * The list of registered observers that are notified is frozen during the
 * execution of this method: if any observers are added or removed from within
 * one of notified observers, it will affect the list observers used next time
 * this method is called.
 *
 * @memberof QBObservable
 *
 * @param[in] self      observable object handle
 * @param[in] arg       optional argument to be passed to observers
 * @param[out] errorOut error info
 **/
extern void
QBObservableNotifyObservers(QBObservable self,
                            SvObject arg,
                            SvErrorInfo *errorOut);

/**
 * Set QBObservable_::changed attribute marking observable object as changed and
 * notify all registered observers about the change.
 *
 * This method acts exactly as calling @ref QBObservableSetChanged and
 * @ref QBObservableNotifyObservers.
 *
 * @memberof QBObservable
 *
 * @param[in] self      observable object handle
 * @param[in] arg       optional argument to be passed to observers
 * @param[out] errorOut error info
 **/
extern void
QBObservableSetChangedAndNotify(QBObservable self,
                                SvObject arg,
                                SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
