/*****************************************************************************
**
** Copyright (C) 2015 Cubiware Sp. z o.o. All rights reserved.
**
** This file is dual-licensed: Cubiware License 1.0 or GPLv2. Please read the
** respective license statements below. You are only allowed to use this code
** with Cubiware proprietary software or third-party proprietary software when 
** choosing Cubiware License 1.0., otherwise you may use GPLv2 license.
**
** Cubiware Sp. z o.o. Software License Version 1.0
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
**
**  GPLv2 License:
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License along
**  with this program; if not, write to the Free Software Foundation, Inc.,
**  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**
******************************************************************************/

#ifndef QB_BINARY_IMAGE_H
#define QB_BINARY_IMAGE_H

/**
 * @file QBBinaryImage.h API for QBBinaryImage manipulation
 **/

#ifdef _CFE_
#include "lib_types.h"
#else
#include <stdint.h>
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
/* little endian */
/** Converts endianness from QBBinaryImage to host */
#define QB_IMAGE_CONVERT_ENDIAN(x)      (x)
#else
/* big endian */
/** Converts endianness from QBBinaryImage to host */
#define QB_IMAGE_CONVERT_ENDIAN(x)      (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
#endif

#define QB_IMAGE_VERSION_DEF                     2
#define QB_IMAGE_VERSION                         QB_IMAGE_CONVERT_ENDIAN((uint32_t) QB_IMAGE_VERSION_DEF)

/* magic values */
/** Magic number for static QBBinaryImages */
#define QB_IMAGE_MAGIC_QBSH                      QB_IMAGE_CONVERT_ENDIAN((uint32_t) 0x48534251)
/** Magic number for dynamic QBBinaryImages */
#define QB_IMAGE_MAGIC_QBDH                      QB_IMAGE_CONVERT_ENDIAN((uint32_t) 0x48444251)

/* verification modes */
/** Verification using RSA-2048 and SHA-256 */
#define QB_IMAGE_VERIFICATION_RSA2048_SHA256     QB_IMAGE_CONVERT_ENDIAN((uint32_t) 0x01)
/** Verification using SHA-256 based HMAC */
#define QB_IMAGE_VERIFICATION_HMAC256_SHA256     QB_IMAGE_CONVERT_ENDIAN((uint32_t) 0x80)

/* encryption modes */
/** Encryption using AES-128 CBC (not-supported) */
#define QB_IMAGE_ENCRYPTION_AES128CBC            QB_IMAGE_CONVERT_ENDIAN((uint32_t) 0x01)
/** Encryption using AES-256 CBC */
#define QB_IMAGE_ENCRYPTION_AES256CBC            QB_IMAGE_CONVERT_ENDIAN((uint32_t) 0x02)
/** Encryption using AES-256 CBC with key chaining */
#define QB_IMAGE_ENCRYPTION_AES256CBC_KEYCHAIN   QB_IMAGE_CONVERT_ENDIAN((uint32_t) 0x03)

/** Header for QBBinaryImage
 *
 * Each QBBinaryImage consists of QBImageHeader and payload data.
 * The header contains all "non-secret" data needed to verify and decrypt the payload.
 * Header is partially encrypted
*/
typedef struct
__attribute__((__packed__))
QBImageHeader_ {
    /** Data that needs to be encrypted for storage */
    struct __attribute__((__packed__)) encryptedData_ {
        /** magic value, always one of the following strings
         * @c "QBSH" -- static content verified using RSA
         * @c "QBDH" -- dynamic content verified using HMAC
         */
        uint32_t magic;
        /** Version of the header and included security. */
        uint32_t version;
        /** Size of the contained image (payload) */
        uint32_t imageSize;
        /** Verification mode used for the header and image */
        uint32_t verificationMode;
        /** Reserved for future use, pads "general" information to 128 bytes*/
        uint8_t reserved1[112];


        /** Encryption mode used for the image */
        uint32_t encryptionMode;
        /** Initialization vector for image decryption (if required by encryption
         * mode, otherwise filled with random bytes) */
        uint8_t imageIV[16];
        /** Encrypted random key (keychain modes) for the image (if required by
         * encryption mode, otherwise filled with random bytes) */
        uint8_t encryptedKey[32];
        /** Initialization vector for encrypted key (if required by encryption
         * mode, otherwise filled with random bytes) */
        uint8_t encryptedKeyIV[16];
        /** Reserved for future use, pads "encryption" information to 128 bytes */
        uint8_t reserved2[60];

        /** Verification data for the image */
        union {
            /** RSA signature (for QBSH) of the image */
            uint8_t signature[1024];
            /** HMAC (for QBDH) of the image */
            uint8_t HMAC[32];
        } imageSignature;
    } encryptedData;

    /** Data that does not need to be encrypted for storage */
    struct __attribute__((__packed__)) plainData_ {
        /* Initialization vector for the header,
           bytes 0x0000-0x04ff of header are encrypted*/
        uint8_t headerIV[16];
        /* Reserved for future use, pads plain data to 128 bytes*/
        uint8_t reserved3[112];
    } plainData;

    /** Verification data for the header */
    union {
        /** RSA signature (for QBSH) of header bytes 0x0000-0x057f */
        uint8_t signature[1024];
        /** HMAC (for QBDH) of header bytes 0x0000-0x057f */
        uint8_t HMAC[32];
    } headerSignature;
} QBImageHeader;

