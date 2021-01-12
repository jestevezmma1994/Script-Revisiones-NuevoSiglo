/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_RB_LOCATOR_H_
#define SV_RB_LOCATOR_H_

/**
 * @file SvRBLocator.h
 * @brief Resource manager locator class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvRBLocator Resource manager locator class
 * @ingroup QBResourcesManagerLibrary
 *
 * A utility class that performs local file lookups.
 *
 * @{
 **/

/**
 * Resource locator class.
 * @class SvRBLocator SvRBLocator.h <QBResourceManager/SvRBLocator.h>
 * @extends SvObject
 **/
typedef struct SvRBLocator_ *SvRBLocator;


/**
 * Get runtime type identification object representing
 * resource locator class.
 *
 * @relates SvRBLocator
 *
 * @return resource locator class
 **/
extern SvType
SvRBLocator_getType(void);

/**
 * Initialize resource locator class.
 *
 * @memberof SvRBLocator
 *
 * @param[in] self      resource locator handle
 * @param[in] dirList   file search path as a list of directories
 *                      separated by colons (UNIX-style)
 * @param[in] argv0     executable name
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern SvRBLocator
SvRBLocatorInit(SvRBLocator self,
                const char *dirList,
                const char *argv0,
                SvErrorInfo *errorOut);

/**
 * Find a file in preconfigured directories list.
 *
 * @memberof SvRBLocator
 *
 * @param[in] self      resource locator handle
 * @param[in] fileName  relative file name
 * @return              newly allocated buffer containing
 *                      full path to the file, @c NULL if not found
 **/
extern char *
SvRBLocatorFindFile(SvRBLocator self,
                    const char *fileName);

/**
 * Find a file in preconfigured directories list.
 *
 * This method does the same as SvRBLocatorFindFile(), but returns
 * newly created SvString object with a path to the file.
 * Caller is responsible for releasing returned object.
 *
 * @memberof SvRBLocator
 *
 * @param[in] self      resource locator handle
 * @param[in] fileName  relative file name
 * @return              full path to the file, @c NULL if not found
 **/
extern SvString
SvRBLocatorFindFileAsString(SvRBLocator self,
                            const char *fileName);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
