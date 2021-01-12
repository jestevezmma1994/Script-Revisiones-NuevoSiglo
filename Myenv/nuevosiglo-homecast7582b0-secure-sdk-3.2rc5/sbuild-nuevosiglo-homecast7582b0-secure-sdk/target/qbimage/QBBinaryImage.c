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

#include <polarssl/aes.h>
#include <polarssl/rsa.h>
#include <polarssl/bignum.h>
#include <polarssl/sha256.h>
#include <polarssl/version.h>

struct QBImageContext_ {
    sha256_context shaCtx;
    rsa_context rsaCtx;
    aes_context aesCtx;
    unsigned char iv[16];
};

#include "QBBinaryImage.h"

#ifndef QB_IMAGE_LOG_LEVEL
#define QB_IMAGE_LOG_LEVEL 0
#endif

#if defined(__linux)

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#if QB_IMAGE_LOG_LEVEL > 0
#include <stdio.h>
#define QBImagePrintf(...)     fprintf(stderr, "QBBinaryImage :: " __VA_ARGS__)
#else
#define QBImagePrintf(...)     do { } while(0)
#endif

int QBImageRand(void *buffer, size_t length)
{
    int randfd;
    int res;

    randfd = open("/dev/urandom", O_RDONLY);
    if (randfd == -1) {
        QBImagePrintf("Unable to open /dev/urandom\n");
        return -1;
    }

    res = read(randfd, buffer, length);
    if (res < 0) {
        QBImagePrintf("Read error on /dev/urandom\n");
        return -1;
    }

    close(randfd);

    if ((size_t) res != length) {
        QBImagePrintf("Unable to read %d bytes from /dev/urandom\n", length);
        return -1;
    }

    return 0;
}

static void *QBImageMalloc(size_t size)
{
    return malloc(size);
}

static void QBImageFree(void *ptr)
{
    free(ptr);
}

#elif defined(_CFE_) && (CFE_ENABLE_HW_CRYPTO == 1)

#include "lib_printf.h"
#include "lib_malloc.h"
#include "lib_string.h"
#include "Brcm_BL_Crypto.h"
#include "bchp_common.h"

#if QB_IMAGE_LOG_LEVEL > 0
#define QBImagePrintf(...)     xprintf("QBBinaryImage :: " __VA_ARGS__)
#else
#define QBImagePrintf(...)     do { } while(0)
#endif

static void *QBImageMalloc(unsigned int size)
{
    return KMALLOC(size, 0);
}

static void QBImageFree(void * ptr)
{
    KFREE(ptr);
}

int QBImageRand(void *buffer, size_t length)
{
    Brcm_BL_Crypto_InitParams initParams;
    Brcm_BL_CryptoHandle hCrypto;
    BRCM_BL_CRYPTO_ERROR_CODE errCode;

    memset(&initParams, 0, sizeof (Brcm_BL_Crypto_InitParams));
    initParams.virtual_base     = (unsigned int)(BCHP_PHYSICAL_OFFSET | 0xA0000000);
    initParams.virtual_mem_base = (unsigned int)(0xA0000000);
    initParams.DeviceMallocFunc = QBImageMalloc;
    initParams.DeviceFreeFunc   = QBImageFree;
    initParams.MallocFunc       = QBImageMalloc;
    initParams.FreeFunc         = QBImageFree;

    errCode = Brcm_BL_Crypto_Open(&initParams, &hCrypto);
    if ((errCode != BRCM_BL_CRYPTO_ERROR_CODE_SUCCESS) || (hCrypto == NULL)) {
        xprintf("Brcm_BL_Crypto_Open failed with error 0x%08x\n", errCode);
        return -1;
    }

    errCode = Brcm_BL_Crypto_Common_GenerateRandomNumber(hCrypto, length, buffer);
    Brcm_BL_Crypto_Close(hCrypto);
    if (errCode != BRCM_BL_CRYPTO_ERROR_CODE_SUCCESS) {
        printf("Brcm_BL_Crypto_Common_GenerateRandomNumber failed with error 0x%08x\n", errCode);
        return -1;
    }

    return 0;
}

#elif defined(_CFE_) && (CFE_ENABLE_HW_CRYPTO == 0)