/**
 * Size of header for sanity checking
 **/
#define QB_IMAGE_HEADER_SIZE (128 + 128 + 1024 + 128 + 1024)

/**
 * This is a dummy function that generates compile-time error if there is something wrong with header padding
 **/
static inline void There_Is_A_Problem_With_Header_Padding(void)
{
    switch (0) {
    case 0:
    case sizeof(QBImageHeader) == QB_IMAGE_HEADER_SIZE:
        break;
    }
}

/** Context for cryptographic operations */
typedef struct QBImageContext_ *QBImageContext;

/**
 * Generate sequence of random numbers.
 *
 * @param[in/out] buffer  preallocated buffer for random numbers
 * @param[in] length      length of the sequence to generate (@a buffer must be at least this size)
 *
 * @return                @c 0 on success, @c -1 in case of error
 **/
int QBImageRand(void *buffer, size_t length);

/**
 * Create @ref QBImageContext for verification/encryption operations.
 *
 * @return                @c QBImageContext on success, @c NULL in case of error
 **/
QBImageContext QBImageContextCreate(void);

/**
 * Destroy @ref QBImageContext.
 *
 * @param[in] ctx      @ref QBImageContext to destroy
 **/
void QBImageContextDestroy(QBImageContext ctx);

/**
 * Extract (verify and then decrypt relevant data) static @ref QBImageHeader from given buffer.
 *
 * @param[in] encryptedHeader  buffer containing encrypted header
 * @param[in] aesKey           key used for decryption
 * @param[in] rsaN             RSA public modulus used for verification
 * @param[in] rsaNSize         size of @a rsaN
 * @param[in] rsaN             RSA public exponent used for verification
 * @param[in] rsaNSize         size of @a rsaE
 * @param[in,out] qbHeader     preallocated buffer for verified and decrypted header
 *
 * @return                     @c 0 on success, @c < 0 in case of error
 **/
int QBImageStaticGetVerifiedHeader(const QBImageHeader *encryptedHeader,
                                   const unsigned char aesKey[32],
                                   const void *rsaN, const size_t rsaNSize,
                                   const void *rsaE, const size_t rsaESize,
                                   QBImageHeader *qbHeader);

/**
 * Setup verification of static QBBinaryImage payload.
 *
 * @param[in,out] ctx          @ref QBImageContext to initialize
 * @param[in] rsaN             RSA public modulus used for verification
 * @param[in] rsaNSize         size of @a rsaN
 * @param[in] rsaN             RSA public exponent used for verification
 * @param[in] rsaNSize         size of @a rsaE
 *
 * @return                     @c 0 on success, @c < 0 in case of error
 **/
int QBImageStaticVerifyStart(QBImageContext ctx,
                             const void *rsaN, const size_t rsaNSize,
                             const void *rsaE, const size_t rsaESize);

/**
 * Process chunk of static QBBinaryImage payload.
 *
 * @param[in,out] ctx          @ref QBImageContext to update
 * @param[in] buffer           buffer containing chunk of QBBinaryImage payload
 * @param[in] length           length of the chunk
 **/
void QBImageStaticVerifyUpdate(QBImageContext ctx, const void *buffer, const size_t length);

/**
 * Process final steps of static QBBinaryImage payload verification.
 *
 * @param[in,out] ctx          @ref QBImageContext used for processing
 * @param[in] qbHeader         @ref QBImageHeader containing signature data
 *
 * @return                     @c 0 on success, @c < 0 in case of error/invalid signature
 **/
int QBImageStaticVerifyFinish(QBImageContext ctx, const QBImageHeader *qbHeader);

/**
 * Create unsigned static QBBinaryImage.
 *
 * @param[in] payload                buffer containing payload for the image
 * @param[in] payloadSize            size of the payload for the image
 * @param[in] aesKey                 key used for encryption
 * @param[in,out] qbImageBuffer      preallocated buffer for QBBinaryImage
 * @param[in,out] qbImageBufferSize  @a in size of the buffer, @a out size of the QBBinaryImage
 *
 * @return                           @c 0 on success, @c < 0 in case of error
 **/
