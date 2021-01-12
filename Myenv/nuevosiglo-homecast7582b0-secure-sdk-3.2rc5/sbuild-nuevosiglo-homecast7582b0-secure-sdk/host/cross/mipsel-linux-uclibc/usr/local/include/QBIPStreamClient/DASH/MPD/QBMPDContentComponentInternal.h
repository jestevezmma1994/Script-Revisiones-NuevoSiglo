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

#ifndef QBMPDCONTENTCOMPONENTINTERNAL_H_
#define QBMPDCONTENTCOMPONENTINTERNAL_H_

/**
 * @file QBMPDContentComponentInternal.h
 * @brief Content Component element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDCommonAttributes.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDDescriptor.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDContentComponent.h>

/**
 * @addtogroup QBMPDContentComponent
 * @{
 **/

/**
 * Create Content Component element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Content Component element, @c NULL in case of error
 **/
extern QBMPDContentComponent
QBMPDContentComponentCreate(SvErrorInfo *errorOut);

/**
 * Set identifier for Content Component element.
 *
 * @param[in]  self     Content Component element handle
 * @param[in]  id       identifier for media component
 * @param[out] errorOut error info
 **/
extern void
QBMPDContentComponentSetId(QBMPDContentComponent self,
                           int id,
                           SvErrorInfo *errorOut);

/**
 * Set language code for Content Component element.
 *
 * @param[in]  self     Content Component element handle
 * @param[in]  language language code
 * @param[out] errorOut error info
 **/
extern void
QBMPDContentComponentSetLang(QBMPDContentComponent self,
                             SvString language,
                             SvErrorInfo *errorOut);

/**
 * Set content type for Content Component element.
 *
 * @param[in]  self         Content Component element handle
 * @param[in]  contentType  media content type
 * @param[out] errorOut     error info
 **/
extern void
QBMPDContentComponentSetContentType(QBMPDContentComponent self,
                                    SvString contentType,
                                    SvErrorInfo *errorOut);

/**
 * Set picture aspect ratio for Content Component element.
 *
 * @param[in]  self     Content Component element handle
 * @param[in]  par      picture aspect ratio of the video media component type
 * @param[out] errorOut error info
 **/
extern void
QBMPDContentComponentSetPar(QBMPDContentComponent self,
                            SvString par,
                            SvErrorInfo *errorOut);

/**
 * Add accessibility scheme to Content Component element.
 *
 * @param[in]  self          Content Component element handle
 * @param[in]  accessibility information about accessibility scheme
 * @param[out] errorOut      error info
 **/
extern void
QBMPDContentComponentAddAccessiblity(QBMPDContentComponent self,
                                     QBMPDDescriptor accessibility,
                                     SvErrorInfo *errorOut);

/**
 * Add role annotation scheme to Content Component element.
 *
 * @param[in]  self     Content Component element handle
 * @param[in]  role     information about role annotation scheme
 * @param[out] errorOut error info
 **/
extern void
QBMPDContentComponentAddRole(QBMPDContentComponent self,
                             QBMPDDescriptor role,
                             SvErrorInfo *errorOut);

/**
 * Add rating scheme to Content Component element.
 *
 * @param[in]  self     Content Component element handle
 * @param[in]  rating   information about rating scheme
 * @param[out] errorOut error info
 **/
extern void
QBMPDContentComponentAddRating(QBMPDContentComponent self,
                               QBMPDDescriptor rating,
                               SvErrorInfo *errorOut);

/**
 * Add viewpoint annotation scheme to Content Component element.
 *
 * @param[in]  self      Content Component element handle
 * @param[in]  viewpoint information about viewpoint annotation scheme
 * @param[out] errorOut  error info
 **/
extern void
QBMPDContentComponentAddViewpoint(QBMPDContentComponent self,
                                  QBMPDDescriptor viewpoint,
                                  SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDCONTENTCOMPONENTINTERNAL_H_ */
