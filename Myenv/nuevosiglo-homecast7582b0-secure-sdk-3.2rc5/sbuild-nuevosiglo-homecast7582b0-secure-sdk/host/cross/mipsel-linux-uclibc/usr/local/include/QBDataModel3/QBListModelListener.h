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

#ifndef QB_LIST_MODEL_LISTENER_H_
#define QB_LIST_MODEL_LISTENER_H_

/**
 * @file QBListModelListener.h
 * @brief List data model listener interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBListModelListener List data model listener interface
 * @ingroup QBDataModel3
 * @{
 *
 * An interface for objects that want to receive notifications
 * about changes in the list data model.
 **/

/**
 * List data model listener interface.
 **/
typedef const struct QBListModelListener_ {
    /**
     * Notify that new objects appeared in the data source.
     *
     * This method notifies the listener that some number of objects
     * (at least one) appeared in the data source. New objects are
     * available in the range (@a first; @a first + @a count - @a 1).
     * Objects, that previously were in this range, are available
     * at higher indices.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModelListener
     * @param[in] first     index of the first added object
     * @param[in] count     number of added objects
     **/
    void (*itemsAdded)(SvObject self_,
                       size_t first, size_t count);

    /**
     * Notify that some objects disappeared from the data source.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModelListener
     * @param[in] first     index of the first removed object
     * @param[in] count     number of removed objects
     **/
    void (*itemsRemoved)(SvObject self_,
                         size_t first, size_t count);

    /**
     * Notify that some objects changed.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModelListener
     * @param[in] first     index of the first modified object
     * @param[in] count     number of modified objects
     **/
    void (*itemsChanged)(SvObject self_,
                         size_t first, size_t count);

    /**
     * Notify that order of objects in some range have changed.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModelListener
     * @param[in] first     index of the first object in the reordered range
     * @param[in] count     number of objects in the reordered range
     **/
    void (*itemsReordered)(SvObject self_,
                           size_t first, size_t count);
} *QBListModelListener;
#define QBListModelListener_t QBListModelListener_

/**
 * Get runtime type identification object representing
 * QBListModelListener interface.
 *
 * @return QBListModelListener interface object
 **/
extern SvInterface
QBListModelListener_getInterface(void);

/**
 * @}
 **/


/**
 * @defgroup QBListModelListener2 List data model listener (version 2) interface
 * @ingroup QBDataModel3
 * @{
 *
 * Extended version of @ref QBListModelListener.
 **/

/**
 * List data model listener (version 2) interface.
 **/
typedef const struct QBListModelListener2_ {
    /**
     * Notify that new objects appeared in the data source.
     *
     * This method notifies the listener that some number of objects
     * (at least one) appeared in the data source. New objects are
     * available in the range (@a first; @a first + @a count - @a 1).
     * Objects, that previously were in this range, are available
     * at higher indices.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModelListener2
     * @param[in] dataSource source of this notification
     * @param[in] first     index of the first added object
     * @param[in] count     number of added objects
     **/
    void (*itemsAdded)(SvObject self_,
                       SvObject dataSource,
                       size_t first, size_t count);

    /**
     * Notify that some objects disappeared from the data source.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModelListener2
     * @param[in] dataSource source of this notification
     * @param[in] first     index of the first removed object
     * @param[in] count     number of removed objects
     **/
    void (*itemsRemoved)(SvObject self_,
                         SvObject dataSource,
                         size_t first, size_t count);

    /**
     * Notify that some objects changed.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModelListener2
     * @param[in] dataSource source of this notification
     * @param[in] first     index of the first modified object
     * @param[in] count     number of modified objects
     **/
    void (*itemsChanged)(SvObject self_,
                         SvObject dataSource,
                         size_t first, size_t count);

    /**
     * Notify that order of objects in some range have changed.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModelListener2
     * @param[in] dataSource source of this notification
     * @param[in] first     index of the first object in the reordered range
     * @param[in] count     number of objects in the reordered range
     **/
    void (*itemsReordered)(SvObject self_,
                           SvObject dataSource,
                           size_t first, size_t count);

    /**
     * Notify that objects have been replaced.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModelListener2
     * @param[in] dataSource source of this notification
     * @param[in] first     index of the first replaced object
     * @param[in] count     count of the replaced objects
     **/
    void (*itemsReplaced)(SvObject self_,
                          SvObject dataSource,
                          size_t first, size_t count);
} *QBListModelListener2;

/**
 * Get runtime type identification object representing
 * QBListModelListener2 interface.
 *
 * @return QBListModelListener2 interface object
 **/
extern SvInterface
QBListModelListener2_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
