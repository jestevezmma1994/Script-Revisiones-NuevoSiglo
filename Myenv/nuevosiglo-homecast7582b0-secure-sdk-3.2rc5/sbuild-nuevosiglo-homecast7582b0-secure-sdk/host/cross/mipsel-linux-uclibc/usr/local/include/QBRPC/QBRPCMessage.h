/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_RPC_MESSAGE_H_
#define QB_RPC_MESSAGE_H_

#include <SvFoundation/SvCoreTypes.h>

#include <common/QBRPCMessageCommon.h>


/**
 * Get string field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      string length, @c -1 in case of error
 **/
static inline ssize_t QBRPCMessageGetStringField(QBRPCMessage* self, char** v) {
    return QBRPCMessageGetStringField_(self, v);
}

/**
 * Get unsigned long field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetULongField(QBRPCMessage* self, uint32_t* v) {
    return QBRPCMessageGetULongField_(self, v);
}

/**
 * Get char field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetCharField(QBRPCMessage* self, char* v) {
    return QBRPCMessageGetCharField_(self, v);
}

/**
 * Get octet field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetOctetField(QBRPCMessage* self, uint8_t* v) {
    return QBRPCMessageGetOctetField_(self, v);
}

/**
 * Get boolean field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetBooleanField(QBRPCMessage* self, bool* v) {
    return QBRPCMessageGetBooleanField_(self, v);
}

/**
 * Get short field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetShortField(QBRPCMessage* self, int16_t* v) {
    return QBRPCMessageGetShortField_(self, v);
}

/**
 * Get unsigned short field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetUShortField(QBRPCMessage* self, uint16_t* v) {
    return QBRPCMessageGetUShortField_(self, v);
}

/**
 * Get long field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetLongField(QBRPCMessage* self, int32_t* v) {
    return QBRPCMessageGetLongField_(self, v);
}

/**
 * Get long long field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetLongLongField(QBRPCMessage* self, int64_t* v) {
    return QBRPCMessageGetLongLongField_(self, v);
}

/**
 * Get unsigned long long field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetULongLongField(QBRPCMessage* self, uint64_t* v) {
    return QBRPCMessageGetULongLongField_(self, v);
}

/**
 * Get float field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetFloatField(QBRPCMessage* self, float* v) {
    return QBRPCMessageGetFloatField_(self, v);
}

/**
 * Get double field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetDoubleField(QBRPCMessage* self, double* v) {
    return QBRPCMessageGetDoubleField_(self, v);
}

/**
 * Get long double field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageGetLongDoubleField(QBRPCMessage* self, long double* v) {
    return QBRPCMessageGetLongDoubleField_(self, v);
}

/**
 * Append string field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 string to be appended
 * @param[in] length            string length
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
static inline int QBRPCMessageAppendStringField(QBRPCMessage* self, const char* v, ssize_t length) {
    return QBRPCMessageAppendStringField_(self, v, length);
}

/**
 * Get string field from message payload as SvString.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                pointer to SvString for storing read value.
 *
 * @return                      string length, @c -1 in case of error
 **/
ssize_t QBRPCMessageGetSvStringField(QBRPCMessage* self, SvString* v);

/**
 * Append SvString field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] s                 string to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
int QBRPCMessageAppendSvStringField(QBRPCMessage* self, SvString s);

#endif // QB_RPC_MESSAGE_H_
