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

#ifndef FILE_MPEG_COMMON_H
#define FILE_MPEG_COMMON_H

/**
 * @file mpeg_common.h Definitions of constants used in MPEG library
 **/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mpeg_tables.h"
#include "bits32.h"


/**
 * @defgroup mpeg_common Constants defined by MPEG and DVB standards
 * @ingroup mpeg
 * @{
 **/


/////////////////////////////////////////////
// Elementary stream types
#define MPEG_STREAM_TYPE_MPEG1_VIDEO     0x01
#define MPEG_STREAM_TYPE_MPEG2_VIDEO     0x02
#define MPEG_STREAM_TYPE_MPEG1_AUDIO     0x03
#define MPEG_STREAM_TYPE_MPEG2_AUDIO     0x04

#define MPEG_STREAM_TYPE_PES_PRIVATE     0x06

#define MPEG_STREAM_TYPE_MPEG4_AUDIO     0x11
#define MPEG_STREAM_TYPE_MPEG4_GENERIC   0x12
#define MPEG_STREAM_TYPE_ADTS_AUDIO      0x0F
#define MPEG_STREAM_TYPE_MPEG4_VIDEO     0x10
#define MPEG_STREAM_TYPE_H264_VIDEO      0x1B
#define MPEG_STREAM_TYPE_H265_VIDEO      0x24

// metadata stream types, ISO/IEC 13818-1:2000/Amendment 3 (2003)
#define MPEG_STREAM_TYPE_META_PES        0x15
#define MPEG_STREAM_TYPE_META_SECTIONS   0x16
#define MPEG_STREAM_TYPE_META_DATA_CAROUSEL 0x17
#define MPEG_STREAM_TYPE_META_OBJECT_CAROUSEL 0x18
#define MPEG_STREAM_TYPE_META_SDP        0x19

// A/52 = AC3 in ATSC
#define MPEG_STREAM_TYPE_A52_AUDIO       0x81

#define MPEG_STREAM_TYPE_SCTE27_SUBTITLES 0x82
#define MPEG_STREAM_TYPE_SPU_SUBTITLES   0x82
#define MPEG_STREAM_TYPE_LPCM_AUDIO      0x83
#define MPEG_STREAM_TYPE_SDDS_AUDIO      0x84
#define MPEG_STREAM_TYPE_DTS_AUDIO       0x85
#define MPEG_STREAM_TYPE_A52B_AUDIO      0x91
#define MPEG_STREAM_TYPE_SPUB_SUBTITLES  0x92
#define MPEG_STREAM_TYPE_LPCMB_AUDIO     0x93
#define MPEG_STREAM_TYPE_SDDSB_AUDIO     0x94
#define MPEG_STREAM_TYPE_MICROSOFT_VIDEO 0xA0
#define MPEG_STREAM_TYPE_EXTENDED        0xEA
/////////////////////////////////////////////


/////////////////////////////////////////////
// codec consts
#define MPEG_AUDIO_FORMAT_UNKNOWN   0x00
#define MPEG_AUDIO_FORMAT_MP2       0x50
#define MPEG_AUDIO_FORMAT_MP3       0x55

#define MPEG_AUDIO_FORMAT_AAC       0xFF
#define MPEG_AUDIO_FORMAT_WMA       0x0160
#define MPEG_AUDIO_FORMAT_WMA2      0x0161
#define MPEG_AUDIO_FORMAT_WMAP      0x0162

#define MPEG_AUDIO_FORMAT_AC3_SPDIF 0x92

// fourcc EAC3
#define MPEG_AUDIO_FORMAT_EAC3      0x33434145

// FIXME - this value is for SIGMA only
//#define MPEG_AUDIO_FORMAT_ADTS      0x6134706d

// TODO - perhaps for SIGMA it should be different ???
//#define MPEG_AUDIO_FORMAT_AAC_ENV   0x1001

// fourcc MP4A
#define MPEG_AUDIO_FORMAT_MP4A      0x4134504d

// fourcc AACH
#define MPEG_AUDIO_FORMAT_AAC_HE    0x48434141

#define MPEG_AUDIO_FORMAT_DTS       0x535444

/////////////////////////////////////////////


/// Value of synchronization byte: first byte of every MPEG-TS packet.
#define MPEG_TS_SYNC_BYTE           0x47
/// Length of MPEG-TS packet header in bytes.
#define MPEG_TS_HEADER_LENGTH       4
/// Total length of MPEG-TS packet in bytes.
#define MPEG_TS_PACKET_LENGTH       188

