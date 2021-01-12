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

#ifndef QB_CONTENT_PROTECTION_DESC_H
#define QB_CONTENT_PROTECTION_DESC_H

/**
 * @file SMP/mpeg2/include/mpeg_descriptors/dvb/qb_content_protection_desc.h
 * @brief Content protection descriptor
 **/

#include <stdint.h>
#include <mpeg_psi_parser.h>

typedef struct _qb_content_protection_desc qb_content_protection_desc;

/*
 * Information about enabled settings in descriptor
 */
typedef enum {
    /* Macrovision protections with custom cpc value **/
    qb_content_protection_enabled_macrovision_cpc = 0x0001,
    qb_content_protection_enabled_reserved1 = 0x0002,
    qb_content_protection_enabled_reserved2 = 0x0004,
    qb_content_protection_enabled_reserved3 = 0x0008,
    qb_content_protection_enabled_reserved4 = 0x0010,
    qb_content_protection_enabled_reserved5 = 0x0020,
    qb_content_protection_enabled_reserved6 = 0x0040,
    qb_content_protection_enabled_reserved7 = 0x0080,
    qb_content_protection_enabled_reserved8 = 0x0100,
    qb_content_protection_enabled_reserved9 = 0x0200,
    qb_content_protection_enabled_reserved10 = 0x0400,
    qb_content_protection_enabled_reserved11 = 0x0800,
    qb_content_protection_enabled_reserved12 = 0x1000,
    qb_content_protection_enabled_reserved13 = 0x2000,
    qb_content_protection_enabled_reserved14 = 0x4000,
    qb_content_protection_enabled_reserved15 = 0x8000
} qb_content_protection_enabled;

/*
 * Information about content protection descriptor
 */
struct _qb_content_protection_desc
{
  /** Each bit provides information about enabled settings (each bit description in qb_content_protection_enabled) */
  uint16_t enabled;
  /** CPC value (used in macrovision_cpc setting type) */
  uint8_t macrovision_cpc;
};

/**
 * Initialize content protection descriptor
 *
 * @param[in] cpd handle to descriptor
 * @param[in] data raw binary descriptor data
 * @return 0 on success, value < 0 on error
 */
int qb_content_protection_desc_init(qb_content_protection_desc* cpd, mpeg_data* data);
/**
 * Destorys content protection descriptor
 *
 * @param[in] cpd handle to descriptor
 */
void qb_content_protection_desc_destroy(qb_content_protection_desc* cpd);
/**
 * Reports information about contnet descriptor
 *
 * @param[in] cpd handle to descriptor
 * @param[in] indent_string string printed before descripotr information
 *
 */
void qb_content_protection_desc_report(const qb_content_protection_desc* cpd, const char* indent_string);

#endif // QB_CONTENT_PROTECTION_DESC_H
