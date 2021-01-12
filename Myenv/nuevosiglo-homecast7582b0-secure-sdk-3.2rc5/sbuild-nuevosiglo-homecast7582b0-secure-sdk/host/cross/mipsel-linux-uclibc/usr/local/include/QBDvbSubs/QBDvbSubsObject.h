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

#ifndef QB_DVB_SUBS_OBJECT_H
#define QB_DVB_SUBS_OBJECT_H

#include "QBDvbSubsBuff.h"

#ifdef __cplusplus
extern "C" {
#endif

struct QBDvbSubsObject_s;
typedef struct QBDvbSubsObject_s* QBDvbSubsObject;

/**
 * Create new dvb subtitle object.
 * \param id  as seen in EN 300 743
 * \param x horizontal position within the region this object belongs to
 * \param y vertical position within the region this object belongs to
 */
QBDvbSubsObject  QBDvbSubsObjectCreate(int id, int x, int y);

/** Get id of this object.
 */
int QBDvbSubsObjectGetId(QBDvbSubsObject obj);

/** Get position of the object within its region.
 */
void QBDvbSubsObjectGetPosition(QBDvbSubsObject obj, int* x, int* y);

/** Get poiter to pixel data
 */
unsigned char* QBDvbSubsObjectGetPixelData(QBDvbSubsObject obj);

/** Get line length and number of lines
 */
void QBDvbSubsObjectGetLineParams(QBDvbSubsObject obj, int* lineLength, int* lineNum);

/** Parses bits and creates index array
 * \param obj   dvb subtitle object
 * \param data  encoded, as seen in EN 300 743
 * \param len   length of \a data
 * \param top   if true top pixel data, otherwise bottom pixel data
 */
void QBDvbSubsObjectParseBits(QBDvbSubsObject obj, QBDvbSubsBuff buff, const unsigned char* data,
                              int top_data_len, int bottom_data_len);

bool QBDvbSubsObjectWasPrinted(QBDvbSubsObject obj);

void QBDvbSubsObjectSetPrinted(QBDvbSubsObject obj);

// TODO: functions for decoding bitmaps.

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DVB_SUBS_OBJECT_H
