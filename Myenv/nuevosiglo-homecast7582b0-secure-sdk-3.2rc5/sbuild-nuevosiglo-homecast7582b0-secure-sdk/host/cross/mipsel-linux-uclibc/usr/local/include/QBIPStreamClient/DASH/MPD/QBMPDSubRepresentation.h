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

#ifndef QBMPDSUBREPRESENTATION_H_
#define QBMPDSUBREPRESENTATION_H_

/**
 * @file QBMPDSubRepresentation.h
 * @brief Sub-Representation element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>

/**
 * @defgroup QBMPDSubRepresentation Sub-Representation element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Sub-Representation element class.
 * @class QBMPDSubRepresentation
 * @extends QBMPDRepresentationBase
 **/
typedef struct QBMPDSubRepresentation_ *QBMPDSubRepresentation;

/**
 * Get runtime type identification object representing
 * type of Sub-Representation element class.
 *
 * @return Sub-Representation element class
 **/
extern SvType
QBMPDSubRepresentation_getType(void);

/**
 * Get level of Sub-Representation element.
 *
 * @param[in] self Sub-Representation element handle
 * @return         level, @c -1 in case of error
 **/
extern int
QBMPDSubRepresentationGetLevel(QBMPDSubRepresentation self);

/**
 * Get array of Sub-Representation @c level values of Sub-Representation element.
 *
 * @param[in]  self                 Sub-Representation element handle
 * @param[out] dependencyLevelCount number of elements in returned array
 * @param[out] errorOut             error info
 * @return                          array of Sub-Representation @c level values, @c NULL in case of error
 **/
extern unsigned int *
QBMPDSubRepresentationGetDependencyLevel(QBMPDSubRepresentation self,
                                         unsigned int *dependencyLevelCount,
                                         SvErrorInfo *errorOut);

/**
 * Get bandwidth of Sub-Representation element.
 *
 * @param[in] self Sub-Representation element handle
 * @return         bandwidth, @c -1 in case of error
 **/
extern int
QBMPDSubRepresentationGetBandwidth(QBMPDSubRepresentation self);

/**
 * Get all media components as whitespace-separated list of @c ContentComponent ids of Sub-Representation.
 *
 * @param[in] self Sub-Representation element handle
 * @return         all media components contained in this Sub-Representation, @c NULL in case of error
 **/
extern SvString
QBMPDSubRepresentationGetContentComponent(QBMPDSubRepresentation self);

/**
 * @}
 **/

#endif /* QBMPDSUBREPRESENTATION_H_ */
