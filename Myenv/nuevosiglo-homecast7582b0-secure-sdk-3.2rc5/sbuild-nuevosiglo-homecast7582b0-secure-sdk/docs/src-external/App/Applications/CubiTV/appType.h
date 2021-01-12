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

#ifndef QBAPPTYPE_H_
#define QBAPPTYPE_H_

/**
 * @file appType.h Methods for getting application type
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>

/**
 * @defgroup QBAppType Methods for getting application type
 * @ingroup CubiTV_utils
 * @{
 */

/**
 * Is application type DVB
 *
 * @return    @c true if application type is DVB, @c false otherwise
 **/
bool QBAppTypeIsDVB(void)__attribute__((weak));

/**
 * Is application type IP
 *
 * @return    @c true if application type is IP, @c false otherwise
 **/
bool QBAppTypeIsIP(void)__attribute__((weak));

/**
 * Is application type SAT
 *
 * @return    @c true if application type is SAT, @c false otherwise
 **/
bool QBAppTypeIsSAT(void)__attribute__((weak));

/**
 * Is application type Hybrid
 *
 * @return    @c true if application type is Hybrid, @c false otherwise
 **/
bool QBAppTypeIsHybrid(void)__attribute__((weak));

/**
 * Is application type PVR (internal disk)
 *
 * @return    @c true if application type is PVR, @c false otherwise
 **/
bool QBAppTypeIsPVR(void)__attribute__((weak));

/**
 * @}
 **/

#endif
