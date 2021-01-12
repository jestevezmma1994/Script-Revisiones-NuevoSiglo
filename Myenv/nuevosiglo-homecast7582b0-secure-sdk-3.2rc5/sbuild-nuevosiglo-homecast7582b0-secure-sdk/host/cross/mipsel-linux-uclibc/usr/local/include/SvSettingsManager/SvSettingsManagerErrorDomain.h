/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_SETTINGS_MANAGER_ERROR_DOMAIN_H_
#define SV_SETTINGS_MANAGER_ERROR_DOMAIN_H_

/**
 * @file SvSettingsManagerErrorDomain.h Settings Manager error domain
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvSettingsManagerErrorDomain Settings Manager error domain
 * @ingroup SvSettingsManager
 * @{
 *
 * Error codes returned by modules of this library.
 **/

/**
 * Settings Manager error domain.
 **/
#define SvSettingsManagerErrorDomain "com.sentivision.SvSettingsManager"

/**
 * Settings Manager error domain codes.
 **/
typedef enum {
    /** @cond */
    SvSettingsManagerError_reserved = 0,
    /** @endcond */
    /// error parsing settings file
    SvSettingsManagerError_parseError,
    /// error reading settings file
    SvSettingsManagerError_ioError,
    /// can't load resource specified in settings file
    SvSettingsManagerError_resourceMissing,
    /** @cond */
    SvSettingsManagerError_last
    /** @endcond */
} SvSettingsManagerError;

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
