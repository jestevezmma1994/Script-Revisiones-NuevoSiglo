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

#ifndef STORAGE_INPUT_BLOCK_DECRYPTOR_H
#define STORAGE_INPUT_BLOCK_DECRYPTOR_H

#include "storage_input.h"

#include <fibers/c/fibers.h>
#include <SvPlayerKit/SvDRMInfo.h>
#include <SvFoundation/SvObject.h>
#include <stdint.h>

/**
 * @file storage_input_block_decryptor.h storage input block decryptor class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup StorageInputBlockDecryptor Storage input block decryptor
 * This class supports decryption of content encrypted at file level. It has to be used if content can not be parsed
 * before its decryption.
 * @ingroup autoplayer
 * @{
 **/

/**
 * @brief storage_input_block_decryptor_params structure
 */
struct storage_input_block_decryptor_params {
    SvScheduler scheduler; /**< scheduler */
    struct storage_input* sub_input; /**< sub storage input */
    SvObject decryptionPlugin; /**< Block decryption plugin */
    SvDRMInfo drmInfo; /**< DRM info with all required information for playback */
    int64_t blockSize; /**< This is size of data block used for content decryption. In case of 0 and negative value default value is used. @see storage_input.h */
};

/**
 * @brief Create storage_input_block_decryptor instance
 * @param[in] params init params
 * @param[out] out created storage_input instance
 * @return @c 0 if block decryptor has been successfully created, negative value otherwise
 */
int storage_input_block_decryptor_create(const struct storage_input_block_decryptor_params* params, struct storage_input** out);

/**
* @}
*/

#endif // #ifndef STORAGE_INPUT_BLOCK_DECRYPTOR_H
