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
******************************************************************************/

#ifndef SVDATAFORMAT_CHECK_CHANGES_H
#define SVDATAFORMAT_CHECK_CHANGES_H

#include "sv_data_format.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum svdataformat_check_audio_flags
{
  svdataformat_check_audio__skip_dual_mono = 1,
};

/** Check if video/audio services have changed.
 *  Transport-stream only.
 */
//bool svdataformat_check_changes_ts_va(const struct svdataformat* format1, const struct svdataformat* format2);

/**
 * Check if content protection parameters have changed.
 *
 * @param[in] format1: handle to base dataformat
 * @param[in] format2: handle to date format which should be compared
 * @return true when changed, false otherwise
 */
bool svdataformat_check_changes_ts_content_protection(const struct svdataformat* format1,
                                                      const struct svdataformat* format2);
/** Check if conditional-access parameters have changed.
 *  Transport-stream only.
 */
bool svdataformat_check_changes_ts_ca(const struct svdataformat* format1,
                                      const struct svdataformat* format2);


bool svdataformat_check_video_changes(const struct svdataformat* format1,
                                      const struct svdataformat* format2,
                                      int track1,
                                      int track2);

bool svdataformat_check_audio_changes(const struct svdataformat* format1,
                                      const struct svdataformat* format2,
                                      int track1,
                                      int track2,
                                      unsigned int flags);


#ifdef __cplusplus
}
#endif

#endif // #ifndef SVDATAFORMAT_CHECK_CHANGES_H
