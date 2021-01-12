/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PLAYREADY_MANAGER_H_
#define QB_PLAYREADY_MANAGER_H_

/**
 * @file QBPlayReadyManager.h
 * @brief Manages different MS play ready implementations.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBPlayReady QBPlayReadyManager
 * @ingroup DRM
 * @{
 */

#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * QBPlayReadyInterface implementation types
 */
enum QBPlayReadyImplementation {
    QBPlayReadyImplementation_Conax = 0     /**< Conax implementation of playready */
};

/**
 * Initialize internal data structure.
 *
 * @param[out] errorOut error info
 */
void QBPlayReadyManagerInit(SvErrorInfo* errorOut);

/**
 * Add new implemntation of playready with given type. If type is already present
 * it will be replaced.
 *
 * @param[in] type implementation type of QBPlayReadyInterface
 * @param[in] impl implementation object
 * @param[out] errorOut error info
 */
void QBPlayReadyManagerAddImplementation(enum QBPlayReadyImplementation type, SvObject impl, SvErrorInfo* errorOut);

/**
 * Get object that implements QBPlayReadyInterface with given type.
 *
 * @param[in] type type of implementation
 * @param[out] errorOut error info
 * @return object or NULL in case of error
 */
SvObject QBPlayReadyManagerGetImplementation(enum QBPlayReadyImplementation type, SvErrorInfo* errorOut);

/**
 * Destroy internal data structure.
 */
void QBPlayReadyManagerDeinit(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 **/

#endif // QB_PLAYREADY_MANAGER_H_
