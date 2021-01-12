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

#ifndef _SV_SMP_XTREAM_STORAGE_H_
#define _SV_SMP_XTREAM_STORAGE_H_

#include <stdint.h>

/// -- Device API

// == Common device handle
typedef struct xtream_device *x_dev;
/*
Device handle used by all functions
*/

// == Close an opened device
void xtream_close_device(x_dev dev);
/*
This will free all resources associated with the device.
Further operation on a closed handle will not be possible
*/

// == Open a storage device
x_dev xtream_open_device(const char *path, int64_t offset, int64_t dev_size, uint32_t dev_align);
/*
Returns device handle or NULL if the given path does not exist or is an invalid (unformatted) device.
*/
// == Format a storage device
int xtream_format_device(const char *path, uint64_t guid, uint32_t blksize, uint32_t fileratio,
                         int64_t offset, int64_t dev_size, uint32_t dev_align);
int xtream_format_device2(const char* path, uint32_t block_size, uint32_t file_cnt,
                          int64_t offset, int64_t dev_size);
/*
Initializes the storage structures used by xtream API.
A GUID may be specified for device identification.
Block size may be set by the user or automatically assigned if 0 (default is 2MB).
File ratio should be set to the minimum average object size in blocks.
*/

/** flush all meta-info into disk
 */
void xtream_sync(x_dev dev);


/// -- Device information
// == Get single block size
uint32_t xtream_get_blocksize(x_dev dev);
// == Get capacity (total size)
uint64_t xtream_get_capacity(x_dev dev);
// == Get used size
uint64_t xtream_get_usage(x_dev dev);
// == Get available space
uint64_t xtream_get_freespace(x_dev dev);

int xtream_get_fd(x_dev dev);

void xtream_print(x_dev dev);


/// -- Object operations
/*
Objects are represented as 32-bit identifiers.
These identifiers are device-specific
*/

#define XTREAM_FILETYPE_NORMAL     ((uint32_t)0x1)
#define XTREAM_FILETYPE_TEMPORARY  ((uint32_t)0x80000001)
#define XTREAM_GUID_TEMPORARY(id)  ( (((uint64_t)(id)) << 32) | XTREAM_FILETYPE_TEMPORARY )

// == Create a new object
uint32_t xtream_create_object(x_dev dev, uint64_t guid, uint32_t type);
/*
Automatically assigns a new ID for the object.
A 64-bit GUID and 32-bit file type can be associated with the ID,
these can be used later to locate the xtream IDs.
*/

uint32_t xtream_create_temp_object(x_dev dev);
/*
Temporary objects are being deleted upon device opening.
*/


// == Pre-allocate an object
uint32_t xtream_allocate_object(x_dev dev, uint64_t guid, uint32_t type,uint64_t size);
/*
Creates an object and allocates specified size. If there is
not enough space available, returns 0.
*/

// == Flush object
uint32_t xtream_flush_object(x_dev dev, uint32_t id, uint64_t size);
/*
Stores the object metadata on the device. User has to provide the
effective file size.
*/

/// similar to \a xtream_flush_object, but more lightweight.
/// used to update file size while it is still being used.
void xtream_set_object_size(x_dev dev, uint32_t id, uint64_t size);


// == Lookup object
uint32_t xtream_lookup_object(x_dev dev, uint64_t guid, uint32_t type);
/*
Uses the previously provided GUID and type to lookup the xtream
object ID
*/

// == Delete object
int xtream_delete_object(x_dev dev, uint32_t id);
/*
Deletes the object specified by id and frees up the space used
*/

// == Find an unused object ID
uint32_t xtream_get_unused_object_id(x_dev dev);
/*
Allows to assign and handle IDs externally instead of using GUID lookups.
*/

/// Returns the object size (as set using flush, allocate or set_object_size)
uint64_t xtream_get_object_size(x_dev dev, uint32_t id);
uint64_t xtream_get_object_physical_size(x_dev dev, uint32_t id);


typedef int (xtream_test_remove_fun)(void* arg, uint32_t id);

uint32_t xtream_remove_objects(x_dev dev, xtream_test_remove_fun* test_fun, void* test_arg);

/// -- Reading and writing
/*
Xtream is a block-oriented API. There are simple functions
that allow to write continuous parts of data, but the block
api gives much more control over the whole process.
*/

#if 0
// == Read block
uint32_t xtream_request_block(x_dev dev, uint32_t id, uint32_t block, char *buffer);
/*
Read complete block specified by [<block>] parameter into [<buffer>].
Returns block size or 0 on error.
*/

// == Write block
uint32_t xtream_commit_block(x_dev dev, uint32_t id, uint32_t block, char *buffer);
/*
Write complete block specified by [<block>] parameter from [<buffer>].
Returns block size or 0 on error.
*/
#endif

// == Read data
uint32_t xtream_request_data(x_dev dev, uint32_t id, uint64_t offset, char *buffer, uint32_t count);
/*
Read [<count>] bytes data starting at [<offset>]
Returns [<count>] or 0 on error.
*/

uint32_t xtream_commit_data(x_dev dev, uint32_t id, uint64_t offset, const char *buffer, uint32_t count);
/*
Write [<count>] bytes data starting at [<offset>]
Returns [<count>] or 0 on error.
*/

uint32_t xtream_reserve_space(x_dev dev, uint32_t object_id, uint64_t object_off, uint32_t count,
                              uint64_t* offset_tab, uint32_t* size_tab, int* tab_cnt);

uint32_t xtream_get_device_offset(x_dev dev, uint32_t object_id, uint64_t object_off, uint32_t count,
                                  uint64_t* offset_tab, uint32_t* size_tab, int* tab_cnt);


#endif
