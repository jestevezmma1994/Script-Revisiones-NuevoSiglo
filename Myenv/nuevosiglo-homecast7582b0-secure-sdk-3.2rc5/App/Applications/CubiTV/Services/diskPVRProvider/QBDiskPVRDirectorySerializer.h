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

#ifndef QB_DISK_PVR_DIRECTORY_SERIALIZER_H_
#define QB_DISK_PVR_DIRECTORY_SERIALIZER_H_

/**
 * @file QBDiskPVRDirectorySerializer.h Disk pvr directory serializer file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBPVRTypes.h>
#include <QBContentManager/QBContentCategory.h>
#include <SvCore/SvErrorInfo.h>

#include <stdbool.h>

/**
 * @defgroup QBDiskPVRDirectorySerializer Disk pvr directory serializer.
 * @ingroup QBDiskPVRProvider
 * @{
 **/

/**
 * Disk PVR directory serializer is responsible for serializing and
 * deserializing directories.
 *
 * @class QBDiskPVRDirectorySerializer QBDiskPVRDirectorySerializer.h "QBDiskPVRDirectorySerializer.h"
 * @implements SvJSONClassHelper
 * @extends SvObject
 **/
typedef struct QBDiskPVRDirectorySerializer_ *QBDiskPVRDirectorySerializer;

/**
 * Create disk PVR directory serializer.
 *
 * @memberof QBDiskPVRDirectorySerializer
 *
 * @param[in] parent                    parent category for directories
 * @param[in] addSearchToDirectories    should search node be added to directories
 * @param[in] searchProviderTest        search provider test associated data
 * @param[out] errorOut                 error info
 * @return                              created directory serializer, @c NULL in case of error
 **/
QBDiskPVRDirectorySerializer
QBDiskPVRDirectorySerializerCreate(QBContentCategory parent,
                                   bool addSearchToDirectories,
                                   QBPVRSearchProviderTest searchProviderTest,
                                   SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif /* QB_DISK_PVR_DIRECTORY_SERIALIZER_H_ */
