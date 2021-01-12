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

#ifndef QB_OSK_BG_STRIPE_GEOMETRY_H_
#define QB_OSK_BG_STRIPE_GEOMETRY_H_

/**
 * @file QBOSKBGStripeGeometry.h
 * @brief On Screen Keyboard stripe geometry class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBOSKBGStripeGeometry On Screen Keyboard background stripe geometry class
 * @ingroup QBOSKCore
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

typedef struct QBOSKBGStripeGeometry_ *QBOSKBGStripeGeometry;

/**
 * Create QBOSKBGStripeGeometry object.
 * It represents a logical structure of a stripe. It is used to create a hash table of stripes
 * to reuse bitmaps if a stripe of the same structure already exists.
 *
 * @param[in] keysArray     array of keys to be used in this stripe
 * @param[in] arrayLength   length of keys array
 * @param[out] errorOut     error info
 * @return QBOSKStripeGeometry created object
 **/
extern QBOSKBGStripeGeometry
QBOSKBGStripeGeometryCreate(char *keysArray, unsigned int arrayLength, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif
