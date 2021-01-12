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

#ifndef QB_PHYS_MEM_SEGMENT_H_
#define QB_PHYS_MEM_SEGMENT_H_

/**
 * @file QBPhysMemSegment.h
 * @brief Physical memory segment class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stddef.h>
#include <sys/types.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPhysMemSegment Physical memory segment class
 * @ingroup QBSHMem
 * @{
 **/

/**
 * Physical memory segment class.
 * @class QBPhysMemSegment
 * @extends QBBasicMemSegment
 **/
typedef struct QBPhysMemSegment_ *QBPhysMemSegment;


/**
 * Get runtime type identification object representing
 * physical memory segment class.
 *
 * @return physical memory segment class
 **/
extern SvType
QBPhysMemSegment_getType(void);

/**
 * Create physical memory segment object.
 *
 * @memberof QBPhysMemSegment
 *
 * @param[in] offset    offset of the segment in physical memory space,
 *                      in bytes (must be a multiple of page size)
 * @param[in] length    length of the segment, in bytes (it will be rounded
 *                      up to a multiple of page size)
 * @param[in] rights    UNIX access rights to the segment
 * @param[out] errorOut error info
 * @return              created segment or @c NULL in case of error
 **/
extern QBPhysMemSegment
QBPhysMemSegmentCreate(off_t offset, size_t length,
                       unsigned int rights,
                       SvErrorInfo *errorOut);

/**
 * Initialize physical memory segment object
 * by mapping an area of physical memory space.
 *
 * @memberof QBPhysMemSegment
 *
 * @param[in] self      physical memory segment handle
 * @param[in] offset    offset of the segment in physical memory space,
 *                      in bytes (must be a multiple of page size)
 * @param[in] length    length of the segment, in bytes (it will be rounded
 *                      up to a multiple of page size)
 * @param[in] rights    UNIX access rights to the segment
 * @param[out] errorOut error info
 * @return              @a self or @c NULL in case of error
 **/
extern QBPhysMemSegment
QBPhysMemSegmentInit(QBPhysMemSegment self,
                     off_t offset, size_t length,
                     unsigned int rights,
                     SvErrorInfo *errorOut);

/**
 * Get address of the segment in the physical memory space.
 *
 * @memberof QBPhysMemSegment
 *
 * @param[in] self      physical memory segment handle
 * @return              physical segment address, @c -1 in case of error
 **/
extern off_t
QBPhysMemSegmentGetAddress(QBPhysMemSegment self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
