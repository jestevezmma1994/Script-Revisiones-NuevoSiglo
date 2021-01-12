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

#ifndef MSTORE_VOD_ORDER_H_
#define MSTORE_VOD_ORDER_H_

/**
 * @file  MStoreVoDOrder.h
 * @brief MStoreVoD order class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <time.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvString.h>

/**
 * MStoreVoD order class.
 */
struct MStoreVoDOrder_ {
    struct SvObject_ super_; ///< Parent

    SvValue  id;        ///< Order ID
    SvString title;     ///< Content title
    time_t   authStart; ///< Start date
    time_t   authEnd;   ///< End date
};
typedef struct MStoreVoDOrder_ *MStoreVoDOrder;

/**
 * Get runtime type identification object
 * representing MStoreVoD order class.
 *
 * @return MStoreVoD order class
 **/
SvType
MStoreVoDOrder_getType(void);

#endif /* MSTORE_VOD_ORDER_H_ */
