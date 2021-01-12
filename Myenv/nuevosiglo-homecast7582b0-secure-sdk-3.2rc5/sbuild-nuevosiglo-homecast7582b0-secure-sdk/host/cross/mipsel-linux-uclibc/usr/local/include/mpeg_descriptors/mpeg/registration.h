/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef FILE_MPEG_REGISTRATION_DESC_H
#define FILE_MPEG_REGISTRATION_DESC_H

/**
 * @file registration.h Registration descriptor API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <mpeg_psi_parser.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup mpeg2 mpeg2: registration descriptor API
 * @ingroup MediaPlayer
 * @{
 *
 * Registration descriptor API is used to parse MPEG-2 Registration Descriptor
 * which (may) contain various kinds of private data depending on the context.
 * Currently this code extracts fourcc information for H.265 streams which may use
 * this descriptor in PMT in combination with stream_type == MPEG_STREAM_TYPE_PES_PRIVATE
 * (in contrast to just using stream_type = 0x24 (assigned to HEVC streams).
 **/

/**
 * Types of registration descriptor.
 **/
enum mpeg_registration_desc_type
{
    MPEG_ES_RT_UNKNOWN,
    MPEG_ES_RT_HEVC
};

typedef struct _mpeg_registration_desc mpeg_registration_desc;

/**
 * Output structure with parsed registration descriptor's data.
 **/
struct _mpeg_registration_desc
{
    /// Type
    enum mpeg_registration_desc_type type;
    /// FourCC code
    unsigned int fourcc;
};

/**
 * Parses descriptor.
 *
 * @param[out] reg  pointer to an initialized (usually static) mpeg_registration_desc structure
 * @param[in] data  pointer to raw descriptor data carried via mpeg_data structure
 * @return          returns -1 on failure and 0 on success. One needs to check mpeg_registration_desc.type to
 *                  understand was parsed out. Success roughly means that all pointers were OK and format
 *                  resembled that of registration descriptor.
 **/
int mpeg_registration_desc_init(mpeg_registration_desc* reg, mpeg_data* data);

/**
 * Destroys descriptor.
 *
 * @note This function is entirely without any effect. In other words it doesn't do anything and is defined
 *       only for compatibility.
 *
 * @param[in] reg     pointer to an initialized (may be static) mpeg_registration_desc structure
 **/
void mpeg_registration_desc_destroy(mpeg_registration_desc* reg);

/**
 * Logs the the structure of this descriptor to whatever log facility.
 *
 * @param[in] reg   pointer to an initialied (usually static) mepg_registration_desc structure
 * @param[in] indent_string indentation string that prefixes entire output (pretty-print purposes)
 **/
void mpeg_registration_desc_report(mpeg_registration_desc* reg, const char* indent_string);

/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif /* FILE_MPEG_REGISTRATION_DESC_H */

