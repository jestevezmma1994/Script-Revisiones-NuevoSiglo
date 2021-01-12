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

#ifndef QBMPDSUBREPRESENTATIONINTERNAL_H_
#define QBMPDSUBREPRESENTATIONINTERNAL_H_

/**
 * @file QBMPDSubRepresentationInternal.h
 * @brief Sub-Representation element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSubRepresentation.h>

/**
 * @addtogroup QBMPDSubRepresentation
 * @{
 **/

/**
 * Create Sub-Representation element.
 *
 * @param[out] errorOut error info
 * @return              new instance of Sub-Representation element, @c NULL in case of error
 **/
extern QBMPDSubRepresentation
QBMPDSubRepresentationCreate(SvErrorInfo *errorOut);

/**
 * Set level for Sub-Representation element.
 *
 * @param[in]  self     Sub-Representation element handle
 * @param[in]  level    Sub-Representation level
 * @param[out] errorOut error info
 **/
extern void
QBMPDSubRepresentationSetLevel(QBMPDSubRepresentation self,
                               int level,
                               SvErrorInfo *errorOut);

/**
 * Set array of Sub-Representation @c level values for Sub-Representation element.
 *
 * @param[in]  self                 Sub-Representation element handle
 * @param[in]  dependencyLevel      array of Sub-Representation @c level values
 * @param[in]  dependencyLevelCount number of elements in array @c dependencyLevel
 * @param[out] errorOut             error info
 **/
extern void
QBMPDSubRepresentationSetDependencyLevel(QBMPDSubRepresentation self,
                                         unsigned int *dependencyLevel,
                                         unsigned int dependencyLevelCount,
                                         SvErrorInfo *errorOut);

/**
 * Set bandwidth for Sub-Representation element.
 *
 * @param[in]  self      Sub-Representation element handle
 * @param[in]  bandwidth bandwidth in bits per second
 * @param[out] errorOut  error info
 **/
extern void
QBMPDSubRepresentationSetBandwidth(QBMPDSubRepresentation self,
                                   int bandwidth,
                                   SvErrorInfo *errorOut);

/**
 * Set all media components as whitespace-separated list of @c ContentComponent ids for Sub-Representation.
 *
 * @param[in]  self             Sub-Representation element handle
 * @param[in]  contentComponent array of media components as whitespace-separated list of @c ContentComponent ids
 * @param[out] errorOut         error info
 **/
extern void
QBMPDSubRepresentationSetContentComponent(QBMPDSubRepresentation self,
                                          SvString contentComponent,
                                          SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDSUBREPRESENTATIONINTERNAL_H_ */
