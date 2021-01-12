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

#ifndef FILE_MPEG_TIME_H
#define FILE_MPEG_TIME_H

typedef struct _mpeg_time mpeg_time;

#include "mpeg_common.h"

#define MPEG_TIME_MSB_MAX      0x00000001
#define MPEG_TIME_LSB_MAX      0xFFFFFFFF
#define MPEG_TIME_MSB_FACTOR  47721.858844444
#define MPEG_TIME_LSB_DIVIDER 90000.0

extern mpeg_time mpeg_time_q;
extern mpeg_time mpeg_time_h;
extern mpeg_time mpeg_time_m;
extern mpeg_time mpeg_time_z;

/**
 * Holds 33b time in standard MPEG 90KHz clock
 */

struct _mpeg_time
{
    int sign; // 0 not valid, other value denotes sign (-1,1)
    uint32_t msb;  // bit  32
    uint32_t lsb;  // bits 0..31
};

/***
 * General routines
 */

/**
 * Reset timestamp
 */
void mpeg_time_reset ( mpeg_time* ts );

/**
 * PES PTS/DTS style: 4 [3] 1 [15] 1 [15] 1 = 5 bytes
 */
int mpeg_time_parse ( mpeg_time* ts, uint8_t* ptr, uint8_t flags );

/**
 * Is timestamp valid
 */
int mpeg_time_valid ( mpeg_time* ts );

/**
 * Copy timestamp
 */
void mpeg_time_copy ( mpeg_time* dst, mpeg_time* src );

void mpeg_time_report ( mpeg_time* ts );

/**
 * Non-wrapping routines
 */

/**
 * check whether time stamps equal
 */
int mpeg_time_eq ( mpeg_time* a, mpeg_time* b );

/**
 * check whether a is less than b
 */
int mpeg_time_lt ( mpeg_time* a, mpeg_time* b );

/**
 * check whether a is less than or equal to b
 */
int mpeg_time_le ( mpeg_time* a, mpeg_time* b );

/**
 * check whether a is greater than b
 */
int mpeg_time_gt ( mpeg_time* a, mpeg_time* b );

/**
 * check whether a is greater than or equal to b
 */
int mpeg_time_ge ( mpeg_time* a, mpeg_time* b );

/**
 * subtract b from a; the result may be negative
 */
mpeg_time mpeg_time_sub ( mpeg_time* a, mpeg_time* b );

/**
 * Convert timestamp value to seconds
 */
double mpeg_time_to_seconds ( mpeg_time* a );

/**
 * Convert useconds to timestamp
 */
mpeg_time mpeg_time_from_usec ( uint32_t usec );

/**
 * increment a in place
 */
void mpeg_time_inc ( mpeg_time* a );

/**
 * decrement a in place
 */
void mpeg_time_dec ( mpeg_time* a );

/***
 * Wrapping routines
 */

/**
 * check whether a is less than b (b-range <= a < b)
 */
int mpeg_time_lt_range ( mpeg_time* a, mpeg_time* b, mpeg_time* range );

/**
 * check whether a is less than or equal to b (b-range <= a <= b)
 */
int mpeg_time_le_range ( mpeg_time* a, mpeg_time* b, mpeg_time* range );

/**
 * check whether a is greater than b (b < a <= b+range)
 */
int mpeg_time_gt_range ( mpeg_time* a, mpeg_time* b, mpeg_time* range );

/**
 * check whether a is greater than or equal to b (b <= a <= b+range)
 */
int mpeg_time_ge_range ( mpeg_time* a, mpeg_time* b, mpeg_time* range );

/**
 * add a to b in a cyclic manner
 */
mpeg_time mpeg_time_add ( mpeg_time* a, mpeg_time* b );



/**
 * calculate distance between two timestamps - the distance is always positive
 */
mpeg_time mpeg_time_distance ( mpeg_time* a, mpeg_time* b );

#endif
