/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef FILE_MPEG_PCR_H
#define FILE_MPEG_PCR_H

#include "mpeg_time.h"
#include "mpeg_common.h"
#include <limits.h> // for ULLONG_MAX

// 42bit 27Mhz clock
typedef unsigned long long mpeg_pcr;

#define MPEG_PCR_MAX  2576980377600ULL
#define MPEG_PCR_INV  ULLONG_MAX

#define MPEG_JITTER_MAX  48600000000LL

/***
 * General routines
 */

/**
 * TS PCR/OPCR style: [33] 6 [9] = 6 bytes */
int mpeg_pcr_parse(mpeg_pcr* pcr, const uint8_t* ptr);

/**
 * Reset PCR */
static inline __attribute__((always_inline))
void mpeg_pcr_reset(mpeg_pcr* pcr)
{
    *pcr = MPEG_PCR_INV;
}

/**
 * Is PCR valid */
int mpeg_pcr_valid(mpeg_pcr pcr);

/**
 * Copy PCR value */
mpeg_pcr mpeg_pcr_copy(mpeg_pcr pcr);


/***
 * Non-wrapping routines
 */

/**
 * Convert useconds to PCR */
mpeg_pcr mpeg_pcr_from_usec(unsigned long long usec);

/**
 * Convert pcr value to seconds */
double mpeg_pcr_to_seconds(mpeg_pcr a);

/***
 * Wrapping routines (wraps around MPEG_PCR_MAX)
 */

/**
 * check whether a is less than b (b-range <= a < b) */
int mpeg_pcr_lt_range(mpeg_pcr a, mpeg_pcr b, mpeg_pcr range);

/**
 * check whether a is less than or equal to b (b-range <= a <= b) */
int mpeg_pcr_le_range(mpeg_pcr a, mpeg_pcr b, mpeg_pcr range);

/**
 * check whether a is greater than b (b < a <= b+range) */
int mpeg_pcr_gt_range(mpeg_pcr a, mpeg_pcr b, mpeg_pcr range);

/**
 * check whether a is greater than or equal to b (b <= a <= b+range) */
int mpeg_pcr_ge_range(mpeg_pcr a, mpeg_pcr b, mpeg_pcr range);

/**
 * Calculate distance between a and b (in cyclic manner) */
mpeg_pcr mpeg_pcr_distance(mpeg_pcr a, mpeg_pcr b);

/**
 * subtract b from a; the result is negative if a <= b <= a+range,
 * positive if b <= a <= b+range and invalid otherwise */
long long mpeg_pcr_sub_range(mpeg_pcr a, mpeg_pcr b, mpeg_pcr range);

#endif
