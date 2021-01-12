/*******************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2006 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau, application
** service provider, or similar business, or make any other use of this Software
** without express written permission from Sentivision K.K.

** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product
**     for sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
*******************************************************************************/

#ifndef SvFoundation_SvData_h
#define SvFoundation_SvData_h

/**
 * @file SvData.h Data class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvData Data class
 * @ingroup SvFoundationCoreClasses
 * @{
 **/

/**
 * Data class.
 * @class SvData
 * @extends SvObject
 **/
typedef struct SvData_ *SvData;

/**
 * SvData class internals.
 * @private
 **/
struct SvData_ {
    /// super class
    struct SvObject_ super_;
    /**
     * size of data buffer in bytes
     * @private
     **/
    size_t length;
    /**
     * data buffer
     * @private
     **/
    char *base;
    /**
     * parent object
     * @private
     **/
    SvData parent;
};


/**
 * Get runtime type identification object representing SvData class.
 *
 * @return data class
 **/
extern SvType
SvData_getType(void);

/**
 * Create an uninitialized data object.
 *
 * @memberof SvData
 *
 * @param[in] length size of data object in bytes
 * @param[out] errorOut error info
 * @return created data object, @c NULL in case of error
 **/
extern SvData
SvDataCreateWithLength(size_t length,
                       SvErrorInfo *errorOut);

/**
 * Create a data object with a copy of data from a buffer.
 *
 * @memberof SvData
 *
 * @param[in] bytes data buffer to be copied to created data object
 * @param[in] length size of @a bytes buffer
 * @param[out] errorOut error info
 * @return created data object, @c NULL in case of error
 **/
extern SvData
SvDataCreateWithBytesAndLength(const char *bytes,
                               size_t length,
                               SvErrorInfo *errorOut);

/**
 * Create a data object as a subrange of a parent's data object.
 *
 * @memberof SvData
 *
 * @param[in] parent parent data handle
 * @param[in] offset offset in bytes inside parent's data
 * @param[in] length length in bytes
 * @param[out] errorOut error info
 * @return created data object, @c NULL in case of error
 **/
extern SvData
SvDataCreateSubrange(SvData parent,
                     size_t offset,
                     size_t length,
                     SvErrorInfo *errorOut);

/**
 * Create a data object from contents of an opened file.
 *
 * @memberof SvData
 * @since 1.12
 *
 * @param[in] fd file descriptor opened for reading
 * @param[in] maxLength max allowed file size in bytes, @c -1 for default value
 * @param[out] errorOut error info
 * @return created data object, @c NULL in case of error
 **/
extern SvData
SvDataCreateFromFD(int fd,
                   off_t maxLength,
                   SvErrorInfo *errorOut);

/**
 * Create a data object from file contents.
 *
 * @memberof SvData
 * @since 1.12
 *
 * @param[in] path path to a file to read data from
 * @param[in] maxLength max allowed file size in bytes, @c -1 for default value
 * @param[out] errorOut error info
 * @return created data object, @c NULL in case of error
 **/
extern SvData
SvDataCreateFromFile(const char *path,
                     off_t maxLength,
                     SvErrorInfo *errorOut);

/**
 * Get the size of the data object.
 *
 * @memberof SvData
 *
 * @param[in] self data handle
 * @return size of data object in bytes
 **/
extern size_t
SvDataGetLength(SvData self);

/** @cond */
static inline size_t
SvDataLength(SvData self)
{
    return SvDataGetLength(self);
}
/** @endcond */

/**
 * Get the content of the data object as byte buffer.
 *
 * @memberof SvData
 *
 * @param[in] self data handle
 * @return direct pointer to data stored in data object
 **/
extern char *
SvDataGetBytes(SvData self);

/** @cond */
static inline char *
SvDataBytes(SvData self)
{
    return SvDataGetBytes(self);
}
/** @endcond */

/**
 * Write the contents of the data object to a file.
 *
 * @memberof SvData
 * @since 1.12
 *
 * @param[in] self data handle
 * @param[in] path path to a file to write data to
 * @param[in] overwrite @c true to overwrite existing file, @c false to fail if already exists
 * @param[out] errorOut error info
 **/
extern void
SvDataWriteToFile(SvData self,
                  const char *path,
                  bool overwrite,
                  SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
