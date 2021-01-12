/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_APP_EXTENSIONS_H_
#define QB_APP_EXTENSIONS_H_

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>

/**
 * @defgroup QBAppExtensions QBAppExtensions: application extensions
 * @ingroup CubiTV
 * @{
 **/

#define QB_GLOBAL_COLOR_TRANSFORMATION 0 ///< QBGlobalColorTransformation extension id

/**
 * Application extensions.
 * @class QBAppExtensions
 * @extends SvObject
 **/
typedef struct QBAppExtensions_ * QBAppExtensions;

/**
 * Create QBAppExtensions.
 *
 * @memberof QBAppExtensions
 *
 * This is a method that allocates and initializes QBAppExtensions.
 *
 * @param[out] *errorOut        error info
 * @return                      created QBAppExtensions instance, @c NULL in case of error
 **/
QBAppExtensions
QBAppExtensionsCreate(SvErrorInfo *errorOut);

/**
 * Add extension with given id.
 *
 * @memberof QBAppExtensions
 *
 * @param[in] self              app extensions handle
 * @param[in] id                id of extension to add
 * @param[in] extension         extension to add
 **/
void
QBAppExtensionsAdd(QBAppExtensions self, SvValue id, SvObject extension);

/**
 * Remove extension with given id.
 *
 * @memberof QBAppExtensions
 *
 * @param[in] self              app extensions handle
 * @param[in] id                id of extension to remove
 **/
void
QBAppExtensionsRemove(QBAppExtensions self, SvValue id);

/**
 * Get extension with given id.
 *
 * @memberof QBAppExtensions
 *
 * @param[in] self              app extensions handle
 * @param[in] id                id of extension to get
 * @return                      extension with given id, @c NULL if not found
 **/
SvObject
QBAppExtensionsGet(QBAppExtensions self, SvValue id);

/**
 * @}
 **/

#endif /* QB_APP_EXTENSIONS_H_ */
