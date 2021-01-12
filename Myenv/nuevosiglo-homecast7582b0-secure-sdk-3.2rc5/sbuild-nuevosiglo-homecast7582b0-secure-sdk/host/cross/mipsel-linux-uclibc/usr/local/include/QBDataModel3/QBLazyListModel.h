/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_LAZY_LIST_MODEL_H_
#define QB_LAZY_LIST_MODEL_H_

/**
 * @file QBLazyListModel.h Lazy list data model interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBDataModel3/QBListModel.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBLazyListModel Lazy list data model interface
 * @ingroup QBDataModel3
 * @{
 *
 * Lazy list data model is a specialization of @ref QBListModel adding
 * operation for lazy retrieving objects.
 *
 * @image html QBLazyListModel.png
 **/

/**
 * Lazy list data model interface.
 **/
typedef const struct QBLazyListModel_ {
    /**
     * Super interface.
     **/
    struct QBListModel_ super_;

    /**
     * Lazy get single objects from data source.
     *
     * This method returns single object from data source if it is
     * available locally. Unlike QBListModel::getObject() it does
     * not trigger data fetch when object is not available.
     *
     * @param[in] self_     handle to an object implementing @ref QBLazyListModel
     * @param[in] idx       index of the requested object
     * @return              handle to an object at index @a idx,
     *                      @c NULL if not available
     **/
    SvObject (*tryGetObject)(SvObject self_,
                             size_t idx);

    /**
     * Schedule prefetch of objects for future use.
     *
     * @param[in] self_     handle to an object implementing @ref QBLazyListModel
     * @param[in] idx       index of the first object to be prefetched
     * @param[in] count     number of objects to prefetch
     **/
    void (*prefetch)(SvObject self_,
                     size_t idx,
                     size_t count);
} *QBLazyListModel;
#define QBLazyListModel_t QBLazyListModel_


/**
 * Get runtime type identification object representing
 * QBLazyListModel interface.
 *
 * @return QBLazyListModel interface object
 **/
extern SvInterface
QBLazyListModel_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