#include "lib_printf.h"
#include "lib_malloc.h"
#include "lib_string.h"
#include "bchp_common.h"

#if QB_IMAGE_LOG_LEVEL > 0
#define QBImagePrintf(...)     xprintf("QBBinaryImage :: " __VA_ARGS__)
#else
#define QBImagePrintf(...)     do { } while(0)
#endif

static void *QBImageMalloc(unsigned int size)
{
    return KMALLOC(size, 0);
}

static void QBImageFree(void * ptr)
{
    KFREE(ptr);
}

int QBImageRand(void *buffer, size_t length)
{
    static unsigned long seed = 1234567890;
    long x, hi, lo, t;
    int offset = 0;

    for (offset = 0; offset < length; offset += 4) {
        x = seed;
        hi = x / 127773;
        lo = x % 127773;
        t = 16807 * lo - 2836 * hi;
        if (t <= 0) t += 0x7fffffff;
        seed = t;
        memcpy(buffer + offset, &t, length > offset + 4 ? 4 : length - offset);
    }

    return 0;
}

#else // defined(__linux) defined(_CFE_)
#error Unknown build environment
#endif

QBImageContext QBImageContextCreate(void)
{
    QBImageContext ctx = (QBImageContext) QBImageMalloc(sizeof(struct QBImageContext_));
    memset(ctx, 0, sizeof(struct QBImageContext_));
    return ctx;
}

void QBImageContextDestroy(QBImageContext ctx)
{
    QBImageFree(ctx);
}

/* QBImage_memcmp
 *
 * A memcmp implementation, which is time attack resistant.
 *
 * Parameters:
 *      s1      buffer A
 *      s2      buffer B
 *      n       amount of data to compare (in bytes)
 *
 * Returns:
 *      0 if buffers are equal, nonzero otherwise
 */
static int QBImageMemcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *t1 = s1;
    const unsigned char *t2 = s2;
    int ret = 0;

    while (n--) {
        ret |= (int) (*t1++ ^ *t2++);
    }
    return ret;
}

static int QBImageDecryptBuffer(const void *in, const size_t size, const unsigned char aesKey[32], const unsigned char IV[16], void *out)
{
    aes_context aesCtx;
    unsigned char tempIV[16];
    int ret = -1;

    aes_init(&aesCtx);
    aes_setkey_dec(&aesCtx, aesKey, 32 * 8);

    memcpy(tempIV, IV, 16);
    if (aes_crypt_cbc(&aesCtx, AES_DECRYPT, size, tempIV, in, out) != 0) {
        QBImagePrintf("Decryption error\n");
        memset(out, 0, size);
    } else {
        ret++;
    }

    return ret;
}

static int QBImageEncryptBuffer(const void *in, const size_t size, const unsigned char aesKey[32], const unsigned char IV[16], void *out)
{
    aes_context aesCtx;
    unsigned char tempIV[16];
    int ret = -1;

    aes_init(&aesCtx);
    aes_setkey_enc(&aesCtx, aesKey, 32 * 8);

    memcpy(tempIV, IV, 16);
    if (aes_crypt_cbc(&aesCtx, AES_ENCRYPT, size, tempIV, in, out) != 0) {
        QBImagePrintf("Encryption error\n");
        memset(out, 0, size);
    } else {
        ret++;
    }

    return ret;
}

