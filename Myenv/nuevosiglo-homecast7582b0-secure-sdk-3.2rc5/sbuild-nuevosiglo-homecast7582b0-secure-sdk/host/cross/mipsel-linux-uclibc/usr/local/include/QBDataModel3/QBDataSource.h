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

#ifndef QB_DATA_SOURCE_H_
#define QB_DATA_SOURCE_H_

/**
 * @file QBDataSource.h Abstract data source class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvWeakReference.h>
#include <QBDataModel3/QBSafeIterator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBDataSource Abstract data source class
 * @ingroup QBDataModel3
 * @{
 *
 * An abstract class that can serve as a base for other
 * classes that implement some data model.
 **/

/**
 * Element of the data source listeners array.
 * @private
 **/
struct QBDataSourceItem_ {
    /**
     * identifier of the listener
     * @private
     **/
    uintptr_t ID;
    /**
     * weak reference to the listener
     * @private
     **/
    SvWeakReference ref;
};

/**
 * Data source class.
 * @class QBDataSource
 * @extends SvObject
 **/
typedef struct QBDataSource_ {
    /// base class
    struct SvObject_ super_;
    /// handle to an interface that has to be implemented by listeners
    SvInterface listenerIface;
    /// current number of registered listeners
    size_t targetsCnt;
    /// current size of @a targets array
    size_t targetsMax;
    /// array of registered listeners
    struct QBDataSourceItem_ *targets;
} *QBDataSource;
#define QBDataSource_t QBDataSource_


/**
 * Get runtime type identification object representing
 * type of abstract data source class.
 *
 * @return abstract data source class
 **/
extern SvType
QBDataSource_getType(void);

/**
 * Initialize data source object.
 *
 * @memberof QBDataSource
 *
 * @param[in] self      data source handle
 * @param[in] listenerIface handle to the interface that must be implemented
 *                      by all registered listeners, @c NULL to accept anybody
 * @param[out] errorOut error info
 * @return              @a self, @c NULL in case of error
 */
extern QBDataSource
QBDataSourceInit(QBDataSource self,
                 SvInterface listenerIface,
                 SvErrorInfo *errorOut);

/**
 * Get an iterator over all registered listeners.
 *
 * @deprecated This method is not thread-safe and have been deprecated.
 * Rewrite your code to use QBDataSourceGetSafeIterator().
 *
 * @memberof QBDataSource
 *
 * @param[in] self      data source handle
 * @return              iterator over data source listeners
 **/
extern SvIterator
QBDataSourceIterator(QBDataSource self);

/**
 * Get a safe iterator over all registered listeners.
 *
 * @memberof QBDataSource
 *
 * @param[in] self      data source handle
 * @return              safe iterator over data source listeners
 **/
extern QBSafeIterator
QBDataSourceGetSafeIterator(QBDataSource self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
