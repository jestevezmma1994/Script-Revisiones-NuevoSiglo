/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2011-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_DVB_SATELLITES_DB_H_
#define QB_DVB_SATELLITES_DB_H_

/**
 * @file QBDVBSatellitesDB.h database with satellites
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <QBDVBSatelliteDescriptor.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvImmutableArray.h>
#include <SvFoundation/SvString.h>
#include <fibers/c/fibers.h>
#include <unistd.h> // for ssize_t
#include <stdbool.h>

/**
 * Satellites database class.
 **/
typedef struct QBDVBSatellitesDB_ *QBDVBSatellitesDB;

/**
 * Get runtime type identification object representing QBDVBSatellitesDB class.
 *
 * @return QBDVBSatellitesDB runtime type identification object
 **/
SvType
QBDVBSatellitesDB_getType(void);

/**
 * Create empty database.
 *
 * @param[out] errorOut error info
 * @return created satellites database, @c NULL in case of error
 **/
extern QBDVBSatellitesDB
QBDVBSatellitesDBCreate(SvErrorInfo *errorOut);

/**
 * Start loading database from default location.
 *
 * @param[in] self satellites database handle
 * @param[in] scheduler scheduler handle
 * @param[out] errorOut error info
 **/
extern void
QBDVBSatellitesDBLoad(QBDVBSatellitesDB self,
                      SvScheduler scheduler,
                      SvErrorInfo *errorOut);

/**
 * Check if database has been loaded.
 *
 * @param[in] self satellites database handle
 * @return @c true if database is ready to be used
 **/
extern bool
QBDVBSatellitesDBIsLoaded(QBDVBSatellitesDB self);

/**
 * Get number of known satellites.
 *
 * @param[in] self satellites database handle
 * @return satellites count, @c -1 in case of error
 **/
extern ssize_t
QBDVBSatellitesDBGetSatellitesCount(QBDVBSatellitesDB self);

/**
 * Get array of all satellites.
 *
 * @param[in] self satellites database handle
 * @return array of @ref QBDVBSatelliteDescriptor objects,
 *         @c NULL in case of error
 **/
extern SvImmutableArray
QBDVBSatellitesDBGetSatellites(QBDVBSatellitesDB self);

/**
 * Find satellite descriptor by ID.
 *
 * @param[in] self satellites database handle
 * @param[in] ID satellite ID
 * @return descriptor for satellite with @a ID, @c NULL if not found
 **/
extern QBDVBSatelliteDescriptor
QBDVBSatellitesDBGetSatelliteByID(QBDVBSatellitesDB self,
                                    SvString ID);

/**
 * Create list of satellite descriptors only for satellites which have specified position.
 *
 * @param[in] self satellites database handle
 * @param[in] position satellite position (shall be normalized to 360 degrees)
 * @return list of satellite descriptors or @c NULL if not found
 **/
extern SvArray
QBDVBSatellitesDBCreateListOfSatellitesByPosition(QBDVBSatellitesDB self,
                                                  unsigned int position);

/**
 * Create new satellite file.
 *
 * Creates new file with empty set of transponders
 *
 * @param[in] self satellites database handle
 * @param[in] name satellite name
 * @param[in] longitude satellite position in seconds
 * @param[out] errorOut error info
 * @return descriptor for new satellite, @c NULL if error
 **/
extern QBDVBSatelliteDescriptor
QBDVBSatellitesDBCreateEntry(QBDVBSatellitesDB self,
                             SvString name,
                             unsigned int longitude,
                             SvErrorInfo *errorOut);

/**
 * Remove satellite.
 *
 * @param[in] self satellites database handle
 * @param[in] desc satellite descriptor
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatellitesDBRemoveEntry(QBDVBSatellitesDB self,
                             QBDVBSatelliteDescriptor desc);

/**
 * Save satellite.
 *
 * @param[in] self satellites database handle
 * @param[in] desc satellite descriptor
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatellitesDBSaveEntry(QBDVBSatellitesDB self,
                           QBDVBSatelliteDescriptor desc);

/**
 * Restore satellite transponders defaults.
 *
 * @param[in] self satellites database handle
 * @param[in] desc satellite descriptor
 * @return on success positive value, @c negative value in case of error
 **/
extern int
QBDVBSatellitesDBRestoreSatellite(QBDVBSatellitesDB self,
                                          QBDVBSatelliteDescriptor desc);

/**
 * Set directory path for satellites data.
 *
 * @param[in] self satellites database handle
 * @param[in] path new path
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatellitesDBSetDefaultDBDir(QBDVBSatellitesDB self,
                                 SvString path);

/**
 * Set directory path for storing modified satellites data.
 *
 * @param[in] self satellites database handle
 * @param[in] path new path
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatellitesDBSetUserDBDir(QBDVBSatellitesDB self,
                              SvString path);

/**
 * Set default satellites.
 *
 * @param[in]  self satellites database handle
 * @param[in]  defaultSatelliteID default satellite ID of type SvString
 * @param[out] errorOut error info handle
 *
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatellitesDBAddDefaultSatelliteByID(QBDVBSatellitesDB self, SvString defaultSatelliteID,
        SvErrorInfo* errorOut);

/**
 * Check if given satelliteID corresponds to a default satellite.
 *
 * @param[in]  self satellites database handle
 * @param[in]  satelliteID satellite ID to check
 * @return @c true if satelliteID corresponds to a default satellite
 **/
extern bool
QBDVBSatellitesDBIsDefaultSatellite(QBDVBSatellitesDB self, SvString satelliteID);

#endif // QB_DVB_SATELLITES_DB_H_