int QBImageStaticGetVerifiedHeader(const QBImageHeader *encryptedHeader,
                                   const unsigned char aesKey[32],
                                   const void *rsaN, const size_t rsaNSize,
                                   const void *rsaE, const size_t rsaESize,
                                   QBImageHeader *qbHeader)
{
    static const int steps = 8;

    rsa_context rsaCtx;
    unsigned char shaHash[32];
    unsigned char shaHashBackup[32];
    int ret = -steps;
    int verifyResult = 0;

    memset(qbHeader, 0, sizeof(QBImageHeader));
    memset(shaHash, 0, 32);
    memset(shaHashBackup, 0, 32);
    // Load RSA key from constants in qb_keys.h
    rsa_init(&rsaCtx, RSA_PKCS_V15, 0);
    if (mpi_read_binary(&rsaCtx.N, rsaN, rsaNSize) != 0) {
        QBImagePrintf("Unable to load key N\n");
    } else {
        ret++;
    }
    if (mpi_read_binary(&rsaCtx.E, rsaE, rsaESize) != 0) {
        QBImagePrintf("Unable to load key E\n");
    } else {
        ret++;
    }
    rsaCtx.len = (mpi_msb(&rsaCtx.N) + 7) >> 3;

    sha256((uint8_t *) encryptedHeader, sizeof(QBImageHeader) - sizeof(qbHeader->headerSignature), shaHash, 0);
    memcpy(shaHashBackup, shaHash, 32);
    verifyResult = rsa_pkcs1_verify(&rsaCtx, NULL, NULL, RSA_PUBLIC, POLARSSL_MD_SHA256, 32, shaHash, ((QBImageHeader *) encryptedHeader)->headerSignature.signature);
    if (verifyResult) {
        QBImagePrintf("Invalid QB Image header signature\n");
    } else {
        ret++;
    }

    verifyResult = rsa_pkcs1_verify(&rsaCtx, NULL, NULL, RSA_PUBLIC, POLARSSL_MD_SHA256, 32, shaHashBackup, ((QBImageHeader *) encryptedHeader)->headerSignature.signature);
    if (verifyResult) {
        QBImagePrintf("Invalid QB Image header signature (second pass)\n");
    } else {
        ret++;
    }

    if (QBImageDecryptBuffer(&encryptedHeader->encryptedData, sizeof(qbHeader->encryptedData), aesKey, encryptedHeader->plainData.headerIV, &qbHeader->encryptedData) == 0) {
        ret++;
    }

    if (qbHeader->encryptedData.magic != QB_IMAGE_MAGIC_QBSH) {
        QBImagePrintf("Invalid QB Image magic\n");
    } else {
        ret++;
    }

    if (qbHeader->encryptedData.verificationMode != QB_IMAGE_VERIFICATION_RSA2048_SHA256) {
        QBImagePrintf("Invalid QB Image verification mode\n");
    } else {
        ret++;
    }

    if (qbHeader->encryptedData.version != QB_IMAGE_VERSION) {
        QBImagePrintf("Invalid QB Image version\n");
    } else {
        ret++;
    }

    if (ret != 0)
        QBImagePrintf("Static header (%d/%d)\n", steps + ret, steps);

    return ret;
}

int QBImageStaticVerifyStart(QBImageContext ctx,
                             const void *rsaN, const size_t rsaNSize,
                             const void *rsaE, const size_t rsaESize)
{
    static const int steps = 2;

    int ret = -steps;

    rsa_init(&ctx->rsaCtx, RSA_PKCS_V15, 0);
    if (mpi_read_binary(&ctx->rsaCtx.N, rsaN, rsaNSize) != 0) {
        QBImagePrintf("Unable to load key N\n");
    } else {
        ret++;
    }

    if (mpi_read_binary(&ctx->rsaCtx.E, rsaE, rsaESize) != 0) {
        QBImagePrintf("Unable to load key E\n");
    } else {
        ret++;
    }
    ctx->rsaCtx.len = (mpi_msb(&ctx->rsaCtx.N) + 7) >> 3;

    sha256_starts(&ctx->shaCtx, 0);

    if (ret != 0)
        QBImagePrintf("Static verify start (%d/%d)\n", steps + ret, steps);

    return ret;
}

void QBImageStaticVerifyUpdate(QBImageContext ctx, const void *buffer, const size_t length)
{
    sha256_update(&ctx->shaCtx, buffer, length);
}

