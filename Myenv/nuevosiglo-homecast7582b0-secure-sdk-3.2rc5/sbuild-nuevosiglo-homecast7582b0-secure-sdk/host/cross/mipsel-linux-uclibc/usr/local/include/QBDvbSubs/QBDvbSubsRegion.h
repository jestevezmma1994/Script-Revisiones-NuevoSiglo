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

#ifndef QB_DVB_SUBS_REGION_H
#define QB_DVB_SUBS_REGION_H

#include "QBDvbSubsObject.h"
#include "QBDvbSubsClut.h"

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>

#ifdef __cplusplus
extern "C" {
#endif

struct QBDvbSubsRegion_s {
  struct SvObject_ super_;

  int id;

  int x;
  int y;
  int w;
  int h;

  int clut_id;
  int pixel_depth;

  SvArray  objects;
};

typedef struct QBDvbSubsRegion_s* QBDvbSubsRegion;

/**
 * Create new dvb subtitle region.
 * \param id  as seen in EN 300 743
 * \param x horizontal position on the screen
 * \param y vertical position on the screen
 */
QBDvbSubsRegion  QBDvbSubsRegionCreate(int id, int x, int y);


/** Add single object to the region.
 */
void QBDvbSubsRegionAddObject(QBDvbSubsRegion region, QBDvbSubsObject obj);

/** Find object with specified id in this region.
 */
QBDvbSubsObject QBDvbSubsRegionFindObject(QBDvbSubsRegion region, int id);

/** Iterate over all objects in this region.
 */
SvIterator  QBDvbSubsRegionIterate(QBDvbSubsRegion region);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DVB_SUBS_REGION_H
