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

#ifndef QB_RPC_MESSAGE_COMMON_H_
#define QB_RPC_MESSAGE_COMMON_H_

#include <common/QBRPCInterfaceCall.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * RPC message types.
 **/
typedef enum {
    QBRPCMessageType_unknown = 0,
    QBRPCMessageType_hello,
    QBRPCMessageType_resource,
    QBRPCMessageType_queryRequest,
    QBRPCMessageType_queryResponse,
    QBRPCMessageType_callRequest,
    QBRPCMessageType_callResponse,
    QBRPCMessageType_MonitorRequest,
    QBRPCMessageType_MonitorResponse,
    QBRPCMessageType__count__
} QBRPCMessageType;

/**
 * RPC message field types.
 **/
typedef enum {
    QBRPCMessageFieldType_unknown = 0,
    QBRPCMessageFieldType_char,
    QBRPCMessageFieldType_octet,
    QBRPCMessageFieldType_boolean,
    QBRPCMessageFieldType_short,
    QBRPCMessageFieldType_ushort,
    QBRPCMessageFieldType_long,
    QBRPCMessageFieldType_ulong,
    QBRPCMessageFieldType_longlong,
    QBRPCMessageFieldType_ulonglong,
    QBRPCMessageFieldType_float,
    QBRPCMessageFieldType_double,
    QBRPCMessageFieldType_longdouble,
    QBRPCMessageFieldType_string,
    QBRPCMessageFieldType_enum,
    QBRPCMessageFieldType_struct,
    QBRPCMessageFieldType_array,
    QBRPCMessageFieldType__count__
} QBRPCMessageFieldType;


/**
 * On-wire format of the message header.
 **/
struct QBRPCMessageHeader {
    uint8_t type;
    uint8_t reserved;
    uint16_t payloadLength;
    uint32_t seq;
    int32_t descriptor;
};


/**
 * QBRPC message class.
 **/
typedef struct QBRPCMessage_ {
    /// message type
    QBRPCMessageType type;

    /// sequential number for matching request with response, @c 0 if n/a
    unsigned int seq;

    /// resource (file descriptor) associated with message, @c -1 if n/a
    int descriptor;

    /// message payload length
    size_t payloadLength;

    /// message payload
    unsigned char *payload;

    /// buffer size
    size_t bufferLength;

    /// number of fields in the payload
    size_t fieldsCount;

    /// offset of the next field in the payload
    size_t readOffset;
} QBRPCMessage;


/**
 * Create 'hello' message.
 *
 * @param[in] UUID              endpoint UUID
 *
 * @return                      @a message, @c NULL in case of error
 **/
extern QBRPCMessage* QBRPCMessageCreateHello(const char* UUID);

/**
 * Create message representing a resource (file).
 *
 * @param[in] label             resource label
 * @param[in] descriptor        resource (file) descriptor
 *
 * @return                      @a message, @c NULL in case of error
 **/
extern QBRPCMessage* QBRPCMessageCreateResource(const char* label, int descriptor);

/**
 * Create message representing a query request.
 *
 * @param[in] serviceName       service name
 * @param[in] interfaceName     interface name
 *
 * @return                      @a message, @c NULL in case of error
 **/
extern QBRPCMessage* QBRPCMessageCreateQueryRequest(const char* serviceName, const char* interfaceName);

/**
 * Create message representing a response to query request.
 *
 * @param[in] seq               sequential number for matching request with response
 * @param[in] serviceName       service name
 *
 * @return                      @a message, @c NULL in case of error
 **/
extern QBRPCMessage* QBRPCMessageCreateQueryResponse(unsigned int seq, const char* serviceName);

/**
 * Create message representing a call request.
 *
 * @param[in] serviceName       service name
 * @param[in] interfaceName     interface name
 * @param[in] methodName        method name
 * @param[in] oneway            set to @c true if caller expects response to this message
 *
 * @return                      @a message, @c NULL in case of error
 **/
extern QBRPCMessage* QBRPCMessageCreateCallRequest(const char* serviceName, const char* interfaceName, const char* methodName, bool oneway);

/**
 * Create message representing a response to call request.
 *
 * @param[in] serviceName       service name
 * @param[in] seq               sequential number for matching request with response
 * @param[in] status            call's return status
 *
 * @return                      @a message, @c NULL in case of error
 **/
extern QBRPCMessage* QBRPCMessageCreateCallResponse(const char* serviceName, unsigned int seq, QBRPCInterfaceCallStatus status);

/**
 * Create message using header and payload.
 *
 * @param[in] header            message header
 * @param[in] payload           message payload
 *
 * @return                      @a message, @c NULL in case of error
 **/
extern QBRPCMessage* QBRPCMessageConstruct(const struct QBRPCMessageHeader* header, const unsigned char* payload);

/**
 * Create message using header and payload.
 *
 * @param[in] self              message handle
 * @param[out] buffer           buffer for storing serialized message
 * @param[in] availableMemSize  max. available memory size for storing a serialized message in buffer
 *
 * @return                      number of bytes written to buffer, @c -1 in case of error
 **/
extern ssize_t QBRPCMessageSerialize(QBRPCMessage* self, char* buffer, size_t availableMemSize);

/**
 * Get next message field type.
 *
 * @param[in] self              message handle
 *
 * @return                      message field type
 **/
extern QBRPCMessageFieldType QBRPCMessageGetNextFieldType(QBRPCMessage* self);

/**
 * Get string field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      string length, @c -1 in case of error
 **/
extern ssize_t QBRPCMessageGetStringField_(QBRPCMessage* self, char** v);