int QBImageStaticCreateUnsigned(const void *payload, const size_t payloadSize,
                                const unsigned char aesKey[32],
                                void *qbImageBuffer, size_t *qbImageBufferSize);

/**
 * Extract (verify and then decrypt relevant data) dynamic @ref QBImageHeader from given buffer.
 *
 * @param[in] encryptedHeader     buffer containing encrypted header
 * @param[in] hmacKey             key used for HMAC verification
 * @param[in] aesKey              key used for decryption
 * @param[in] additionalData      data appended to HMAC calculations
 * @param[in] additionalDataSize  size of @a additionalData
 * @param[in,out] qbHeader        preallocated buffer for verified and decrypted header
 *
 * @return                        @c 0 on success, @c < 0 in case of error
 **/
int QBImageDynamicGetVerifiedHeader(const QBImageHeader *encryptedHeader,
                                    const unsigned char hmacKey[32],
                                    const unsigned char aesKey[32],
                                    const void *additionalData, const size_t additionalDataSize,
                                    QBImageHeader *qbHeader);

/**
 * Setup verification of dynamic QBBinaryImage payload.
 *
 * @param[in,out] ctx          @ref QBImageContext to initialize
 * @param[in] hmacKey          key used for HMAC verification
 **/
void QBImageDynamicVerifyStart(QBImageContext ctx, const unsigned char hmacKey[32]);

/**
 * Process chunk of dynamic QBBinaryImage payload for verification.
 *
 * @param[in,out] ctx          @ref QBImageContext to update
 * @param[in] buffer           buffer containing chunk of QBBinaryImage payload
 * @param[in] length           length of the chunk
 **/
void QBImageDynamicVerifyUpdate(QBImageContext ctx, const void *buffer, const size_t length);

/**
 * Process final steps of static QBBinaryImage payload verification.
 *
 * @param[in,out] ctx             @ref QBImageContext used for processing
 * @param[in] additionalData      data appended to HMAC calculations
 * @param[in] additionalDataSize  size of @a additionalData
 * @param[in] qbHeader            @ref QBImageHeader containing signature data
 *
 * @return                        @c 0 on success, @c < 0 in case of error/invalid signature
 **/
int QBImageDynamicVerifyFinish(QBImageContext ctx,
                               const void *additionalData, const size_t additionalDataSize,
                               const QBImageHeader *qbHeader);

/**
 * Create dynamic QBBinaryImage.
 *
 * @param[in] payload                buffer containing payload for the image
 * @param[in] payloadSize            size of the payload for the image
 * @param[in] additionalData         data appended to HMAC calculations
 * @param[in] additionalDataSize     size of @a additionalData
 * @param[in] hmacKey                key used for HMAC verification
 * @param[in] aesKey                 key used for encryption
 * @param[in,out] qbImageBuffer      preallocated buffer for QBBinaryImage
 * @param[in,out] qbImageBufferSize  @a in size of the buffer, @a out size of the QBBinaryImage
 *
 * @return                           @c 0 on success, @c < 0 in case of error
 **/
int QBImageDynamicCreate(const void *payload, const size_t payloadSize,
                         const void *additionalData, size_t additionalDataSize,
                         const unsigned char hmacKey[32], const unsigned char aesKey[32],
                         void *qbImageBuffer, size_t *qbImageBufferSize);

/**
 * Setup decryption of QBBinaryImage payload.
 *
 * @param[in,out] ctx          @ref QBImageContext to initialize
 * @param[in] qbHeader         @ref QBImageHeader containing additional decryption information
 * @param[in] aesKey           AES key used for decryption
 *
 * @return                     @c 0 on success, @c < 0 in case of error
 **/
int QBImageDecryptStart(QBImageContext ctx, const QBImageHeader *qbHeader, const unsigned char aesKey[32]);

/**
 * Decrypt chunk of QBBinaryImage payload.
 *
 * @param[in,out] ctx          @ref QBImageContext to update
 * @param[in] payload          buffer containing chunk of QBBinaryImage payload
 * @param[in] payloadSize      length of the chunk
 * @param[in,out] out          preallocated buffer for decrypted data (must be at least @a payloadSize bytes)
 *
 * @return                     @c 0 on success, @c < 0 in case of error
 **/
int QBImageDecrypt(QBImageContext ctx, const void *payload, const size_t payloadSize, void *out);

#endif // QB_BINARY_IMAGE_H
