/*
 * Out-of-memory malloc for MTD/UBI/UBIFS drivers and users.
 *
 * Copyright © 2014 Copyright (C) 2008-2014 Cubiware Sp. z o.o.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef __OOMALLOC_H__
#define __OOMALLOC_H__

/**
 * @file oomalloc.h Out-of-memory malloc for MTD/UBI/UBIFS drivers and users
 **/

#include <linux/slab.h>
#include <linux/types.h>

/**
 * @defgroup OOMalloc OOMalloc: Out-of-memory malloc for MTD/UBI/UBIFS drivers and users
 * @{
 *
 * OOMalloc is the kmalloc() wrapper that keeps a MAX_KMALLOC_SIZE bytes memory reserve.
 * It tries to obtain memory for caller using kernel interface, if that fails it returns
 * its reserve.
 */

/**
 * Allocate kernel memory or return own reserve in case of memory shortage.
 *
 * @param[in] size      amount of memory to allocate in bytes
 * @param[in] flags     SLAB flags to be passed to real allocator;
 *                      ignored if reserve is used
 * @param[in] function  name of function calling this function
 * @param[in] file      name of file containing caller source
 * @param[in] line      number of line of file containing caller
 * @return              @c pointer to allocated memory, otherwise pointer to oomalloc reserve
                        or NULL if neither available bufferotherwise NULLany other value
 **/
void *_oomalloc(size_t size, gfp_t flags, const char *function, const char *file, int line);

/**
 * Free kernel memory or mark reserve buffer no longer in use.
 *
 * @param[in] ptr       pointer to allocated memory
 * @param[in] function  name of function calling this function
 * @param[in] file      name of file containing caller source
 * @param[in] line      number of line of @file containing caller
 **/
void _oomfree(const void *ptr, const char *function, const char *file, int line);

/**
 * Helper macro for _oomalloc that fills in function, file and line parameters.
 */
#define oomalloc(size, flags) _oomalloc(size, flags, __func__, __FILE__, __LINE__)

/**
 * Helper macro for _oomfree that fills in function, file and line parameters.
 **/
#define oomfree(ptr) _oomfree(ptr, __func__, __FILE__, __LINE__)

/**
 * @}
 **/

#endif
