/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QBCASINFO_H_
#define QBCASINFO_H_

/**
 * @file QBCASInfo.h QBCASInfo
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 */

#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <stdint.h>

enum QBCASType_e
{
    QBCASType_unknown,
    QBCASType_conax,
    QBCASType_cryptoguard,
    QBCASType_cryptoguard2,
    QBCASType_comvenient,
    QBCASType_viewrightDVB,
    QBCASType_viewrightIPTV,
    QBCASType_latens,
    QBCASType_generic
};
typedef enum QBCASType_e  QBCASType;

/**
 * Converts smartcard type to name
 */
const char* QBCASTypeToString(QBCASType type);

/**
 * Information about CAS
 */
struct QBCASInfo_t {
  struct SvObject_ super_;

  QBCASType cas_type;
  uint16_t ca_sys_id;

  /// only one of: (complete) or (unique) is given
  SvString  card_number_complete;
  SvString  card_number_part_unique;

  /// @deprecated
  SvString  card_number_part_group;

  SvString  chip_id;
  SvString  sw_ver;

};
typedef struct QBCASInfo_t* QBCASInfo;

QBCASInfo QBCASInfoCreate(SvErrorInfo *errorOut);
SvType QBCASInfo_getType(void);

/**
 * Returns card number
 */
SvString QBCASInfoGetDescriptiveCardNum(const QBCASInfo info);

#endif /* QBCASINFO_H_ */
