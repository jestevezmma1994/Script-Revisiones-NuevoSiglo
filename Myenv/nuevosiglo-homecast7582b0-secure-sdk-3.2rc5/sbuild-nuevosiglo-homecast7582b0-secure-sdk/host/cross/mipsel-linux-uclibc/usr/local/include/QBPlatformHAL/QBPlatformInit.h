/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_PLATFORM_INIT_H_
#define QB_PLATFORM_INIT_H_

/**
 * @file QBPlatformInit.h Platform HAL initialization API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPlatformInit Global HAL layer initialization/deinitialization
 * @ingroup QBPlatformHAL
 * @{
 **/

/**
 * Platform initialization config.
 **/
typedef struct QBPlatformInitConfig_ {
    struct {
        struct {
            bool enabled; /**< indicates if feature is enabled */
        } ipTVReencryption; /**< IP TV Reencryption settings */
    } features; /**< features settings */
} QBPlatformInitConfig;

/**
 * Get default platform configuration.
 *
 * @param[out] config default configuration
 **/
extern void
QBPlatformInitGetDefaultConfig(QBPlatformInitConfig *config);

/**
 * Initialize HAL layer.
 *
 * @param[in] config initialization configuration, if NULL default config will be used.
 *
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformInitWithConfig(const QBPlatformInitConfig *config);

/**
 * Initialize HAL layer using default configuration.
 *
 * This function performs global initialization of the HAL layer.
 *
 * @return              @c 0 on success, @c -1 in case of error
 **/
extern int
QBPlatformInit(void);

/**
 * Close HAL layer.
 **/
extern void
QBPlatformDeinit(void);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