/**
 * Get unsigned long field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetULongField_(QBRPCMessage* self, uint32_t* v);

/**
 * Get char field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetCharField_(QBRPCMessage* self, char* v);

/**
 * Get octet field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetOctetField_(QBRPCMessage* self, uint8_t* v);

/**
 * Get boolean field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetBooleanField_(QBRPCMessage* self, bool* v);

/**
 * Get short field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetShortField_(QBRPCMessage* self, int16_t* v);

/**
 * Get unsigned short field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetUShortField_(QBRPCMessage* self, uint16_t* v);

/**
 * Get long field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetLongField_(QBRPCMessage* self, int32_t* v);

/**
 * Get long long field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetLongLongField_(QBRPCMessage* self, int64_t* v);

/**
 * Get unsigned long long field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetULongLongField_(QBRPCMessage* self, uint64_t* v);

/**
 * Get float field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetFloatField_(QBRPCMessage* self, float* v);

/**
 * Get double field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetDoubleField_(QBRPCMessage* self, double* v);

/**
 * Get long double field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] v                buffer for storing read value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetLongDoubleField_(QBRPCMessage* self, long double* v);

/**
 * Get enum field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] name             buffer for storing enum's type name
 * @param[out] v                buffer for storing enum's value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetEnumField(QBRPCMessage* self, char** name, char** v);

/**
 * Get struct field from message payload.
 * If successful, this method advances message field iterator.
 *
 * @param[in] self              message handle
 * @param[out] typeName         buffer for storing struct's type name
 * @param[out] membersCount     buffer for storing struct members (variables) count
 *                              this value tells how many of next fields (struct member fields) belong to this struct
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetStructField(QBRPCMessage* self, char** typeName, size_t* membersCount);

/**
 * Get struct member field from message payload.
 * This field describes a single struct member (variable).
 * If successful, this method advances message field iterator.
 * Next field should contain struct's member (variable) value.
 *
 * @param[in] self              message handle
 * @param[out] memberName       buffer for storing struct's member (variable) name
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageGetStructMemberField(QBRPCMessage* self, char** memberName);

/**
 * Get array field from message payload.
 * This field contains the count of array elements.
 * If successful, this method advances message field iterator.
 * Next field should represent first array element.
 *
 * @param[in] self              message handle
 * @param[out] fieldsCount      buffer for storing array elements count
 *                              this value tells how many of next fields belong to this array
 * @param[out] type             type of elements stored in array
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 */
extern int QBRPCMessageGetArrayField(QBRPCMessage* self, size_t* fieldsCount, QBRPCMessageFieldType* type);

/**
 * Append string field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 string to be appended
 * @param[in] length            string length
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendStringField_(QBRPCMessage* self, const char* v, ssize_t length);

/**
 * Append unsigned long field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendULongField(QBRPCMessage* self, uint32_t v);

/**
 * Append char field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendCharField(QBRPCMessage* self, char v);

/**
 * Append octet field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/

extern int QBRPCMessageAppendOctetField(QBRPCMessage* self, uint8_t v);

/**
 * Append boolean field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendBooleanField(QBRPCMessage* self, bool v);

/**
 * Append short field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendShortField(QBRPCMessage* self, int16_t v);

/**
 * Append unsigned short field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendUShortField(QBRPCMessage* self, uint16_t v);

/**
 * Append long field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendLongField(QBRPCMessage* self, int32_t v);

/**
 * Append long long field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendLongLongField(QBRPCMessage* self, int64_t v);

/**
 * Append unsigned long long field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendULongLongField(QBRPCMessage* self, uint64_t v);

/**
 * Append float field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendFloatField(QBRPCMessage* self, float v);

/**
 * Append double field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendDoubleField(QBRPCMessage* self, double v);

/**
 * Append long double field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] v                 value to be appended
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendLongDoubleField(QBRPCMessage* self, long double v);

/**
 * Append enum field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] name              enum's type name
 * @param[in] v                 enum's value
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendEnumField(QBRPCMessage* self, const char* name, const char* v);

/**
 * Append struct field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] typeName          struct's type name
 * @param[in] memberCount       struct's members (variables) count
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendStructField(QBRPCMessage* self, const char* typeName, size_t memberCount);

/**
 * Append struct member field to message payload.
 * Variable's value field should be appended after struct member field.
 *
 * @param[in] self              message handle
 * @param[in] memberName        struct's member (variable) name
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 **/
extern int QBRPCMessageAppendStructMemberField(QBRPCMessage* self, const char* memberName);

/**
 * Append array field to message payload.
 *
 * @param[in] self              message handle
 * @param[in] fieldsCount       array's elements count
 * @param[in] type              array's elements type
 *
 * @return                      @c 0 if successful, @c -1 in case of error
 */
extern int QBRPCMessageAppendArrayField(QBRPCMessage* self, size_t fieldsCount, QBRPCMessageFieldType type);

/**
 * Print message info.
 *
 * @param[in] self              message handle
 *
 **/
extern void QBRPCMessagePrintInfo(QBRPCMessage* self);

/**
 * Destroy message.
 *
 * @param[in] self              message handle
 *
 **/
extern void QBRPCMessageDestroy(QBRPCMessage* self);

/**
 * Copy message.
 *
 * @param[in] self              message handle
 *
 * @return                      @a message copy handle, @c NULL in case of error
 **/
extern QBRPCMessage* QBRPCMessageCopy(QBRPCMessage* self);

/**
 * @brief Create message with monitoring request
 * @param interval_ time interval determining how frequent monitor messages are sent
 * @return created message
 */
extern QBRPCMessage *QBRPCMessageCreateMonitorRequest(uint32_t interval_);

/**
 * @brief Create message being monitoring response
 * @return created message
 */
extern QBRPCMessage *QBRPCMessageCreateMonitorResponse(void);

#ifdef __cplusplus
}
#endif

#endif // QB_RPC_MESSAGE_COMMON_H_
