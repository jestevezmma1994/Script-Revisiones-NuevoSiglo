/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QBICSMARTCARDDECRYPTIONSTATE_H_
#define QBICSMARTCARDDECRYPTIONSTATE_H_

#include <SvFoundation/SvCoreTypes.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QBICSmartcardDecryptionState_s {
    struct SvObject_ super_;

    bool start;
    bool isFta;
} * QBICSmartcardDecryptionState;

extern QBICSmartcardDecryptionState QBICSmartcardDecryptionStateCreate(void);

#ifdef __cplusplus
}
#endif


#endif /* QBICSMARTCARDDECRYPTIONSTATE_H_ */