int QBImageStaticVerifyFinish(QBImageContext ctx, const QBImageHeader *qbHeader)
{
    static const int steps = 2;

    int verifyResult;
    int ret = -steps;
    unsigned char shaHash[32];
    unsigned char shaHashBackup[32];

    sha256_finish(&ctx->shaCtx, shaHash);
    memcpy(shaHashBackup, shaHash, 32);

    verifyResult = rsa_pkcs1_verify(&ctx->rsaCtx, NULL, NULL, RSA_PUBLIC, POLARSSL_MD_SHA256, 32, shaHash, qbHeader->encryptedData.imageSignature.signature);
    if (verifyResult) {
        QBImagePrintf("Invalid image signature - 0x%08x\n", -verifyResult);
    } else {
        ret++;
    }

    verifyResult = rsa_pkcs1_verify(&ctx->rsaCtx, NULL, NULL, RSA_PUBLIC, POLARSSL_MD_SHA256, 32, shaHashBackup, qbHeader->encryptedData.imageSignature.signature);
    if (verifyResult) {
        QBImagePrintf("Invalid image signature (second pass) - 0x%08x\n", -verifyResult);
    } else {
        ret++;
    }

    if (ret != 0)
        QBImagePrintf("Static verify finish (%d/%d)\n", steps + ret, steps);
    return ret;
}

int QBImageDynamicGetVerifiedHeader(const QBImageHeader *encryptedHeader,
                                    const unsigned char hmacKey[32],
                                    const unsigned char aesKey[32],
                                    const void *additionalData, const size_t additionalDataSize,
                                    QBImageHeader *qbHeader)
{
    static const int steps = 6;

    sha256_context shaCtx;
    unsigned char hmac[32];
    unsigned char hmacBackup[32];
    int ret = -steps;

    memset(hmac, 0, 32);
    memset(hmacBackup, 0, 32);
    sha256_hmac_starts(&shaCtx, hmacKey, 32, 0);
    sha256_hmac_update(&shaCtx, (const unsigned char *) encryptedHeader, sizeof(QBImageHeader) - sizeof(qbHeader->headerSignature));
    if (additionalDataSize && additionalData != NULL)
        sha256_hmac_update(&shaCtx, additionalData, additionalDataSize);
    sha256_hmac_finish(&shaCtx, hmac);
    memcpy(hmacBackup, hmac, 32);

    if (QBImageMemcmp(hmac, encryptedHeader->headerSignature.HMAC, 32)) {
        QBImagePrintf("QB Image header HMAC invalid\n");
    } else {
        ret++;
    }

    if (QBImageMemcmp(hmacBackup, encryptedHeader->headerSignature.HMAC, 32)) {
        QBImagePrintf("QB Image header HMAC invalid (second pass)\n");
    } else {
        ret++;
    }

    if (QBImageDecryptBuffer(&encryptedHeader->encryptedData, sizeof(qbHeader->encryptedData), aesKey, encryptedHeader->plainData.headerIV, &qbHeader->encryptedData) == 0) {
        ret++;
    }

    if (qbHeader->encryptedData.magic != QB_IMAGE_MAGIC_QBDH) {
        QBImagePrintf("Invalid QB Image magic\n");
    } else {
        ret++;
    }

    if (qbHeader->encryptedData.version != QB_IMAGE_VERSION) {
        QBImagePrintf("Invalid QB image version\n");
    } else {
        ret++;
    }

    if (qbHeader->encryptedData.verificationMode != QB_IMAGE_VERIFICATION_HMAC256_SHA256) {
        QBImagePrintf("Invalid QB Image verification mode\n");
    } else {
        ret++;
    }

    if (ret != 0)
        QBImagePrintf("Dynamic header (%d/%d)\n", steps + ret, steps);

    return ret;
}

void QBImageDynamicVerifyStart(QBImageContext ctx, const unsigned char hmacKey[32])
{
    sha256_hmac_starts(&ctx->shaCtx, hmacKey, 32, 0);
}

void QBImageDynamicVerifyUpdate(QBImageContext ctx, const void *buffer, const size_t length)
{
    sha256_hmac_update(&ctx->shaCtx, buffer, length);
}

int QBImageDynamicVerifyFinish(QBImageContext ctx, const void *additionalData, const size_t additionalDataSize, const QBImageHeader *qbHeader)
{
    static const int steps = 2;

    unsigned char hmac[32];
    unsigned char hmacBackup[32];
    int ret = -steps;

    memset(hmac, 0, 32);
    memset(hmacBackup, 0, 32);
    if (additionalDataSize && additionalData != NULL)
        sha256_hmac_update(&ctx->shaCtx, additionalData, additionalDataSize);
    sha256_hmac_finish(&ctx->shaCtx, hmac);
    memcpy(hmacBackup, hmac, 32);

    if (QBImageMemcmp(hmac, qbHeader->encryptedData.imageSignature.HMAC, 32)) {
        QBImagePrintf("QB Image HMAC invalid\n");
    } else {
        ret++;
    }

    if (QBImageMemcmp(hmacBackup, qbHeader->encryptedData.imageSignature.HMAC, 32)) {
        QBImagePrintf("QB Image HMAC invalid (second pass)\n");
    } else {
        ret++;
    }

    if (ret != 0)
        QBImagePrintf("Dynamic verify finish (%d/%d)\n", steps + ret, steps);
    return ret;
}

