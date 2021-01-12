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

#ifndef QB_DVB_SATELLITE_DESCRIPTOR_H_
#define QB_DVB_SATELLITE_DESCRIPTOR_H_

#include <QBTunerTypes.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <unistd.h> // for ssize_t


/**
 * Satellite descriptor class.
 **/
typedef struct QBDVBSatelliteDescriptor_ *QBDVBSatelliteDescriptor;

/**
 * Satellite transponder class.
 **/
struct QBDVBSatelliteTransponder_ {
    /// super
    struct SvObject_ super_;
    /// whether transponder is enabled (available for scanning)
    bool isEnabled;
    /// transponder's description in form of tuner parameters
    struct QBTunerParams desc;
};
typedef struct QBDVBSatelliteTransponder_ *QBDVBSatelliteTransponder;

/**
 * Resulting values of transponder verification.
 **/
enum QBDVBSatelliteTransponderCheck_e {
    QBDVBSatelliteTransponderCheck_correct,
    QBDVBSatelliteTransponderCheck_invalidFrequency = -1,
    QBDVBSatelliteTransponderCheck_invalidSymbolRate = -2,
    QBDVBSatelliteTransponderCheck_duplicate = -3,
    QBDVBSatelliteTransponderCheck_other = -4
};

/**
 * Type of transponders' class.
 * @return type of object
 **/
SvType
QBDVBSatelliteTransponder_getType(void);

/**
 * Create a transponder object.
 *
 * @param[in] isEnabled   whether transponder is enabled
 * @param[out] errorOut   error
 * @return transponder object
 **/
QBDVBSatelliteTransponder
QBDVBSatelliteTransponderCreate(bool isEnabled,
                                SvErrorInfo *errorOut);

/**
 * Compare transponders.
 *
 * @param[in] prv     opaque pointer to function's private data
 * @param[in] self_   first transponder
 * @param[in] other_  second transponder
 * @return 0  on equality of frequencies, otherwise a signed difference of them
 **/
int
QBDVBSatelliteTransponderCompare(void *prv,
                                SvObject self_,
                                SvObject other_);

/**
 * Get satellite name.
 *
 * @param[in] self satellite descriptor handle
 * @return satellite name or @c NULL in case of error
 **/
extern SvString
QBDVBSatelliteDescriptorGetName(QBDVBSatelliteDescriptor self);

/**
 * Get name of the file this descriptor was loaded from.
 *
 * @param[in] self satellite descriptor handle
 * @return file name or @c NULL in case of error
 **/
extern SvString
QBDVBSatelliteDescriptorGetFileName(QBDVBSatelliteDescriptor self);

/**
 * Get ID of the satellite.
 *
 * @param[in] self satellite descriptor handle
 * @return satellite ID or @c NULL in case of error
 **/
extern SvString
QBDVBSatelliteDescriptorGetID(QBDVBSatelliteDescriptor self);

/**
 * Get satellite position.
 *
 * @param[in] self satellite descriptor handle
 * @return satellite position in 1/10s of a degree east;
 *         values in range @c 0 to @c 3599, @c -1 in case of error
 **/
extern unsigned int
QBDVBSatelliteDescriptorGetPosition(QBDVBSatelliteDescriptor self);

/**
 * Get number of transponders on a satellite.
 *
 * @param[in] self satellite descriptor handle
 * @return transponders count, @c -1 in case of error
 **/
extern ssize_t
QBDVBSatelliteDescriptorGetTranspondersCount(QBDVBSatelliteDescriptor self);

/**
 * Get number of enabled transponders on a satellite.
 *
 * @param[in] self satellite descriptor handle
 * @return transponders count, @c -1 in case of error
 **/
extern ssize_t
QBDVBSatelliteDescriptorGetEnabledTranspondersCount(QBDVBSatelliteDescriptor self);

/**
 * Get all transponders.
 *
 * @param[in] self satellite descriptor handle
 * @return array of transponders, @c NULL in case of error
 **/
extern SvArray
QBDVBSatelliteDescriptorGetTransponders(QBDVBSatelliteDescriptor self);

/**
 * Get available transponders for given LNB parameters.
 *
 * @param[in] self satellite descriptor handle
 * @param[in] LNB LNB parameters
 * @param[in, out] availableTransponders array of tuner params for available transponders
 * @return number of available transponders, @c -1 in case of error
 **/
extern ssize_t
QBDVBSatelliteDescriptorGetAvailableTransponders(QBDVBSatelliteDescriptor self,
                                                 const struct QBTunerLNBParams *LNB,
                                                 SvArray availableTransponders);

