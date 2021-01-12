/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_INITIALIZABLE_H_
#define QB_INITIALIZABLE_H_

/**
 * @file QBInitializable.h
 * @brief Generic object initialization interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBAppKit/QBPropertiesMap.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBInitializable Generic object initialization interface
 * @ingroup QBAppKit
 * @{
 **/

/**
 * Generic object initialization interface.
 **/
typedef const struct QBInitializable_ {
    /**
     * Initialize object.
     *
     * @param[in] self_     handle to an object implementing @ref QBInitializable
     * @param[in] properties set of properties to initialize with
     * @param[out] errorOut error info
     * @return              @a self_ or @c NULL in case of error
     **/
    SvObject (*init)(SvObject self_,
                     QBPropertiesMap properties,
                     SvErrorInfo *errorOut);
} *QBInitializable;

/**
 * Get runtime type identification object representing
 * QBInitializable interface.
 *
 * @return QBInitializable interface object
 **/
extern SvInterface
QBInitializable_getInterface(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
