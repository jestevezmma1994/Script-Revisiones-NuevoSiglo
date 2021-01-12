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

#ifndef QB_LIST_MODEL_H_
#define QB_LIST_MODEL_H_

/**
 * @file QBListModel.h List data model interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBListModel List data model interface
 * @ingroup QBDataModel3
 * @{
 *
 * List data model is an abstract interface for any source of data that
 * can be thought of as dynamically changing list (or array) of objects.
 *
 * @image html QBListModel.png
 **/

/**
 * List data model interface.
 **/
typedef const struct QBListModel_ {
    /**
     * Get number of objects in the data source.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModel
     * @return              number of objects
     **/
    size_t (*getLength)(SvObject self_);

    /**
     * Get single objects from data source.
     *
     * @param[in] self_     handle to an object implementing @ref QBListModel
     * @param[in] idx       index of the requested object
     * @return              handle to an object at index @a idx,
     *                      @c NULL if not available
     **/
    SvObject (*getObject)(SvObject self_, size_t idx);
} *QBListModel;
#define QBListModel_t QBListModel_


/**
 * Get runtime type identification object representing
 * QBListModel interface.
 *
 * @return QBListModel interface object
 **/
extern SvInterface
QBListModel_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
