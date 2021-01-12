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

#ifndef QBMPDDESCRIPTORINTERNAL_H_
#define QBMPDDESCRIPTORINTERNAL_H_

/**
 * @file QBMPDDescriptorInternal.h
 * @brief Descriptor element private API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvURI.h>
#include <QBIPStreamClient/DASH/MPD/QBMPDDescriptor.h>

/**
 * @addtogroup QBMPDDescriptor
 * @{
 **/

/**
 * Create Descriptor element.
 *
 * @param[in]  schemeIdURI URI to identify scheme
 * @param[out] errorOut    error info
 * @return                 new instance of Descriptor, @c NULL in case of error
 **/
extern QBMPDDescriptor
QBMPDDescriptorCreate(SvURI schemeIdURI,
                      SvErrorInfo *errorOut);

/**
 * Set value for Descriptor element.
 *
 * @param[in]  self     Descriptor element handle
 * @param[in]  value    value
 * @param[out] errorOut error info
 **/
extern void
QBMPDDescriptorSetValue(QBMPDDescriptor self,
                        SvString value,
                        SvErrorInfo *errorOut);

/**
 * Set identifier for Descriptor element.
 *
 * @param[in] self      Descriptor element handle
 * @param[in] id        identifier
 * @param[out] errorOut error info
 **/
extern void
QBMPDDescriptorSetId(QBMPDDescriptor self,
                     SvString id,
                     SvErrorInfo *errorOut);

/**
 * Set text for Descriptor element.
 *
 * @param[in]  self     Descriptor element handle
 * @param[in]  text     text for descriptor
 * @param[out] errorOut error info
 **/
extern void
QBMPDDescriptorSetText(QBMPDDescriptor self,
                       SvString text,
                       SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QBMPDDESCRIPTORINTERNAL_H_ */
