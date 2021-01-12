/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
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
****************************************************************************/

#ifndef MPEG_TABLES_H
#define MPEG_TABLES_H

/////////////////////////////////////////////
// PID consts
#define MPEG_PID_PAT        0
#define MPEG_PID_CAT        0x01
#define MPEG_PID_NULL       0x1FFF
#define MPEG_PID_INVALID    (~0U)
#define MPEG_PID_NIT        0x10
#define MPEG_PID_SDT        0x11
#define MPEG_PID_BAT        0x11
#define MPEG_PID_EIT        0x12
#define MPEG_PID_TDT        0x14
/////////////////////////////////////////////


/////////////////////////////////////////////
// table_id consts
#define MPEG_TABLE_ID_RESERVED          0xFF
#define MPEG_TABLE_ID_PAT               0x00
#define MPEG_TABLE_ID_CAT               0x01
#define MPEG_TABLE_ID_PMT               0x02
#define MPEG_TABLE_ID_METADATA          0x06

#define MPEG_TABLE_ID_NIT_ACTUAL        0x40
#define MPEG_TABLE_ID_NIT_OTHER         0x41
#define MPEG_TABLE_ID_NIT_MASK          0xfe

#define MPEG_TABLE_ID_SDT_ACTUAL        0x42
#define MPEG_TABLE_ID_SDT_OTHER         0x46
#define MPEG_TABLE_ID_SDT_MASK          0xfb

#define MPEG_TABLE_ID_BAT               0x4a

#define MPEG_TABLE_ID_EIT_PF_ACTUAL     0x4e
#define MPEG_TABLE_ID_EIT_PF_OTHER      0x4f
#define MPEG_TABLE_ID_EIT_PF_MASK       0xfe

#define MPEG_TABLE_ID_TDT               0x70
#define MPEG_TABLE_ID_TDT_TOT           0x73
#define MPEG_TABLE_ID_TDT_MASK          0xfc

#define MPEG_TABLE_ID_ECM_1             0x82
/////////////////////////////////////////////

#endif // #ifndef MPEG_TABLES_H
