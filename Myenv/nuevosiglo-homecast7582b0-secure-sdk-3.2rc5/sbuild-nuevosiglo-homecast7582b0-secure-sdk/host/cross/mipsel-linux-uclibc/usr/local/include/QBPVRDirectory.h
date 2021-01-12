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

#ifndef QB_PVR_DIRECTORY_H_
#define QB_PVR_DIRECTORY_H_

/**
 * @file QBPVRDirectory.h Virtual pvr provider directory class file.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBPVRTypes.h>

#include <QBContentManager/QBContentCategoryInternal.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvWeakList.h>

/**
 * @defgroup QBPVRDirectory QBPVRDirectory: virtual PVR provider directory
 * @ingroup QBPVRProvider
 * @{
 **/

/**
 * PVR directory type.
 */
typedef enum {
    QBPVRDirectoryType_unknown,         ///< unknown directory type
    QBPVRDirectoryType_normal,          ///< normal directory type
    QBPVRDirectoryType_keyword,         ///< keyword directory type
    QBPVRDirectoryType_series,          ///< keyword directory type
    QBPVRDirectoryType_repeated,        ///< repeated recordings directory type
} QBPVRDirectoryType;

/**
 * Get directory type from name.
 *
 * @param[in] str           directory type name
 * @return                  directory type
 */
QBPVRDirectoryType
QBPVRDirectoryTypeFromName(const char *str);

/**
 * Create directory type name string.
 *
 * @param[in] type          directory type
 * @return                  directory type name string
 */
SvString
QBPVRDirectoryTypeCreateName(QBPVRDirectoryType type);

/**
 * QBPVR directory.
 * @class QBPVRDirectory
 * @extends QBContentCategory
 **/
typedef struct QBPVRDirectory_ {
    struct QBContentCategory_ super_;           ///< super type

    QBPVRDirectoryType type;                    ///< directory type
    int64_t quotaLimit;                         ///< quota limit, @c -1 if unknown
    SvString channelId;                         ///< channel id
    SvString channelName;                       ///< channel name
    union {
        QBPVRRepeatedRecordingsInfo repeatedInfo;   ///< repetition specifying data for repeated recordings
        SvString keyword;                           ///< keyword for keyword recordings
        SvString seriesID;                          ///< series ID for series recordings
    };                                          ///< data specific to directory type

    SvWeakList listeners;                       ///< listeners
} *QBPVRDirectory;

/**
 * Get runtime type identification object representing QBPVRDirectory class.
 *
 * @memberof QBPVRDirectory
 *
 * @return QBPVRDirectory runtime type identification object
 **/
extern SvType
QBPVRDirectory_getType(void);

/**
 * QBPVRDirectory listener.
 **/
typedef const struct QBPVRDirectoryListener_ {
    /**
     * Directory changed.
     *
     * @param[in] self          listener handle
     * @param[in] directory     directory handle
     **/
    void (*changed)(SvObject self, QBPVRDirectory directory);
} *QBPVRDirectoryListener;

/**
 * Get runtime type identification object representing
 * QBPVRDirectoryListener interface.
 *
 * @return QBPVRDirectoryListener interface object
**/
extern SvInterface
QBPVRDirectoryListener_getInterface(void);

/**
 * Add QBPVRDirectory listener.
 *
 * @memberof QBPVRDirectory
 *
 * @param[in] self          directory handle
 * @param[in] listener      listener object
 */
void
QBPVRDirectoryAddListener(QBPVRDirectory self, SvObject listener);

/**
 * Remove QBPVRDirectory listener.
 *
 * @memberof QBPVRDirectory
 *
 * @param[in] self          directory handle
 * @param[in] listener      listener object
 */
void
QBPVRDirectoryRemoveListener(QBPVRDirectory self, SvObject listener);

/**
 * @}
 **/

#endif /* QB_PVR_DIRECTORY_H_ */
