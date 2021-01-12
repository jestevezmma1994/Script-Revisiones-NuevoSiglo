/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2012 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_FIRMWARE_VERSION_H_
#define QB_FIRMWARE_VERSION_H_

/**
 * @file QBFirmwareVersion.h
 * @brief Firmware version class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBFirmwareVersion Firmware version class
 * @ingroup QBUpgrade
 * @{
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>


/**
 * Firmware version class.
 * @class QBFirmwareVersion
 * @extends SvObject
 **/
typedef struct QBFirmwareVersion_ {
    /// super class
    struct SvObject_ super_;

    /// full firmware version as string
    SvString fullVersion;

    /// @c true for CURRENT version, otherwise @c false
    bool isCurrent;
    /// major version, @c 0 if @a isCurrent is @a true
    unsigned int major;
    /// minor version, @c 0 if @a isCurrent is @a true
    unsigned int minor;
    /// path version, @c 0 if @a isCurrent is @a true
    unsigned int patch;
} *QBFirmwareVersion;


/**
 * Get runtime type identification object
 * representing QBFirmwareVersion class.
 *
 * @return QBFirmwareVersion type identification object
 **/
extern SvType
QBFirmwareVersion_getType(void);

/**
 * Create firmware version instance from string representation.
 *
 * @memberof QBFirmwareVersion
 *
 * @param[in] versionStr string representation of firmware version
 * @param[out] errorOut error info
 * @return              created version object, @c NULL in case of error
 **/
extern QBFirmwareVersion
QBFirmwareVersionCreate(SvString versionStr,
                        SvErrorInfo *errorOut);

/**
 * Compare two firmware versions.
 *
 * @memberof QBFirmwareVersion
 *
 * @param[in] self      firmware version handle to compare with @a other
 * @param[in] other     firmware version to compare @a self with
 * @return              @c -1 if @a self version is older than @a other
 *                      (also when @a other is CURRENT), @c 0 if @a self
 *                      and @a other is the same version, @c 1 of @a self version
 *                      is newer than @a other (also when @a self is CURRENT)
 **/
extern int
QBFirmwareVersionCompare(QBFirmwareVersion self,
                         QBFirmwareVersion other);

/**
 * @}
 **/


#endif
