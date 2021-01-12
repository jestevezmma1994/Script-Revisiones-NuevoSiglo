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

#ifndef QBMPDCOMMONATTRIBUTES_H_
#define QBMPDCOMMONATTRIBUTES_H_

/**
 * @file QBMPDCommonAttributes.h
 * @brief Common Attributes types
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBMPDCommonAttributes Common Attributes types
 * @ingroup QBMPDElement
 * @{
 **/

/**
 * Stream Access Point type
 **/
typedef enum {
    QBMPDCommonAttributesSAPType_unknown = -1, ///< special marker when type is unknown
    QBMPDCommonAttributesSAPType_0,            ///< SAP type 0
    QBMPDCommonAttributesSAPType_1,            ///< SAP type 1
    QBMPDCommonAttributesSAPType_2,            ///< SAP type 2
    QBMPDCommonAttributesSAPType_3,            ///< SAP type 3
    QBMPDCommonAttributesSAPType_4,            ///< SAP type 4
    QBMPDCommonAttributesSAPType_5,            ///< SAP type 5
    QBMPDCommonAttributesSAPType_6             ///< SAP type 6
} QBMPDCommonAttributesSAPType;

/**
 * Video Scan type
 **/
typedef enum {
    QBMPDCommonAttributesVideoScanType_unknown = -1, ///< special marker when type is unknown
    QBMPDCommonAttributesVideoScanType_P,            ///< progressive mode
    QBMPDCommonAttributesVideoScanType_I             ///< interlaced mode
} QBMPDCommonAttributesVideoScanType;

/**
 * @}
 **/

#endif /* QBMPDCOMMONATTRIBUTES_H_ */
