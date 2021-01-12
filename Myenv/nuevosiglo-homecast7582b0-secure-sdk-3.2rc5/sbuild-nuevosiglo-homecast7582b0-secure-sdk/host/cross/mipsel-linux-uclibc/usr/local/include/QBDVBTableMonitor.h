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

#ifndef QBDVBTABLEMONITOR_H_
#define QBDVBTABLEMONITOR_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvData.h>
#include <mpeg_tables/sdt.h>
#include <mpeg_tables/nit.h>
#include <mpeg_tables/cat.h>
#include <mpeg_tables/pat.h>
#include <mpeg_tables/bat.h>

typedef enum {
    QBDVBTableMonitorTableID_Undefined = 0, // 0
    QBDVBTableMonitorTableID_NIT,
    QBDVBTableMonitorTableID_SDT, // 2
    QBDVBTableMonitorTableID_CAT,
    QBDVBTableMonitorTableID_PAT, // 4
    QBDVBTableMonitorTableID_BAT
} QBDVBTableMonitorTableID;

struct QBDVBTableMonitorListener_t {
    void (*NITVersionChanged)(SvObject self_, unsigned int tunerNum, mpeg_nit_parser *parser);
    void (*SDTVersionChanged)(SvObject self_, unsigned int tunerNum, mpeg_sdt_parser *parser);
    void (*CATVersionChanged)(SvObject self_, unsigned int tunerNum, mpeg_cat_parser *parser);
    void (*PATVersionChanged)(SvObject self_, unsigned int tunerNum, mpeg_pat_parser *parser);
    void (*BATVersionChanged)(SvObject self_, unsigned int tunerNum, mpeg_bat_parser *parser);
    void (*tableRepeated)(SvObject self_, unsigned int tunerNum, SvBuf chb, QBDVBTableMonitorTableID id);
};
typedef struct QBDVBTableMonitorListener_t *QBDVBTableMonitorListener;

SvInterface QBDVBTableMonitorListener_getInterface(void);


struct QBDVBLinkageDescriptorListener_ {
    void (*descriptorReceived)(SvObject self_, QBDVBTableMonitorTableID tableID, uint8_t linkageType, SvData descriptorData);
};
typedef const struct QBDVBLinkageDescriptorListener_ *QBDVBLinkageDescriptorListener;

SvInterface QBDVBLinkageDescriptorListener_getInterface(void);

/**
 * QBDVBTableMonitor type
 *
 * @class QBDVBTableMonitor
 * @extends SvObject
 * @implements QBConfigListener
 * @implements QBAsyncService
 * @implements QBInitializable
*
 * This class expects to be initialized by @link QBInitializable @endlink
 * interface init method and has following dependencies that shall be passed in
 * properties map:
 *
 * | Name                | Type                | Description
 * | ------------------- | ------------------- | -------------------------------------
 * | tunerCnt            | int                 | number of tuners
 **/
typedef struct QBDVBTableMonitor_t *QBDVBTableMonitor;

/**
 * Get runtime type identification object representing QBDVBTableMonitor class.
 *
 * @relates QBDVBTableMonitor
 *
 * @return type identification object
**/
SvType
QBDVBTableMonitor_getType(void);

void QBDVBTableMonitorAddListener(QBDVBTableMonitor self, SvObject listener);

void QBDVBTableMonitorRemoveListener(QBDVBTableMonitor self, SvObject listener);

#endif
