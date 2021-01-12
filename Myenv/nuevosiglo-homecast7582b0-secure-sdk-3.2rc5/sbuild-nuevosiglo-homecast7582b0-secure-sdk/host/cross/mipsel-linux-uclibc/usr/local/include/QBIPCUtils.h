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

#ifndef QB_IPC_UTILS_H
#define QB_IPC_UTILS_H

/**
 * @file QBIPCSocket.h IPC Socket API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBIPCUtils QBIPCUtils
 * @{
 **/

/// Default timeout in seconds for select in QBIPCSocketSend function
#define QB_IPC_SEND_TIMEOUT_SEC 2

/**
 * Serialize an IPC message in \#key1=value1\#key2=value2\#...\# syntax.
 *
 * @param[out] buf      buffer to be filled in with serialized message
 * @param[in] maxLen    maximum bytes that can be filled in the buf
 * @param[in] keys      an array of keys
 * @param[in] values    an array of values with indices corresponding to keys
 * @param[in] cnt       count of arrays of keys and values
 * @return  number of serialized bytes
 **/
int
QBIPCMessageSerialize(unsigned char *buf,
                      int maxLen,
                      const unsigned char *keys[],
                      const unsigned char *values[],
                      int cnt);

/**
 * Deserialize an IPC message in \#key1=value1\#key2=value2\#...\# syntax.
 *
 * @param[in] buf       buffer containing the IPC message (be careful - the buffer will be modified!!!)
 * @param[in] len       number of bytes in the buffer
 * @param[out] keys     an array to be filled in with deserialized keys
 * @param[out] values   an array to be filled in with deserialized values
 * @param[in] maxCnt    maximum count of keys and values that can be parsed
 * @return  number of deserialized keys/values
 **/
int
QBIPCMessageDeserialize(char *buf,
                        int len,
                        char **keys,
                        char **values,
                        int maxCnt);

/**
 * Send an IPC message without setting up a QBIPCSocket.
 * The QBIPCSocket will be set up automatically.
 *
 * @param[in] path  path of the QBIPCSocket on which the message is to be sent
 * @param[in] str   an IPC message to be sent
 * @return  zero on success, -1 on failure
 **/
int
QBIPCQuickSend(const char *path, const char *str);

/**
 * @}
**/

#endif /* QB_IPC_UTILS_H */
