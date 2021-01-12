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

#ifndef QBDVBCHANNELDESC_H
#define QBDVBCHANNELDESC_H

#include <stdbool.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvHashTable.h>
#include <QBDVBScannerTypes.h>

typedef struct QBDVBChannelDesc_s *QBDVBChannelDesc;
struct QBDVBChannelDesc_s {
  struct SvObject_ super_;

  /// [onid,sid,tisd] must be unique, must be known.
  int sid;
  int tsid;

  bool foundInNIT;

  /// Found in NIT or SDT.
  int onid;
  int nid;
  QBDVBChannelType  type;

  /// Found int NIT (actual, other). -1 iff unknown
  /// Logical Channel Number
  int lcn;
  bool visible;
  bool running;
  bool free_CA_mode;

  bool countryAvailability;

  /// The only fields we need from SDT
  SvString name;
  SvString provider;

  SvArray links;

  SvHashTable muxidToVariant;

  QBTunerType tunerType;

  SvHashTable metadata;
};


QBDVBChannelDesc QBDVBChannelDescCreate(int sid, int tsid, int onid, QBDVBChannelType type, QBTunerType tunerType);
bool QBDVBChannelDescHasLCN(const QBDVBChannelDesc self);

#endif // QBDVBCHANNELDESC_H
