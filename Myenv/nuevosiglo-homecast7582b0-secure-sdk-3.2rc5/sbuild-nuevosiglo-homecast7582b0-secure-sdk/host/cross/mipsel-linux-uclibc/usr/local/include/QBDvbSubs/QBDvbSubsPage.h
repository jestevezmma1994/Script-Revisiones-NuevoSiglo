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

#ifndef QB_DVB_SUBS_PAGE_H
#define QB_DVB_SUBS_PAGE_H

#include "QBDvbSubsObject.h"
#include "QBDvbSubsClut.h"
#include "QBDvbSubsRegion.h"

#include <SvFoundation/SvIterator.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


struct QBDvbSubsDDS_s {
    int x, y;
    int w, h;
};

struct QBDvbSubsPage_s {
  struct SvObject_ super_;

  int id;
  int64_t pts;
  int timeout;

  SvArray  clut_tab;
  SvArray  region_tab;

  struct QBDvbSubsDDS_s dds;
  bool dds_present;
};
typedef struct QBDvbSubsPage_s* QBDvbSubsPage;

/**
 * Create new dvb subtitle page.
 * \param id  page id, as seen in subtitle_descriptor
 * \param pts  33-bit 90kHz timestamp, for synchronizing display with the a/v stream
 * \param timeout  timeout (in seconds) after thich this page is no longer valid and must not be displayed (even if there is no newer version)
 */
QBDvbSubsPage  QBDvbSubsPageCreate(int id, int timeout);


/** Add single Color Lookup Table to the page.
 */
void QBDvbSubsPageAddClut(QBDvbSubsPage page, QBDvbSubsClut clut);

/** Find specific Color Lookup Table in this page.
 */
QBDvbSubsClut QBDvbSubsPageFindClut(QBDvbSubsPage page, int id);


/** Add single region to the page.
 */
void QBDvbSubsPageAddRegion(QBDvbSubsPage page, QBDvbSubsRegion region);

/** Find specific region in this page.
 */
QBDvbSubsRegion QBDvbSubsPageFindRegion(QBDvbSubsPage page, int id);

/** Iterate over all regions in this page.
 */
SvIterator  QBDvbSubsPageIterate(QBDvbSubsPage page);


/** Find specific object in this page.
 */
QBDvbSubsObject QBDvbSubsPageFindObject(QBDvbSubsPage page, int id);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_DVB_SUBS_PAGE_H
