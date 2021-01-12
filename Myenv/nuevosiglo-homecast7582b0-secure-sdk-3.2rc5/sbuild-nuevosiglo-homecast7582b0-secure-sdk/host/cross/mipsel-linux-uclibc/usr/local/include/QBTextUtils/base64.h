/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_BASE_64_H
#define QB_BASE_64_H

#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Decode Base64 encoded data.
 *
 * @param[in] in    Base64 string
 * @param[in] inlen length of @a in in bytes
 * @param[in] out   output buffer, at least (3 * (@a inlen / 4)) bytes long
 * @return          number of bytes written to @a out, @c -1 on error
 **/
extern ssize_t QBBase64Decode(const char* in, size_t inlen, unsigned char* out);

/**
 * Decode Base64 encoded data.
 *
 * @param[in] in    Base64 string
 * @param[in] inlen length of @a in in bytes
 * @param[in] out   output buffer, at least (3 * (@a inlen / 4)) bytes long
 * @param[in] ignoreWhiteSpace @c true to silently ignore white space characters
 *                  on input (space, horizontal tab, new line, carriage return)
 * @return          number of bytes written to @a out, @c -1 on error
 **/
extern ssize_t QBBase64Decode_(const char* in, size_t inlen, unsigned char* out, bool ignoreWhiteSpace);

/**
 * Encode data to Base64.
 *
 * @param[in] in    data to encode
 * @param[in] inlen length of @a in in bytes
 * @param[in] out   output buffer, at least (4 * ((@a inlen + 2) / 3)) bytes long
 * @return          number of bytes written to @a out, @c -1 on error
 **/
extern ssize_t QBBase64Encode(const unsigned char *in, size_t inlen, char *out);

/**
 * Encode data to Base64URL.
 *
 * This method encodes data to Base 64 Encoding with URL and Filename Safe Alphabet.
 *
 * @param[in] in    data to encode
 * @param[in] inlen length of @a in in bytes
 * @param[in] out   output buffer, at least (4 * ((@a inlen + 2) / 3)) bytes long
 * @return          number of bytes written to @a out, @c -1 on error
 **/
extern ssize_t QBBase64URLEncode(const unsigned char *in, size_t inlen, unsigned char *out);

struct QBBase64Decoder {
    /// @c true to silently ignore white space characters on input
    bool ignoreWhiteSpace;
    /// number of padding '=' characters found
    unsigned char padCount;
    /// number of bytes left from previous input chunk (from @c 0 to @c 3)
    unsigned short int tailLength;
    /// tail bits from previous input chunk
    unsigned int tail;
};

/**
 * Initialize Base64 decoder.
 *
 * @param[in] decoder   Base64 decoder handle
 * @param[in] ignoreWhiteSpace @c true to silently ignore white space characters
 *                      on input (space, horizontal tab, new line, carriage return)
 **/
static inline void QBBase64DecoderInit(struct QBBase64Decoder *const decoder, bool ignoreWhiteSpace)
{
    decoder->ignoreWhiteSpace = ignoreWhiteSpace;
    decoder->padCount = 0;
    decoder->tailLength = 0;
    decoder->tail = 0;
}

/**
 * Check if last Base64 encoded data chunk was decoded completely.
 *
 * @param[in] decoder   Base64 decoder handle
 * @return              @c true if complete decoded data was written to the output,
 *                      @c false if decoder has some data in internal buffer
 **/
static inline bool QBBase64DecoderIsComplete(const struct QBBase64Decoder *const decoder)
{
    return decoder->tailLength == 0;
}

/**
 * Decode next chunk of Base64 encoded data.
 *
 * @param[in] decoder   Base64 decoder handle
 * @param[in] input     Base64 string
 * @param[in] inputLength length of @a input in bytes
 * @param[in] output    output buffer, at least (3 * @a inputLength / 4 + 2) bytes long
 * @return              number of bytes written to @a output, @c -1 on error
 **/
extern ssize_t QBBase64DecoderDecode(struct QBBase64Decoder *const decoder,
                                     const char *input,
                                     size_t inputLength,
                                     unsigned char *output);


#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_BASE_64_H
