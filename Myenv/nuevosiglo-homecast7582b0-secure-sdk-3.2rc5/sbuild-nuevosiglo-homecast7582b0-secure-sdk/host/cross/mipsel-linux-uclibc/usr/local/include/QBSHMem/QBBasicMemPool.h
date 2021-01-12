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

#ifndef QB_BASIC_MEM_POOL_H_
#define QB_BASIC_MEM_POOL_H_

/**
 * @file QBBasicMemPool.h
 * @brief Basic memory pool class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvWeakList.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBBasicMemPool Basic memory pool class
 * @ingroup QBSHMem
 * @{
 **/

/**
 * Basic memory pool class.
 * @class QBBasicMemPool
 * @extends SvObject
 **/
typedef struct QBBasicMemPool_ {
    /// base class
    struct SvObject_ super_;
    /// aggregate length of all allocated segments in the pool, in bytes
    size_t totalLength;
    /// max length of all segments in the pool, in bytes
    size_t maxLength;
    /// max number of segments in the pool
    unsigned int maxSegments;
    /// all allocated segments
    SvWeakList segments;
} *QBBasicMemPool;


/**
 * Get runtime type identification object representing
 * basic memory pool class.
 *
 * @return basic memory pool class
 **/
extern SvType
QBBasicMemPool_getType(void);

/**
 * Initialize basic memory pool.
 *
 * @memberof QBBasicMemPool
 *
 * @param[in] self         basic memory pool object handle
 * @param[in] maxSegments  max number of memory segments to allocate,
 *                         @c 0 for unlimited
 * @param[in] maxLength    max total length of allocated segments,
 *                         @c 0 for unlimited
 * @param[out] errorOut    error info
 * @return                 @a self or @c NULL in case of error
 **/
extern QBBasicMemPool
QBBasicMemPoolInit(QBBasicMemPool self,
                   unsigned int maxSegments,
                   size_t maxLength,
                   SvErrorInfo *errorOut);

/**
 * Find segment containing given @a address.
 *
 * @memberof QBBasicMemPool
 *
 * @param[in] self         basic memory pool object handle
 * @param[in] address      memory address
 * @return                 segment containing @a address, @c NULL if not found
 **/
extern SvObject
QBBasicMemPoolFindSegment(QBBasicMemPool self,
                          void *address);

/**
 * Get total length of all segments in the pool.
 *
 * @memberof QBBasicMemPool
 *
 * @param[in] self         basic memory pool object handle
 * @return                 total length in bytes, @c -1 in case of error
 **/
extern size_t
QBBasicMemPoolGetTotalLength(QBBasicMemPool self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