/**
 * Calculate how many transponders can be received with given LNB parameters.
 *
 * @param[in] self satellite descriptor handle
 * @param[in] LNB LNB parameters
 * @return number of transponders that can be received, @c -1 in case of error
 **/
extern ssize_t
QBDVBSatelliteDescriptorGetAvailableTranspondersCount(QBDVBSatelliteDescriptor self,
                                                      const struct QBTunerLNBParams *LNB);

/**
 * Get first transponder that can be tuned for given LNB parameters.
 *
 * @param[in] self satellite descriptor handle
 * @param[in] LNB LNB parameters
 * @return transponder, @c NULL in case of error/not found
 **/
const struct QBTunerParams *
QBDVBSatelliteDescriptorGetFirstAvailableTransponder(QBDVBSatelliteDescriptor self,
                                                     const struct QBTunerLNBParams *LNB);

/**
 * Add new transponder.
 *
 * @param[in] self satellite descriptor handle
 * @param[in] transponder pointer to transponder to be added
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatelliteDescriptorAddTransponder(QBDVBSatelliteDescriptor self,
                                       QBDVBSatelliteTransponder transponder);

/**
 * Remove existing transponder.
 *
 * @param[in] self satellite descriptor handle
 * @param[in] transponder pointer to transponder to be removed
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatelliteDescriptorRemoveTransponder(QBDVBSatelliteDescriptor self,
                                          QBDVBSatelliteTransponder transponder);

/**
 * Edit transponder (replace the old transponder with new one).
 *
 * @param[in] self satellite descriptor handle
 * @param[in] oldTransponder pointer to old transponder
 * @param[in] newTransponder pointer to new transponder
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatelliteDescriptorChangeTransponders(QBDVBSatelliteDescriptor self,
                                           QBDVBSatelliteTransponder oldTransponder,
                                           QBDVBSatelliteTransponder newTransponder);

/**
 * Replace transponders in destination satellite with values from source satellite.
 *
 * @param[in] src source satellite descriptor handle
 * @param[in] dest destination satellite descriptor handle
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatelliteDescriptorReplaceTransponders(const QBDVBSatelliteDescriptor src,
                                            QBDVBSatelliteDescriptor dest);

/**
 * Enable transponder.
 *
 * @param[in] self satellite descriptor handle
 * @param[in] transponder pointer to transponder to be enabled
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatelliteDescriptorEnableTransponder(QBDVBSatelliteDescriptor self,
                                          QBDVBSatelliteTransponder transponder);

/**
 * Disable transponder.
 *
 * @param[in] self satellite descriptor handle
 * @param[in] transponder pointer to transponder to be disabled
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatelliteDescriptorDisableTransponder(QBDVBSatelliteDescriptor self,
                                           QBDVBSatelliteTransponder transponder);

/**
 * Change satellite name.
 *
 * @param[in] self satellite descriptor handle
 * @param[in] name new satellite name
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatelliteDescriptorSetName(QBDVBSatelliteDescriptor self,
                                SvString name);

/**
 * Change satellite position.
 *
 * @param[in] self satellite descriptor handle
 * @param[in] position new satellite position
 * @return on success zero is returned, @c negative value in case of error
 **/
extern int
QBDVBSatelliteDescriptorSetPosition(QBDVBSatelliteDescriptor self,
                                    unsigned int position);

/**
 * Sort satellites by position.
 *
 * @param[in] descList satellites descriptors list
 **/
extern void
QBDVBSatelliteDescriptorSortByPosition(SvArray descList);

/**
 * Sort satellites by name position.
 *
 * @param[in] descList satellites descriptors list
 **/
extern void
QBDVBSatelliteDescriptorSortByName(SvArray descList);

/**
 * Compare satellites by position.
 *
 * @param[in] descA satellite descriptor handle
 * @param[in] descB satellite descriptor handle
 * @return an integer less than, equal to, or greater than zero if descA position is found,
 * respectively, to be less than, to match, or be greater than descB position
 **/
extern int
QBDVBSatelliteDescriptorCompareByPosition(QBDVBSatelliteDescriptor descA,
                                          QBDVBSatelliteDescriptor descB);

/**
 * Compare satellites by name.
 *
 * @param[in] descA satellite descriptor handle
 * @param[in] descB satellite descriptor handle
 * @return an integer less than, equal to, or greater than zero if descA name is found,
 * respectively, to be less than, to match, or be greater than descB name
 **/
extern int
QBDVBSatelliteDescriptorCompareByName(QBDVBSatelliteDescriptor descA,
                                      QBDVBSatelliteDescriptor descB);

#endif // QB_DVB_SATELLITE_DESCRIPTOR_H_
