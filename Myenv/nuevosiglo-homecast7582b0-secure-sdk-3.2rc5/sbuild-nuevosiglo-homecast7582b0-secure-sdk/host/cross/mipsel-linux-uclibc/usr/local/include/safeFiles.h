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


#ifndef SAFEFILES_H
#define SAFEFILES_H

#include <sys/stat.h>

/**
 * @file Src/utils/safeFiles.h Header contains functions to write and read files safely
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup safeFiles safeFiles
 * @{
 **/

/**
* @brief Read data from file and copy them to buffer. Remember to check value
* returned by function before you start reading data from buffer.
*
* @param[in] filename   full path to file with data
* @param[in] buffer     a place where data from file will be stored
*
* @return           return bytes read on success, -errno on failure.
*/
int QBFileToBuffer(const char *filename, char **buffer);

/**
* @brief Read data from file and copy them to buffer. Remember to check value
* returned by function before you start reading data from buffer. This function
* is similar to fileTuBuffer, but it also return size of the file by an
* output parameter.
*
* @param[in] filename   full path to file with data
* @param[in] buffer     a place where data from file will be stored
* @param[out] length    size of the file
* @return           return bytes read on success, -errno on failure.
*/
int QBFileToBufferWithLength(const char *filename, char **buffer, long *length);

/**
* @brief Write data from buffer into a file.
*
* @param[in] filename   full path to file
* @param[in] buffer     buffer with data
*
* @return bytes written on success, -errno on failure.
*/
int QBBufferToFile(const char *filename, const void *buffer);

/**
* @brief Write up to bufferLenght bytes of data from buffer into a file.
*
* @param[in] filename       full path to file
* @param[in] buffer         buffer with data
* @param[in] bufferLength   number of bytes to be written to the file
*
* @return bytes written on success, -errno on failure.
*/
int QBBufferToFileWithLength(const char *filename, const void *buffer, int bufferLength);

/**
* @brief Write data from buffer into a file. File will be given requested mode.
*
* @param[in] filename   full path to file
* @param[in] buffer     buffer with data
* @param[in] mode       type of file attribute bitmasks
*
* @return bytes written on success, -errno on failure.
*/
int QBBufferToFileWithMode(const char *filename, const void *buffer, mode_t mode);

/**
* @brief Write up to bufferLenght bytes of data from buffer into a file.
* File will be given requested mode.
*
* @param[in] filename       full path to file
* @param[in] buffer         buffer with data
* @param[in] bufferLength   number of bytes to be written to the file
* @param[in] mode       type of file attribute bitmasks
*
* @return bytes written on success, -errno on failure.
*/
int QBBufferToFileWithLengthAndMode(const char *filename, const void *buffer, int bufferLength, mode_t mode);

/**
 * @}
**/

#endif
