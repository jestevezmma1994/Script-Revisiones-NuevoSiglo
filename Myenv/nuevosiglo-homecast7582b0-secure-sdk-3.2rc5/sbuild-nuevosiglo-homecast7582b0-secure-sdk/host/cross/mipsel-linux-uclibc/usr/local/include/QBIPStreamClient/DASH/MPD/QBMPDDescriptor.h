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

#ifndef QBMPDDESCRIPTOR_H_
#define QBMPDDESCRIPTOR_H_

/**
 * @file QBMPDDescriptor.h
 * @brief Descriptor element API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>

/**
 * @defgroup QBMPDDescriptor Descriptor element class
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Descriptor element class.
 * @class QBMPDDescriptor
 * @extends SvObject
 **/
typedef struct QBMPDDescriptor_ *QBMPDDescriptor;

/**
 * Get runtime type identification object representing
 * type of Descriptor element class.
 *
 * @return Descriptor element class
 **/
extern SvType
QBMPDDescriptor_getType(void);

/**
 * Get URI which specifies scheme of Descriptor element.
 *
 * @param[in] self Descriptor element handle
 * @return         URI which specifies scheme, @c NULL in case of error
 **/
extern SvURI
QBMPDDescriptorGetSchemeIdURI(QBMPDDescriptor self);

/**
 * Get value of Descriptor element.
 *
 * @param[in] self Descriptor element handle
 * @return         value, @c NULL in case of error
 **/
extern SvString
QBMPDDescriptorGetValue(QBMPDDescriptor self);

/**
 * Get id of Descriptor element.
 *
 * @param[in] self Descriptor element handle
 * @return         identifier, @c NULL in case of error
 **/
extern SvString
QBMPDDescriptorGetId(QBMPDDescriptor self);

/**
 * Get text of Descriptor element.
 *
 * @param[in] self Descriptor element handle
 * @return         text, @c NULL in case of error
 **/
extern SvString
QBMPDDescriptorGetText(QBMPDDescriptor self);

/**
 * @}
 **/

#endif /* QBMPDDESCRIPTOR_H_ */
