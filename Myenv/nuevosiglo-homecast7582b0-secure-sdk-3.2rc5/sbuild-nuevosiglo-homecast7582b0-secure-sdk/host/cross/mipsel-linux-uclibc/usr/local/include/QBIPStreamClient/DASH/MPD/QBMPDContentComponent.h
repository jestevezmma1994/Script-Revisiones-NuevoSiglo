/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBMPDCONTENTCOMPONENT_H_
#define QBMPDCONTENTCOMPONENT_H_

/**
 * @file QBMPDContentComponent.h
 * @brief Content Component API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDCommonAttributes.h>

/**
 * @defgroup QBMPDContentComponent Content Component element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Content Component element class.
 * @class QBMPDContentComponent
 * @extends SvObject
 **/
typedef struct QBMPDContentComponent_ *QBMPDContentComponent;

/**
 * Get runtime type identification object representing
 * type of Content Component element class.
 *
 * @return Content Component element class
 **/
extern SvType
QBMPDContentComponent_getType(void);

/**
 * Get identifier of Content Component element.
 *
 * @param[in] self Content Component element handle
 * @return         identifier, @c -1 in case of error
 **/
extern int
QBMPDContentComponentGetId(QBMPDContentComponent self);

/**
 * Get language code of Content Component element.
 *
 * @param[in] self Content Component element handle
 * @return         language code, @c NULL in case of error
 **/
extern SvString
QBMPDContentComponentGetLang(QBMPDContentComponent self);

/**
 * Get media content type of Content Component element.
 *
 * @param[in] self Content Component element handle
 * @return         media content type, @c NULL in case of error
 **/
extern SvString
QBMPDContentComponentGetContentType(QBMPDContentComponent self);

/**
 * Get picture aspect ratio of Content Component element.
 *
 * @param[in] self Content Component element handle
 * @return         picture aspect ratio, @c NULL in case of error
 **/
extern SvString
QBMPDContentComponentGetPar(QBMPDContentComponent self);


/**
 * Get array of accessibilities (elements of type QBMPDDescriptor) of Content Component element.
 *
 * @param[in]  self     Content Component element handle
 * @return              array of accessibilities, @c NULL in case of error
 **/
extern SvArray
QBMPDContentComponentGetAccessibilities(QBMPDContentComponent self);

/**
 * Get array of roles (elements of type QBMPDDescriptor) of Content Component element.
 *
 * @param[in]  self     Content Component element handle
 * @return              array of roles, @c NULL in case of error
 **/
extern SvArray
QBMPDContentComponentGetRoles(QBMPDContentComponent self);

/**
 * Get array of ratings (elements of type QBMPDDescriptor) of Content Component element.
 *
 * @param[in]  self     Content Component element handle
 * @return              array of ratings, @c NULL in case of errorOut
 **/
extern SvArray
QBMPDContentComponentGetRatings(QBMPDContentComponent self);

/**
 * Get array of viewpoints (elements of type QBMPDDescriptor) of Content Component element.
 *
 * @param[in] self     Content Component element handle
 * @return             array of viewpoints, @c NULL in case of errorOut
 **/
extern SvArray
QBMPDContentComponentGetViewpoints(QBMPDContentComponent self);

/**
 * @}
 **/

#endif /* QBMPDCONTENTCOMPONENT_H_ */
