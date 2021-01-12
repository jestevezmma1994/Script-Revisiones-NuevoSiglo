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

#ifndef MPEG_QUIRKS_H
#define MPEG_QUIRKS_H

#include <stdint.h>
#include <stddef.h>
#include <mpeg_psi_parser.h>
#include <mpeg_descriptors/dvb/logical_channel_number.h>
#include <QBPCRatings/QBPCList.h>

#ifdef __cplusplus
extern "C" {
#endif


uint8_t mpeg_quirks_translate_stream_type(uint8_t type) __attribute__((weak));
void mpeg_eit_parser_pc_rating(int rate, const char *countryCode, QBPCList rating)  __attribute__((weak));
void mpeg_eit_parser_pc_rating_from_descriptor_map(mpeg_descriptor_map *descriptor_map, QBPCList rating)  __attribute__((weak));
/**
 * Get client specific DVB descriptor tag associated to qb content protection descriptor.
 *
 * @return DVB descriptor tag of qb content protection descriptor.
 **/
int qb_content_protection_get_descriptor_tag(void) __attribute__((weak));

/**
 * Get client specific DVB descriptor tag associated to qb mrp descriptor.
 *
 * @return DVB descriptor tag of qb mrp descriptor.
 **/
int qb_mrp_get_descriptor_tag(void) __attribute__((weak));

/**
 * Decode text string from DVB-specific format (see ETSI EN 300 468, Annex A)
 * to NULL-terminated UTF-8 string.
 *
 * @note This function, if defined, is used instead of utfconv_mpeg_to_utf8().
 *
 * @param[in] src       text string in DVB-specific format
 * @param[in] len       length of @a src in bytes
 * @return              newly allocated NULL-terminated string in UTF-8 encoding
 *                      (caller is responsible for freeing it), @c NULL in case of error
 **/
unsigned char *mpeg_quirks_decode_dvb_string(const unsigned char *src, size_t len) __attribute__((weak));

/**
 * Get Logical channel number entry from recieved LCN data.
 * Default implemetation assumes that reserved bits from positions 17 - 21
 * are filled with '1' values, which is not true in all cases. For some more details see [jira CUB-3364].
 *
 * @note This function, if defined, is called inside of mpeg_lcn_desc_get_entry().
 *
 * @param[in]  desc     struct containing unparsed mpeg data
 * @param[in]  i        number of channel to be parsed
 * @param[out] out      filled structure contatining channel data
 */
void mpeg_quirks_lcn_desc_get_entry(const mpeg_lcn_desc* desc, int i, struct mpeg_lcn_desc_entry* out) __attribute__((weak));

#ifdef __cplusplus
}
#endif

#endif // MPEG_QUIRKS_H
