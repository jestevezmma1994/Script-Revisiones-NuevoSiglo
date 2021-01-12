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

#ifndef QBMPDREPRESENTATION_H_
#define QBMPDREPRESENTATION_H_

/**
 * @file QBMPDRepresentation.h
 * @brief Representation API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentBase.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentList.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDSegmentTemplate.h>

/**
 * @defgroup QBMPDRepresentation Representation element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Representation element class.
 * @class QBMPDRepresentation
 * @extends QBMPDRepresentationBase
 **/
typedef struct QBMPDRepresentation_ *QBMPDRepresentation;

/**
 * Get runtime type identification object representing
 * type of Representation element class.
 *
 * @return Representation element class
 **/
extern SvType
QBMPDRepresentation_getType(void);

/**
 * Get identifier of Representation element.
 *
 * @param[in] self Representation element handle
 * @return         identifier, @c NULL in case of error
 **/
extern SvString
QBMPDRepresentationGetId(QBMPDRepresentation self);

/**
 * Get bandwidth attribute of Representation element.
 *
 * @param[in] self Representation element handle
 * @return         bandwidth, @c -1 in case of error
 **/
extern int
QBMPDRepresentationGetBandwidth(QBMPDRepresentation self);

/**
 * Get quality ranking attribute of Representation element.
 *
 * @param[in] self Representation element handle
 * @return         quality ranking, @c -1 in case of error
 **/
extern int
QBMPDRepresentationGetQualityRanking(QBMPDRepresentation self);

/**
 * Get dependency id attribute of Representation element.
 *
 * @param[in] self Representation element handle
 * @return         dependency id, @c NULL in case of error
 **/
extern SvString
QBMPDRepresentationGetDependencyId(QBMPDRepresentation self);

/**
 * Get media stream structure id attribute of Representation element.
 *
 * @param[in] self Representation element handle
 * @return         media stream structure id, @c NULL in case of error
 **/
extern SvString
QBMPDRepresentationGetMediaStreamStructureId(QBMPDRepresentation self);

/**
 * Get array of Base URL elements of Representation element.
 *
 * @param[in]  self     Representation element handle
 * @return              array of Base URL elements, @c NULL in case of error
 **/
extern SvArray
QBMPDRepresentationGetBaseURLs(QBMPDRepresentation self);

/**
 * Get array of Sub-Representation elements of Representation element.
 *
 * @param[in]  self     Representation element handle
 * @return              array of Sub-Representation elements, @c NULL in case of error
 **/
extern SvArray
QBMPDRepresentationGetSubRepresentations(QBMPDRepresentation self);

/**
 * Get Segment Base element of Representation element.
 *
 * @param[in]  self     Representation element handle
 * @return              Segment Base element, @c NULL in case of error
 **/
extern QBMPDSegmentBase
QBMPDRepresentationGetSegmentBase(QBMPDRepresentation self);

/**
 * Get Segment List element of Representation element.
 *
 * @param[in]  self     Representation element handle
 * @return              Segment Base element, @c NULL in case of error
 **/
extern QBMPDSegmentList
QBMPDRepresentationGetSegmentList(QBMPDRepresentation self);

/**
 * Get Segment Template element of Representation element.
 *
 * @param[in] self Representation element handle
 * @return         Segment Template element, @c NULL in case of error
 **/
extern QBMPDSegmentTemplate
QBMPDRepresentationGetSegmentTemplate(QBMPDRepresentation self);

/**
 * @}
 **/

#endif /* QBMPDREPRESENTATION_H_ */
