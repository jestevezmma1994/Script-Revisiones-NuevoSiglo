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

#ifndef QB_PVR_DIRECTORY_INTERNAL_H_
#define QB_PVR_DIRECTORY_INTERNAL_H_

/**
 * @file QBPVRDirectory.h Virtual pvr provider directory class internal methods file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include "QBPVRDirectory.h"
#include <QBPVRTypes.h>

#include <SvCore/SvErrorInfo.h>

/**
 * @addtogroup QBPVRDirectory
 * @{
 **/

/**
 * QBPVRDirectory init params.
 **/
typedef struct QBPVRDirectoryInitParams_ {
    SvString id;                                ///< id of the category, must be unique in the tree
    /// name of the category, it must be user-displayable as
    /// title is set to this value.
    SvString name;
    SvString channelId;                         ///< channel id
    SvString channelName;                       ///< channel name
    QBContentCategory parent;                   ///< handle to parent category
    QBPVRDirectoryType type;                    ///< directory type
    int64_t quota;                              ///< quota, @c 0 in no quota
    QBPVRRepeatedRecordingsInfo repeatedInfo;   ///< repeated recordings info
    SvString keyword;                           ///< keyword
    SvString seriesID;                          ///< series identifier

    bool addSearch;                                ///< should search node be added to directory
    QBPVRSearchProviderTest searchProviderTest;    ///< search provider test associated data
} *QBPVRDirectoryInitParams;

/**
 * Init PVR directory.
 *
 * @memberof QBPVRDirectory
 *
 * @param[in] self              QBPVRDirectory handle
 * @param[in] initParams        initialize params
 * @param[out] errorOut         error info
 * @return @c <0 in case of error, @c 0 otherwise
 **/
int
QBPVRDirectoryInit(QBPVRDirectory self,
                   QBPVRDirectoryInitParams initParams,
                   SvErrorInfo *errorOut);

/**
 * Copy PVR directory.
 *
 * @memberof QBPVRDirectory
 *
 * @param[in] dest              destination handle
 * @param[in] src               source handle
 * @param[out] errorOut         error info
 * @return @c <0 in case of error, @c 0 otherwise
 **/
int
QBPVRDirectoryCopy(QBPVRDirectory dest,
                   QBPVRDirectory src,
                   SvErrorInfo *errorOut);

/**
 * Notify that directory has been changed.
 *
 * @memberof QBPVRDirectory
 *
 * @param[in] self          directory handle
 */
void
QBPVRDirectoryNotifyChanged(QBPVRDirectory self);

/**
 * @}
 **/

#endif /* QB_PVR_DIRECTORY_INTERNAL_H_ */
