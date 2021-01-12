/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SMOOTH_STREAMING_STREAM_INDEX_H_
#define SMOOTH_STREAMING_STREAM_INDEX_H_

/**
 * @file stream_index.h Manage streams
 **/

#include <stdint.h>

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>

#include <SmoothStreaming/types.h>

//the tolerance applied to DVR window. If the index has more chunks than
//the DVR window allows, then those chunks will be dropped only when those
//chunks exceed the window with the tolerance applied
#define STREAM_INDEX_DVR_WINDOW_TOLERANCE 1.5

typedef struct MSSChunk_ {
    uint64Validated start, duration;
} *MSSChunk;

typedef struct MSSChunkSet_ {
    struct MSSChunk_ first;
    size_t count;
} *MSSChunkSet;

typedef enum {
    MSSStreamIndexType_unknown = 0,
    MSSStreamIndexType_video,
    MSSStreamIndexType_audio,
    MSSStreamIndexType_text
} MSSStreamIndexType;

typedef struct MSSStreamIndex_ {
    struct SvObject_ super_;

    MSSStreamIndexType type;
    int qualityLevelsCount;
    int maxWidth, maxHeight;
    int displayWidth, displayHeight;
    int channels, samplingRate, bitsPerSample, packetSize;
    uint64Validated timeScale;
    double timeScaleTo90kFactor;
    uint64Validated calculatedDuration;

    SvString name;
    SvString language;
    SvString subType;
    SvString url;
} *MSSStreamIndex;

typedef struct MSSStreamIndexObj_ *MSSStreamIndexObj;

struct MSSStreamIndexObj_ {
    /** superclass object */
    struct MSSStreamIndex_ super_;
    /** flag indicating if quality levels are sorted by bitrate */
    bool sorted;
    /** array of quality levels for this stream index */
    SvArray qualityLevels;
    /** length of DVR window */
    uint64Validated dvrWindowLength;
    /** end time of chunk that was not added to the index because it was before DVR window */
    uint64Validated lastTrimmedEnd;
    /** current capacity of an index */
    size_t currentCapacity;
    /** index of first element in chunks cyclic buffer */
    size_t firstSet;
    /** number of elements in chunks cyclic buffer */
    size_t setCount;
    /** number of all the chunks in the index (all sets times their count) */
    size_t chunksCount;
    /** cyclic buffer storing chunk sets */
    MSSChunkSet chunks;
};

extern SvType
MSSStreamIndex_getType(void);

extern MSSStreamIndex
MSSStreamIndexInit(MSSStreamIndex self, size_t initialFragments, size_t initialQualLevels);

extern void
MSSStreamIndexSetDVRWindowLength(MSSStreamIndex self, uint64Validated dvrWindowLength);

extern size_t
MSSStreamIndexFragmentsCount(MSSStreamIndex self);

extern size_t
MSSStreamIndexQualityLevelsCount(MSSStreamIndex self);

/**
 * Returns start of the stream.
 *
 * @param[in] streamIndex
 * @return start of the stream
 **/
extern int64_t
MSSStreamIndexGetStreamStartTime(MSSStreamIndex streamIndex);

extern void
MSSStreamIndexAddQualityLevel(MSSStreamIndex self, MSSQualityLevel qualLevel);

extern MSSQualityLevel
MSSStreamIndexGetQualityLevel(MSSStreamIndex self, size_t index);

/**
 * Adds the fragment information to the index.
 * This function works in two modes - manual and automatic. The automatic mode is designed for
 * fragments extracted from manifest (first download or manifest refresh). The manual mode is reserved
 * for fragment information comming from previous fragment (each fragment have information on following fragment).
 * In automatic mode all fragments following the one that is added with this function will be dropped
 * because the manifest will have newer information on those fragments. In Manual mode the following fragments (if any)
 * will not be dropped, because we are not getting info on them untill we download their preceidnig fragments.
 *
 * @param[in] self          this object
 * @param[in] start         start time of added fragment
 * @param[in] duration      duration of added fragment
 * @param[in] count         number of same duration consecutive fragments. (Calling this method with value greater than 1
 *                          is like calling it multiple times with same duration value and start value enlarged each time by duration)
 * @param[in] manualMode    switch for changing between manual and automatic mode. @c true for manual mode, @c false from automatic
 * @return Number of fragments in the index after this fragment info is added
 **/
extern int
MSSStreamIndexAddFragments(MSSStreamIndex self, uint64Validated start, uint64Validated duration, size_t count, bool manualMode);

extern int
MSSStreamIndexGetFragment(MSSStreamIndex self, size_t index, MSSChunk chunkOut);

typedef struct MSSStreamIndexIterator_ *MSSStreamIndexIterator;

extern MSSStreamIndexIterator
MSSStreamIndexCreateIterator(MSSStreamIndex self);

extern MSSStreamIndexIterator
MSSStreamIndexCreateReverseIterator(MSSStreamIndex self);

/**
 * Dumps the iterator content to screen.
 * Prints all the fragments contained in the index marking the one that the itarator is pointing to.
 *
 * @param[in] self      this object
 */
extern void
MSSStreamIndexIteratorLog(MSSStreamIndexIterator self);

extern int
MSSStreamIndexIteratorGetNext(MSSStreamIndexIterator self, MSSChunk chunkOut);

extern int
MSSStreamIndexIteratorPeekNext(MSSStreamIndexIterator self, MSSChunk chunkOut);

extern int
MSSStreamIndexIteratorGetPrev(MSSStreamIndexIterator self, MSSChunk chunkOut);

extern void
MSSStreamIndexIteratorSetDirection(MSSStreamIndexIterator self, bool reverse);

extern size_t
MSSStreamIndexIteratorGetPosition(MSSStreamIndexIterator self);

extern void
MSSStreamIndexReinitialize(MSSStreamIndex streamIndex);

/**
 * Dumps the index data to screen.
 * This function prints the internal structure of the index.
 * The index start and end times gets printed as well as the fragmentation details (consecutive fragments counts,
 * all fragments counts, and all sets counts).
 * In case there are any discontinuities in the index
 * (previous chunk end is less then current chunk start) they will be marked
 * with a tag \verbatim#D#\endverbatim followed by discontinuity dur:wation.
 * In case there are any overlaps in the index
 * (current chunk start is less than previous chunk end) they will be marked
 * with a tag \verbatim@R@\endverbatim followed by the overlap duration.
 *
 * @param[in] streamIndex      this object
 * @param[in] prefix           prefix which will be added to log
 */
extern void
MSSStreamIndexLogIndexData(MSSStreamIndex streamIndex, const char* prefix);

/**
 * @}
 **/

#endif
