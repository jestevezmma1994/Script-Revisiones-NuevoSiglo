/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_POSIX_SHM_SEGMENT_H_
#define QB_POSIX_SHM_SEGMENT_H_

/**
 * @file QBPOSIXSHMSegment.h
 * @brief POSIX shared memory segment class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBPOSIXSHMSegment POSIX shared memory segment class
 * @ingroup QBSHMem
 * @{
 **/

/**
 * POSIX shared memory segment class.
 * @class QBPOSIXSHMSegment
 * @extends SvData
 * @implements QBMemSegment
 **/
typedef struct QBPOSIXSHMSegment_ *QBPOSIXSHMSegment;


/**
 * Get runtime type identification object representing
 * POSIX shared memory segment class.
 *
 * @return POSIX shared memory segment class
 **/
extern SvType
QBPOSIXSHMSegment_getType(void);

/**
 * Get POSIX shared memory segment descriptor.
 *
 * @memberof QBPOSIXSHMSegment
 *
 * @param[in] self         POSIX shared memory segment handle
 * @return                 POSIX shared memory segment descriptor
 **/
extern int
QBPOSIXSHMSegmentGetDescriptor(QBPOSIXSHMSegment self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