int QBImageDecryptStart(QBImageContext ctx, const QBImageHeader *qbHeader, const unsigned char aesKey[32])
{
    static const int steps = 1;

    int ret = -steps;
    unsigned char newKey[32];

    aes_init(&ctx->aesCtx);

    if (qbHeader->encryptedData.encryptionMode == QB_IMAGE_ENCRYPTION_AES256CBC_KEYCHAIN) {
        unsigned char tmpIV[16];
        ret--;
        memset(newKey, 0, 32);
        memcpy(tmpIV, qbHeader->encryptedData.encryptedKeyIV, 16);
        if (QBImageDecryptBuffer(qbHeader->encryptedData.encryptedKey, 32, aesKey, tmpIV, newKey) == 0) {
            ret++;
        }
    } else {
        memcpy(newKey, aesKey, 32);
    }

    aes_setkey_dec(&ctx->aesCtx, newKey, 32 * 8);
    memcpy(ctx->iv, qbHeader->encryptedData.imageIV, 16);

    ret++;
    if (ret != 0)
        QBImagePrintf("Decrypt (%d/%d)\n", steps + ret, steps);

    return ret;
}

int QBImageDecrypt(QBImageContext ctx, const void *payload, const size_t payloadSize, void *out)
{
    if (aes_crypt_cbc(&ctx->aesCtx, AES_DECRYPT, payloadSize, ctx->iv, payload, out) != 0) {
        QBImagePrintf("QB Image AES decryption error\n");
        return -1;
    } else {
        return 0;
    }

    return -1;
}

int QBImageStaticCreateUnsigned(const void *payload, const size_t payloadSize,
                                const unsigned char aesKey[32],
                                void *qbImageBuffer, size_t *qbImageBufferSize)
{
    QBImageHeader qbHeader;
    QBImageHeader *destHeader = (QBImageHeader *) qbImageBuffer;
    unsigned char *destPayload = (unsigned char *) qbImageBuffer + sizeof(QBImageHeader);

    if (payloadSize % 16) {
        QBImagePrintf("Image size is not a multiple of 16 bytes\n");
        *qbImageBufferSize = 0;
        return -1;
    }

    if (qbImageBufferSize == NULL || *qbImageBufferSize < payloadSize + sizeof(QBImageHeader)) {
        QBImagePrintf("Image will not fit the buffer (%d bytes needed)\n", payloadSize + sizeof(QBImageHeader));
        *qbImageBufferSize = 0;
        return -1;
    }

    *qbImageBufferSize = 0;
    /* initialize the header with random data first */
    if (QBImageRand(&qbHeader, sizeof(QBImageHeader)) != 0) {
        QBImagePrintf("Random number generation failed\n");
        return -1;
    }

    /* fill out the header fields */
    qbHeader.encryptedData.magic = QB_IMAGE_MAGIC_QBSH;
    qbHeader.encryptedData.version = QB_IMAGE_VERSION;
    qbHeader.encryptedData.imageSize = QB_IMAGE_CONVERT_ENDIAN((uint32_t) payloadSize);
    qbHeader.encryptedData.verificationMode = QB_IMAGE_VERIFICATION_RSA2048_SHA256;
    qbHeader.encryptedData.encryptionMode = QB_IMAGE_ENCRYPTION_AES256CBC;
    /* header->imageIV is already initialized to random data */
    QBImageEncryptBuffer(payload, payloadSize, aesKey, qbHeader.encryptedData.imageIV, destPayload);

    /* Copy plain data */
    memcpy(&destHeader->plainData, &qbHeader.plainData, sizeof(destHeader->plainData));
    /* Encrypt part of the header and place it in dest buffer */
    QBImageEncryptBuffer(&qbHeader.encryptedData, sizeof(destHeader->encryptedData), aesKey, destHeader->plainData.headerIV, &destHeader->encryptedData);

    /* Copy header signature, this signature is fake, but in qbHeader it was initialized with random data, so we copy it anyway */
    memcpy(&destHeader->headerSignature, &qbHeader.headerSignature, sizeof(destHeader->headerSignature));

    *qbImageBufferSize = payloadSize + sizeof(QBImageHeader);

    return 0;
}

