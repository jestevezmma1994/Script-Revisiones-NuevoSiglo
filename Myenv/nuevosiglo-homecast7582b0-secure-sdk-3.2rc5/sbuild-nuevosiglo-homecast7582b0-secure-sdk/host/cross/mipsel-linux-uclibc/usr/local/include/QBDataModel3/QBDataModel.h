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

#ifndef QB_DATA_MODEL_H_
#define QB_DATA_MODEL_H_

/**
 * @file QBDataModel.h Abstract data model interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBDataModel Abstract data model interface
 * @ingroup QBDataModel3
 * @{
 *
 * An abstract base interface for all data models.
 **/

/**
 * Abstract data model interface.
 **/
typedef const struct QBDataModel_ {
    /**
     * Add new listener.
     *
     * @param[in] self          handle to an object implementing @ref QBDataModel
     * @param[in] listener      listener handle
     * @param[out] errorOut     error info
     **/
    void (*addListener)(SvObject self_,
                        SvObject listener,
                        SvErrorInfo *errorOut);

    /**
     * Remove previously added listener.
     *
     * @param[in] self          handle to an object implementing @ref QBDataModel
     * @param[in] listener      listener handle
     * @param[out] errorOut     error info
     **/
    void (*removeListener)(SvObject self_,
                           SvObject listener,
                           SvErrorInfo *errorOut);
} *QBDataModel;
#define QBDataModel_t QBDataModel_


/**
 * Get runtime type identification object representing
 * QBDataModel interface.
 *
 * @return QBDataModel interface object
 **/
extern SvInterface
QBDataModel_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
