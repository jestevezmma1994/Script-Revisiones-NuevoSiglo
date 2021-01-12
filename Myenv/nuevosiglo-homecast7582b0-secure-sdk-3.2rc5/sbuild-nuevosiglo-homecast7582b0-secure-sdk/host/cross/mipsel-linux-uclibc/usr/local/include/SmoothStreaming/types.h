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

#ifndef SMOOTH_STREAMING_TYPES_H_
#define SMOOTH_STREAMING_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvString.h>

#define DEFAULT_MSS_SCALE 10000000U

/**
 * @brief Representation of unsigned long long with additional validation bool member.
 * Used to match Microsoft's SmoothStreaming requirements of value representation in manifests.
 */
typedef struct uint64Validated_ {
    uint64_t value; /**< 64b unsigned integer value. */
    bool isValid;   /**< Indication if value is legal. */
} uint64Validated;

/**
 * Sets uint64Validated with the given values.
 * @param[out] obj object reference
 * @param[in] newValue new value unsigned long long value to set
 * @param[in] newValidationFlag new bool value to set
 */
extern void uint64ValidatedSet(uint64Validated* obj, uint64_t newValue, bool newValidationFlag);

typedef struct MSSQualityLevel_ {
    struct SvObject_ super_;
    int index;
    int bitrate;
    char fourCC[4];
    int maxWidth, maxHeight;
    SvString codecPrivateData;
    int nalUnitLengthField;
    int channels, samplingRate, bitsPerSample, packetSize;
    int audioTag;
} *MSSQualityLevel;

typedef struct MSSProtectionHeader_ {
    struct SvObject_ super_;
    SvString systemID;
    SvString data;
} *MSSProtectionHeader;

typedef struct MSSClip_ {
    struct SvObject_ super_;
    uint64Validated start, end;
    SvArray streamIndices;
    SvString url;
} *MSSClip;

typedef struct MSSMedia_ {
    struct SvObject_ super_;
    int minor, major;
    uint64Validated timeScale;
    double timeScaleTo90kFactor;
    uint64Validated duration, scale;
    uint64Validated dvrWindowLength;
    uint64Validated lookAheadFragmentCount;
    bool live;
    bool composite;
    SvArray clips;
    MSSProtectionHeader protectionHeader;
} *MSSMedia;

extern SvType
MSSProtectionHeader_getType(void);

extern SvType
MSSMedia_getType(void);

extern SvType
MSSClip_getType(void);

extern SvType
MSSQualityLevel_getType(void);

int
MSSClipMerge(MSSClip self, MSSClip other, unsigned int played_stream_index,
             int *done_chunks, SvErrorInfo *errorOut);

#endif
