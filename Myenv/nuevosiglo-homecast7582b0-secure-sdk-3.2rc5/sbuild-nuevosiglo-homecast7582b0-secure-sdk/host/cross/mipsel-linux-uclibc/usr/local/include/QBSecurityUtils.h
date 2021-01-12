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

#ifndef QBSECURITYUTILS_H
#define QBSECURITYUTILS_H

/**
 * @file QBSecurityUtils.h
 * @brief Utils for secure handling of files and other crypto-related tasks
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <openssl/hmac.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

/**
 * @ingroup Utils QBSecurityUtils
 * @{
 **/

/**
 * QBSecurityUtils routines
 *
 * This modules is designed to "securely" handle (config) files and perhaps other
 * crypto-related primitives. Underneath it relies on OpenSSL/libcrypto
 * primitives and keys stored in "secure" places.
 * Depending on the platform it will probably be extended to handle
 * STB-unique H/W secrets, etc.
 *
**/

/**
 * Write a buffer to filename accompanied by the creation of HMAC-SHA256 for said file.
 * HMAC is kept in two additional files created under the same path: filename.hmac.1
 * and filename.hmac.2
 * HMAC is calculated over file data concatenated with filename
 * Two hmac files are necessary to recover from an errourness condition where
 * system reboot occured after syncing main filename and before syncing hmac
 * representation of the file.
 * This is achieved by sychronizing two hmac files (if they do differ) to the
 * correct hmac value of filename before writing the new set (filename, hmacs) to disk.
 * During unexpected reboot (lack of sync) this will prevent future reads
 * from failing a verification.
 * The exact write flow is as follows:
 * - sync hmac1 and hmac (from possible earlier failure)
 * - create temporary version of filename, filename.hmac.1, filename.hmac.2
 * - do atomic replace of temporary files to final files in the following order: hmac1, filename, hmac2.
 * - unlink temporary files
 *
 * The recovery mechanism is then used by QBSecurityUtilsReadFromFileWithHMAC() when file is read
 * from the device.
 * @param[in] filename name/path of the file the buffer shall be written to
 * @param[in] buffer input data to be written to filename
 * @param[in] data_len number of bytes to be written. Needs to be less or equal to max_len.
 * @param[in] max_len denotes maximum number of bytes that can be written (also read and synced)
 * @return number of bytes written to file in case of success and negative error code in case of failure
 */
int QBSecurityUtilsWriteToFileWithHMAC(const char *filename, const void *buffer, size_t data_len, unsigned long max_len);

/**
 * Read a data filename to buffer followed by HMAC verification. If hmac verification fails
 * then buffer returned is NULL and function returns an error number that is less than zero.
 * Otherwise buffer is allocated to the size of the input data plus 1 - this additional position
 * is then null-terminated (in other words zero is appended to the end of buffer). In this case
 * function returns number of bytes read (greater than zero). It is the responsibility of user
 * to free buffer once it has been used.
 * HMAC is calculated over file data concatenated with filename
 * Note that this function has en explicit limit to the number of bytes that can be read
 * from file (for security reasons), so always make sure to bound this to a reasonable amount with the
 * assumption that the file you want to read has been compromised (e.g. considerably expanded in size)
 * This function also introduces automatic failure recovery from errourness condition
 * when filename and corrsponding hmac files are desynchronized (because of sync failures).
 * This happens by calucating hmac for filename and comparing it whichever hmac -
 * if at least one matches then we successfully verify the filename. Otherwise verification fails
 * and buffer shall not be used for anything in user's code.
 * @param[in] filename name/path of the file that shall be read
 * @param[in] max_len denotes maximum number of bytes that can be read during hmac syncing
 * @param[out] buffer buffer allocated by the function in case verification succeeds
 * @return number of bytes read from file in case of success and negative error code in case of failure
 */
int QBSecurityUtilsReadFromFileWithHMAC(const char *filename, char **buffer, unsigned long max_len);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* QBSECURITYUTILS_H */

