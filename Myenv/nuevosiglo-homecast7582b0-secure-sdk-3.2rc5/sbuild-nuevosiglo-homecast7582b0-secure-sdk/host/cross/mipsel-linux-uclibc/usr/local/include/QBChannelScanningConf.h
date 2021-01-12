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


#ifndef QBCHANNELSCANNINGCONF_H_
#define QBCHANNELSCANNINGCONF_H_

/**
 * @file QBChannelScanningConf.h
 *
 * @brief Channel scanning configuration
 **/

#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBDVBScanner.h>
#include <tunerReserver.h>
#include <QBChannelScanningConfUserParams.h>
#include <QBStandardRastersConfig.h>

/**
 * Defines the channel scanning configuration parameters.
 *
 * The 'force' attribute is used to express tri-state values:
 * - set to true
 * - set to false
 * - not set
 *
 * The parameter selection algorithms use all three values.
 **/
struct QBChannelScanningConf_t {
    struct SvObject_ super_;

    SvString id;    ///< id of the configuration, unique and not to be displayed to the user
    SvString name;  ///< user displayable name of configration

    SvString satelliteID;   ///< Starts the scanning from this satellite
    bool forceSatelliteID;

    int * tvTypes;          ///< DVB/MPEG define certain types in service list descriptor as TV channels.
                            ///< Some operators use also additional nonstandard types, which can be provided here
    int tvTypesCnt;

    int * radioTypes;       ///< DVB/MPEG define certain types in service list descriptor as Radio channels.
                            ///< Some operators use also additional nonstandard types, which can be provided here

    int radioTypesCnt;

    QBTunerModulation * ignoredModulation;  ///< Used to forcefully disable selected modulations
    int ignoredModulationCnt;

    bool forceOnlyActual;
    bool onlyActual;                        ///< If true all 'Other' (SDT/NIT) tables are ignored

    bool forceDropNotFoundInNIT;
    bool dropNotFoundInNIT;

    bool forceTuner;
    QBTunerType tunerType;                  ///< Defines on what tuner type the configuration is valid

    bool forceChaseChannels;
    QBDVBChaseChannelsMethod chaseChannels; ///< If not disabled, it turns on the channel chasing mode which
                                            ///< upon finding first NIT (nonRecursive) or any NIT (recursive)
                                            ///< no longer goes through all muxes/transponders
                                            ///< but only enters all the muxes specified in NIT

    bool forceChaseMissingChannels;
    bool chaseMissingChannels;              ///< If not disabled, it turns on the channel chasing mode which
                                            ///< upon finding first NIT (nonRecursive) or any NIT (recursive)
                                            ///< no longer goes through all muxes/transponders
                                            ///< but only enters all the muxes, which have channels for which
                                            ///< we do not have complete information (like name from SDT), specified in NIT


    bool forceScanOtherSatellites;
    bool scanOtherSatellites;               ///< If true it allows the scanner to search for channels on
                                            ///< different satellites than only the one provided by satelliteID

    bool forceReadBAT;
    QBDVBChaseChannelsMethod readBAT;       ///< Specifies how BAT should be read: not at all, only before chasing (missing) channels, all occurences
    bool forceReadSDTOther;
    QBDVBChaseChannelsMethod readSDTOther;  ///< Specifies how SDT other should be read: not at all, only before chasing (missing) channels, all occurences

    bool forceReadNITOther;
    QBDVBChaseChannelsMethod readNITOther;  ///< Specifies how NIT other should be read: not at all, only before chasing (missing) channels, all occurences

    bool forceRecordMuxesFromNIT;
    bool recordMuxesFromNIT;                ///< If set to true it requests scanner to provide a list of all muxes that were listed in NIT

    int *symbolRate;                        ///< Provides the list of symbol rates that the scanner should try to use
    int symbolRateCnt;

    struct QBChannelScanningConfRange_t* range; ///< Provides the list of muxes that the scanner should use
    int rangeCnt;

    QBTunerModulation* modulation;          ///< Provides the list of modulations that the scanner should try to use
    int modulationCnt;

    QBTunerPolarization* polarization;      ///< Provides the list of polarization that the scanner should try to use
    int polarizationCnt;

    QBTunerStandard standard;               ///< Provides the tuner standard to use
    bool forceStandard;

    bool dropAll;                           ///< True iff other channels should be dropped after channels from this scan are saved

    bool forceLcnOverwrite;                 ///< If true then if new scan was unable to find channel LCN then allow to set it to -1 (instead of keeping old value)
    bool lcnOverwrite;

    bool forceNameOverwrite;
    bool nameOverwrite;                     ///< If true then if new scan was unable to find channel name then allow to set it to "" (instead of keeping old value)


    bool forceNeedLcn;
    bool needLcn;                           ///< If true all channels without LCN will be ignored
    SvArray lcnSource;                      ///< Tells which tables may be used as LCN source (NIT/BAT)

    bool forceNeedName;
    bool needName;                          ///< If true channels without name will be ignored

    bool forceOnlyPAT;
    bool onlyPAT;                           ///< If true the scanner will only scan basing on PAT (it will ignore SDT/NIT/BAT)

    struct {
        QBTunerModulation* modulation;
        int modulationCnt;
        int *symbolRate;
        int symbolRateCnt;
    } chasing;

    struct {
        bool forceIgnoreTsid;
        bool ignoreTsid;                    ///< Sometimes the tsid value in tables is broken. If set to true it is ignored and channel id does not take it into account.
        bool forceIgnoreOnid;
        bool ignoreOnid;                    ///< Sometimes the onid value in tables is broken. If set to true it is ignored and channel id does not take it into account.
    } key;

    SvArray DVBScannerPlugins;              ///< Provides the list of QBDVBScannerPlugin that will be used

    SvHashTable automaticScanParams;        ///< If we do not want the user to provide his selection of a parameter we
                                            ///< can provide its name here. Instead of selection we will use
                                            ///< either all allowed values (if parameter is not provided by configuration)
                                            ///< or all values provided in configuration.
    bool forceUsePredefinedTransponders;
    bool usePredefinedTransponders;         ///< If true only transponders of a satellite will be scanned
    bool forceUseStandardRasters;
    bool useStandardRasters;                ///< If set to true it will limit all parameters by 'standard rasters'. For example it is used to disallow certain modulations in DVB-C QAMB.
    bool seekUntilFirstNITFound;            ///< If true upon finding first NIT stop the channel scanning process
    bool forceSeekUntilFirstNITFound;

    bool doNotDropPrivateIds;               ///< If true all channels with private nid or onid will be saved

    bool forceIgnoreCountryAvailability;    ///< If true, country availability setting will be forced during scan
    bool ignoreCountryAvailability;         ///< If true requirements from country availability descriptor will be ignored

    bool forceKeepTuned;
    bool keepTuned;
};
typedef struct QBChannelScanningConf_t* QBChannelScanningConf;

SvType QBChannelScanningConf_getType(void);

void QBChannelScanningConfFillParams(QBChannelScanningConf conf, QBDVBScannerParams* params, QBStandardRastersConfig rastersConfig, QBTunerReserver *tunerReserver, int NID, int ONID, int BOUQUETID, const char* scanStandard, QBDVBSatellitesDB satellitesDB, const char* defaultSatelliteID);

/**
 * @}
 **/

#endif // QBCHANNELSCANNINGCONF_H_
