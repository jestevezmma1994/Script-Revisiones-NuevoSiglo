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

#ifndef QB_SYSV_SHM_SEGMENT_H_
#define QB_SYSV_SHM_SEGMENT_H_

/**
 * @file QBSysVSHMSegment.h
 * @brief System V shared memory segment class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBSysVSHMSegment System V shared memory segment class
 * @ingroup QBSHMem
 * @{
 **/

/**
 * System V shared memory segment class.
 * @class QBSysVSHMSegment
 * @extends SvData
 * @implements QBMemSegment
 **/
typedef struct QBSysVSHMSegment_ *QBSysVSHMSegment;


/**
 * Get runtime type identification object representing
 * system V shared memory segment class.
 *
 * @return system V shared memory segment class
 **/
extern SvType
QBSysVSHMSegment_getType(void);

/**
 * Get system V shared memory segment identifier.
 *
 * @memberof QBSysVSHMSegment
 *
 * @param[in] self         system V shared memory segment handle
 * @return                 system V shared memory segment identifier
 **/
extern int
QBSysVSHMSegmentGetSHMID(QBSysVSHMSegment self);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
