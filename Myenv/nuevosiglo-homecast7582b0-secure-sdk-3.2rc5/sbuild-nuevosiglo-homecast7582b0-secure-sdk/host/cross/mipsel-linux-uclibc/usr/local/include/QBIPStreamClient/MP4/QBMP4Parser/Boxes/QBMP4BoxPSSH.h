/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_MP4_BOX_PSSH_
#define QB_MP4_BOX_PSSH_

/**
 * @file QBMP4BoxPSSH.h
 * @brief MP4 Protection System Specific Header Box
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBMP4BoxPSSH MP4 Protection System Specific Header Box
 * @ingroup QBMP4Box
 * @{
 **/

#include <QBIPStreamClient/MP4/QBMP4Parser/QBMP4Parser.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct QBMp4BoxPSSH_ {
    /** UUID that uniquely identifies the content protection system that this header belongs to. */
    uint8_t systemID[16];
    /** number of KID entries in the following table */
    uint32_t kidCount;
    /** identifies a key identifier that the Data field applies to. */
    uint8_t **kid;
    /** size in bytes of the Data member */
    uint32_t dataSize;
    /** holds the content protection system specific data. */
    uint8_t *data;
} *QBMp4BoxPSSH;

/**
 * Alloc new MP4 Protection System Specific Header Box
 * @return handle to MP4 Protection System Specific Header Box
 */
QBMp4BoxPSSH QBMp4BoxPSSHAlloc(void);
/**
 * Destroy MP4 Protection System Specific Header Box
 * @param[in] self handle to MP4 Protection System Specific Header Box
 */
void QBMp4BoxPSSHDestroy(QBMp4BoxPSSH self);

/**
 * Fill MP4 Protection System Specific Header Box with parsed data
 *
 * @param[in] self handle to MP4 Protection System Specific Header Box
 * @param[in] version Full Box version
 * @param[in] buf data buffer to parse
 * @param[in] size size of data buffer
 * @return parser code
 */
QBMp4ParserCode QBMp4BoxPSSHParse(QBMp4BoxPSSH self, uint8_t version, uint8_t *buf, uint32_t size);

#ifdef __cplusplus
}
#endif

/**
 * @}
 **/

#endif // #ifndef QB_MP4_BOX_PSSH_
