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

#ifndef VOD_LOGIC_H
#define VOD_LOGIC_H

/**
 * @file  VoDLogic.h
 * @brief VOD Logic API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <QBApplicationController.h>
#include <QBContentManager/QBContentCategory.h>
#include <SvDataBucket2/SvDBRawObject.h>

/**
 * @defgroup VODLogic VOD logic
 * @ingroup CubiTV_logic
 * @{
 **/

typedef struct QBVoDLogic_t *QBVoDLogic;

/**
* Method that handles input events for VOD views
*
* @param[in] self          VOD Logic handle
* @param[in] ev            event representing input from user
* @param[in] data_         data associated with an event
* @return                  @c 0 if the event have not been consumed, otherwise any other value
*/
int QBVoDLogicInputHandler(QBVoDLogic self, SvInputEvent ev, SvObject data_) __attribute__((weak));

/**
* Method that creates an instance of logic class for VOD views
*
* @param[in] appGlobals    application globals handle
* @return                  VOD Logic handle, @c NULL in case of error
*/
QBVoDLogic QBVoDLogicNew(AppGlobals appGlobals) __attribute__((weak));

/**
* Method that creates a window context for content object
*
* @param[in] self          VOD Logic handle
* @param[in] tree          content tree
* @param[in] provider      content provider
* @param[in] path          path to an object
* @param[in] service       content service
* @param[in] object        content object
* @return                  window context handle, @c NULL in case of error
*/
QBWindowContext QBVoDLogicCreateContextForObject(QBVoDLogic self, SvObject tree, SvObject provider, SvObject path, SvObject service, SvObject object) __attribute__((weak));

/**
 * Method that decides if category should be protected with PIN because of rating or adult flag
 *
 * @param[in] appGlobals    application globals handle
 * @param[in] category      category to be checked
 * @return                  true if category should be protected and false otherwise
 **/
bool
QBVODLogicCategoryShouldBeBlocked(AppGlobals appGlobals, QBContentCategory category) __attribute__((weak));

/**
 * Method that decides if category should be protected with PIN because of rating
 *
 * @param[in] appGlobals    application globals handle
 * @param[in] category      category to be checked
 * @return                  true if category should be protected and false otherwise
 **/
bool
QBVODLogicCategoryIsProtected(AppGlobals appGlobals, QBContentCategory category) __attribute__((weak));

/**
 * Method that decides if movie playback is allowed by parental control
 *
 * @param[in] appGlobals    application globals handle
 * @param[in] movie         movie to be checked
 * @return                  true if movie playback is allowed and false otherwise
 */
bool
QBVODLogicAreMoviePlaybackPCAllowed(AppGlobals appGlobals, SvDBRawObject movie) __attribute__((weak));

/**
 * Get maximum allowed size of buffer.
 *
 * @return allowed size, in bytes
 */
int
QBVODLogicGetAllowedSizeOfBuffer(void) __attribute__((weak));

/**
 * Function creates string with message about lack of rating.
 *
 * @return string with message, or null when error occurred.
 **/
SvString QBVODLogicCreateNoRatingMessage(void);

/**
 * Function deciding if the VOD provider should be reset to 'cubimw' if MW does not specify the provider
 *
 * @return @c true if the box should change the provider to 'cubimw' if MW does not use different provider, and @c false otherwise
 **/
bool QBVODLogicShouldResetProviderIfNotSpecifiedByMW(void);
/**
 * @}
 **/
#endif