#define MPEG_TS_NOT_SCRAMBLED  0x00

#define MPEG_TS_ADAPT_RESERVED 0x00
#define MPEG_TS_ADAPT_PAYLOAD  0x01
#define MPEG_TS_ADAPT_ONLY     0x02
#define MPEG_TS_ADAPT_BOTH     0x03

#define MPEG_TS_SECTION_POINTER_LENGTH  1
/////////////////////////////////////////////


/////////////////////////////////////////////
// program_number consts
#define MPEG_PROGRAM_NUMBER_NIT         0
#define MPEG_PROGRAM_NUMBER_INVALID     (~0U)

#define MPEG_SID_INVALID                (~0U)
#define MPEG_TSID_INVALID               (~0U)
#define MPEG_ONID_INVALID               (~0U)
#define MPEG_BOUQUETID_INVALID          (~0U)
/////////////////////////////////////////////


/////////////////////////////////////////////
// descriptors consts
#define MPEG_DESC_TAG_NUMBER          256
#define MPEG_DESC_MIN_LENGTH          2
/////////////////////////////////////////////

/////////////////////////////////////////////
// PSI / sections consts

// first 3 bytes - table_id, indicators, section_length
#define MPEG_PRIVATE_SECTION_HEADER_LENGTH      3

// 8 bytes - table_id, ..., last_section_number
#define MPEG_SECTION_SYNTAX_HEADER_LENGTH       8

#define MPEG_SECTION_MAX_LENGTH                 4096
#define MPEG_SECTION_CRC_32_LENGTH              4

#define MPEG_SECTION_MAX_NUMBER_PER_TABLE       256

#define MPEG_VERSION_NUMBER_INVALID             (~0U)

#define MPEG_PAT_ELEMENT_LENGTH                 4

#define MPEG_PMT_ELEMENT_MIN_LENGTH             5
#define MPEG_PMT_ELEMENT_DESC_LOOP_BIT_OFFSET   (3*8+4)
#define MPEG_PMT_PROGRAM_INFO_HEADER_LENGTH     4

#define MPEG_SDT_ELEMENT_MIN_LENGTH             5
#define MPEG_SDT_ELEMENT_DESC_LOOP_BIT_OFFSET   (3*8+4)
#define MPEG_SDT_SERVICE_LIST_HEADER_LENGTH     3

#define MPEG_EIT_ELEMENT_MIN_LENGTH             12
#define MPEG_EIT_ELEMENT_DESC_LOOP_BIT_OFFSET   (10*8+4)
#define MPEG_EIT_MAIN_LOOP_HEADER_LENGTH        6
/////////////////////////////////////////////

/////////////////////////////////////////////
// running_status - acc. to ETSI 300-468 tab. 6, p. 21
#define MPEG_RUNNING_STATUS_UNDEFINED       0
#define MPEG_RUNNING_STATUS_NOT_RUNNING     1
#define MPEG_RUNNING_STATUS_STARTS_SOON     2
#define MPEG_RUNNING_STATUS_PAUSING         3
#define MPEG_RUNNING_STATUS_RUNNING         4

/////////////////////////////////////////////

/////////////////////////////////////////////
// PES consts
#define MPEG_PES_START_CODE_LENGTH      3
#define MPEG_PES_STREAM_ID_LENGTH       1
#define MPEG_PES_MAX_LENGTH             (512*1024)
#define MPEG_PES_HEADER_LENGTH          6

#define MPEG_ES_START_CODE_LENGHT       4
/////////////////////////////////////////////


/////////////////////////////////////////////
// status codes
#define MPEG_OK             0

// end of buffer
#define MPEG_EOB            1

#define MPEG_ETIMEOUT     -10
#define MPEG_ESCRAMBLED   -11
#define MPEG_EBITERROR    -12
#define MPEG_EPSI         -13
#define MPEG_ENOTFOUND    -14
#define MPEG_ENOTSUPP     -15
#define MPEG_EPES         -16
#define MPEG_EPACKETLOST  -17
#define MPEG_INVALID_TYPE -18
#define MPEG_NOT_FOUND    -19
#define MPEG_EGENERIC     -99
/////////////////////////////////////////////

/**
 * @}
 **/


#endif
