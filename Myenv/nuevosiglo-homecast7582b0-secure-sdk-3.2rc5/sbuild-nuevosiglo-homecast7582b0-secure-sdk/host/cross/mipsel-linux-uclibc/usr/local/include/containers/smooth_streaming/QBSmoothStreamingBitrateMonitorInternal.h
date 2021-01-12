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

#ifndef QBSMOOTHSTREAMINGBITRATEMONITOR_INTERNAL_H_
#define QBSMOOTHSTREAMINGBITRATEMONITOR_INTERNAL_H_

#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvTime.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvWeakList.h>

// It counts bitrate in few ranges. Each range is devided into subranges.
// Each range has weight which is in RANGE_WEIGHTS table.
#define NUMBER_OF_RANGES 3
#define RANGE_MS 3000 // duration of one range in ms
#define NUMBER_OF_SUBRANGES 30 // inside one range
const double RANGE_WEIGHTS[NUMBER_OF_RANGES] = { 0.1, 0.2, 0.7 }; // should be summed to 1.0

#define SUBRANGE_MS (RANGE_MS / NUMBER_OF_SUBRANGES)
#define NUMBER_OF_ALL_SUBRANGES (NUMBER_OF_RANGES * NUMBER_OF_SUBRANGES)


struct QBSmoothStreamingBitrateMonitor_ {
    struct SvObject_ super;

    // Contains bitrates for all subranges (in all ranges).
    unsigned long long sampleSizes[NUMBER_OF_ALL_SUBRANGES + 1]; // +1 for current subrange
    // Current position of sampleSizes table. Index where incoming number of bytes go to.
    int currentIndex;

    // Set to true when whole sampleSizes[] is filled.
    bool allSamplesReady;
    // True when anyone of downloaders is active (is not pasued), otherwise false.
    bool activeTimer;
    SvTime startTimeOfLastSample;
    // Used when bitrate monitor is paused (is not active).
    int totalDownloadMs;
    // Used to get information about download pauses.
    SvWeakList downloaders;
};

#endif /* QBSMOOTHSTREAMINGBITRATEMONITOR_INTERNAL_H_ */