int QBImageDynamicCreate(const void *payload, const size_t payloadSize,
                         const void *additionalData, size_t additionalDataSize,
                         const unsigned char hmacKey[32], const unsigned char aesKey[32],
                         void *qbImageBuffer, size_t *qbImageBufferSize)
{
    sha256_context shaCtx;
    QBImageHeader qbHeader;
    QBImageHeader *destHeader = (QBImageHeader *)qbImageBuffer;
    unsigned char *destPayload = (unsigned char *) qbImageBuffer + sizeof(QBImageHeader);

    if (payloadSize % 16) {
        QBImagePrintf("Image size is not a multiple of 16 bytes\n");
        *qbImageBufferSize = 0;
        return -1;
    }

    if (qbImageBufferSize == NULL || *qbImageBufferSize < payloadSize + sizeof(QBImageHeader)) {
        QBImagePrintf("Image will not fit the buffer (%d bytes needed)\n", payloadSize + sizeof(QBImageHeader));
        *qbImageBufferSize = 0;
        return -1;
    }

    *qbImageBufferSize = 0;

    /* initialize the header with random data first */
    if (QBImageRand(&qbHeader, sizeof(QBImageHeader)) != 0) {
        QBImagePrintf("Random number generation failed\n");
        return -1;
    }

    if (QBImageRand(destHeader, sizeof(QBImageHeader)) != 0) {
        QBImagePrintf("Random number generation failed\n");
        return -1;
    }

    /* fill out the header fields */
    qbHeader.encryptedData.magic = QB_IMAGE_MAGIC_QBDH;
    qbHeader.encryptedData.version = QB_IMAGE_VERSION;
    qbHeader.encryptedData.imageSize = QB_IMAGE_CONVERT_ENDIAN((uint32_t) payloadSize);
    qbHeader.encryptedData.verificationMode = QB_IMAGE_VERIFICATION_HMAC256_SHA256;
    qbHeader.encryptedData.encryptionMode = QB_IMAGE_ENCRYPTION_AES256CBC;
    /* header->imageIV is already initialized to random data */
    QBImageEncryptBuffer(payload, payloadSize, aesKey, qbHeader.encryptedData.imageIV, destPayload);
    /* evaluate HMAC */
    sha256_hmac_starts(&shaCtx, hmacKey, 32, 0);
    sha256_hmac_update(&shaCtx, destPayload, payloadSize);
    if (additionalDataSize && additionalData != NULL)
        sha256_hmac_update(&shaCtx, additionalData, additionalDataSize);
    sha256_hmac_finish(&shaCtx, qbHeader.encryptedData.imageSignature.HMAC);

    /* Copy plain data */
    memcpy(&destHeader->plainData, &qbHeader.plainData, sizeof(qbHeader.plainData));
    /* Encrypt and copy data */
    QBImageEncryptBuffer(&qbHeader.encryptedData, sizeof(destHeader->encryptedData), aesKey, destHeader->plainData.headerIV, &destHeader->encryptedData);
    /* finally evaluate the header HMAC */
    sha256_hmac_starts(&shaCtx, hmacKey, 32, 0);
    sha256_hmac_update(&shaCtx, (unsigned char *) destHeader, sizeof(QBImageHeader) - sizeof(destHeader->headerSignature));
    if (additionalDataSize && additionalData != NULL)
        sha256_hmac_update(&shaCtx, additionalData, additionalDataSize);
    sha256_hmac_finish(&shaCtx, destHeader->headerSignature.HMAC);

    *qbImageBufferSize = payloadSize + sizeof(QBImageHeader);

    return 0;
}
