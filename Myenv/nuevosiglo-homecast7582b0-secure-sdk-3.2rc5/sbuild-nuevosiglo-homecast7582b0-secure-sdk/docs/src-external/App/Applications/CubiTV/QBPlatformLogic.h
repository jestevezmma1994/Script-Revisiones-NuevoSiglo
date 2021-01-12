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

#ifndef QB_PLATFORM_LOGIC_H
#define QB_PLATFORM_LOGIC_H

/**
 * @file QBPlatformLogic.h Platform dependent logic
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <stddef.h>

/**
 * @defgroup QBPlatformLogic Platform dependent logic
 * @ingroup CubiTV_logic
 * @{
 *
 * Platform dependent CubiTV logic
 */

/**
 * Get EPG text renderer cache size.
 *
 * This method returns maximal DSP cache size that text renderer allocate
 * for EPG rendering.
 *
 * @return EPG text renderer cache size in bytes
 */
size_t QBPlatformLogicGetEPGTextRendererCacheSize(void);

/**
 * Get text renderer cache size.
 *
 * This method returns maximal DSP cache size that text renderer allocate
 * for storing texts not related to EPG rendering.
 *
 * @return text renderer cache size in bytes
 */
size_t QBPlatformLogicGetTextRendererCacheSize(void);

/**
 * Get text renderer persistent cache size.
 *
 * This method returns maximal DSP cache size that text renderer allocate
 * for persistent texts.
 *
 * @return EPG text renderer cache size in bytes
 */
size_t QBPlatformLogicGetTextRendererPersistentCacheSize(void);

/**
 * Get cache margin size.
 *
 * This method returns size of cache margin used to prevent memory fragmentation.
 *
 * @return cache margin size in bytes
 */
size_t QBPlatformLogicGetCacheMarginSize(void);

/**
 * Setup input auto repeat.
 **/
void QBPlatformLogicSetupInputAutoRepeat(void);

/**
 * @}
 **/

#endif // QB_PLATFORM_LOGIC_H
