/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_SHMEM_POOL_H_
#define QB_SHMEM_POOL_H_

/**
 * @file QBSHMemPool.h
 * @brief Shared memory pool class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBSharedMemoryPool Shared memory pool class
 * @ingroup QBSHMem
 * @{
 **/

/**
 * Shared memory pool class.
 *
 * This class is an implementation of the @ref QBMemPool
 * that allocates shared memory segments.
 *
 * @class QBSHMemPool
 * @extends QBBasicMemPool
 **/
typedef struct QBSHMemPool_ *QBSHMemPool;


/**
 * Get runtime type identification object representing
 * shared memory pool class.
 *
 * @return shared memory pool class
 **/
extern SvType
QBSHMemPool_getType(void);

/**
 * Initialize shared memory pool.
 *
 * @memberof QBSHMemPool
 *
 * @param[in] self         anonymous memory pool object handle
 * @param[in] maxSegments  max number of memory segments to allocate,
 *                         @c 0 for unlimited
 * @param[in] maxLength    max total length of allocated segments,
 *                         @c 0 for unlimited
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern QBSHMemPool
QBSHMemPoolInit(QBSHMemPool self,
                unsigned int maxSegments,
                size_t maxLength,
                SvErrorInfo *errorOut);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
