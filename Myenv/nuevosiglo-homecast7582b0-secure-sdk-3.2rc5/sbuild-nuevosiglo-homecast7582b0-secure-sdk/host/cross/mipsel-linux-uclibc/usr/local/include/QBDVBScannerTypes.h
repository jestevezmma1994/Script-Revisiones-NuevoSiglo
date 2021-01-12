/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QBDVBSCANNERTYPES_H_
#define QBDVBSCANNERTYPES_H_

#include <QBTunerTypes.h>
#include <QBDVBSatellitesDB.h>
#include <SvFoundation/SvHashTable.h>
#include <SvFoundation/SvArray.h>
#include <QBDVBSatellitesDB.h>
#include <tunerReserver.h>
#include <QBDVBScannerTunerStats.h>

struct QBDVBScanner_s;
typedef struct QBDVBScanner_s  QBDVBScanner;
typedef struct QBDVBChannelVariant_s QBDVBChannelVariant;
typedef struct QBDVBMuxIdDesc_s QBDVBMuxIdDesc;
typedef enum QBDVBChaseChannelsMethod_e QBDVBChaseChannelsMethod;
typedef enum QBDVBChannelType_e  QBDVBChannelType;
typedef void (*QBDVBScannerOnFinish)(void *target, QBDVBScanner* scanner);
typedef void (*QBDVBScannerOnStep)(void *target, QBDVBScanner* scanner);
typedef void (*QBDVBScannerOnRevoke)(void *target, QBDVBScanner* scanner);
typedef struct QBDVBChannelKey_s  QBDVBChannelKey;
typedef enum QBDVBChannelVariantParamsSource_e QBDVBChannelVariantParamsSource;

enum QBDVBChannelVariantParamsSource_e {
      QBDVBChannelVariantParamsSource_SDTActual,
      QBDVBChannelVariantParamsSource_NITActual,
};

struct base_results_s {
  int sid;
  int tsid;
  int onid;

  QBDVBChannelVariantParamsSource paramsSource;
  struct QBTunerParams params;

  int type;
};

/**
 * Channel chase methods
 **/
enum QBDVBChaseChannelsMethod_e {
    QBDVBChaseChannelsType_disabled = 0,
    QBDVBChaseChannelsType_nonRecursiveNIT, // Only one NIT is required
    QBDVBChaseChannelsType_recursiveNIT,    // Always scan for NIT
    QBDVBChaseChannelsType_nonRecursive,    // Only one table instance is required
    QBDVBChaseChannelsType_recursive,       // Look for table on all frequencies
    QBDVBChaseChannelsType_cnt
};

/// As found in NIT/SDT
enum QBDVBChannelType_e {
  QBDVBChannelType_unknown    =   -1,
  QBDVBChannelType_TV         = 0x01,
  QBDVBChannelType_Radio      = 0x02,
  QBDVBChannelType_Radio_AVC  = 0x0A,
  QBDVBChannelType_Data       = 0x0C,
  QBDVBChannelType_HDTV_MPEG2 = 0x11,
  QBDVBChannelType_SDTV_AVC   = 0x16,
  QBDVBChannelType_HDTV_AVC   = 0x19,
};

struct QBDVBMuxIdDesc_s {
  struct SvObject_ super_;
  SvString network_name;
  int nid;
  bool hadLock;
};


struct QBDVBChannelVariant_s {
  struct SvObject_ super_;

  QBDVBChannelVariantParamsSource paramsSource;
  struct QBTunerParams params;

  QBDVBScannerTunerStats tunerStats;
  QBDVBMuxIdDesc* freq_desc;

  SvHashTable metadata;
};

struct QBDVBChannelKey_s {
  struct SvObject_ super_;

  int sid;
  int tsid;
  int onid; /// only used for non-PAT channels (else -1)
};


struct QBDVBScannerParams_s {
  int tunerNum;
  struct {
    /// If not NULL, has the same size as freq
    /// if band[i] != -1 then it means that
    /// band[i] should be used as parameter for tuning, else
    /// self->band
    int *band;
  } forced;

  int* freq;
  int freqCnt;

  QBTunerModulation* modulations;
  int modulationCnt;

  QBTunerPolarization* polarizations;
  int polarizationCnt;

  QBTunerStandard standard;

  int* symbolRates;
  int symbolRateCnt;

  int band;

  //Extra value for tuner tuning
  int* extraVal;
  int extraValCnt;


  void *target;
  QBDVBScannerOnFinish onFinish;
  QBDVBScannerOnStep onStep;
  QBDVBScannerOnRevoke onRevoke;

  struct {
      QBDVBChaseChannelsMethod chaseChannels;  //Means use NIT as a source of frequencies to scan
      bool chaseMissingChannels;               //Means use NIT/SDT as a source of channels, use NIT as a source of frequencies to scan when we have no SDT
      QBTunerModulation *modulations;
      int modulationCnt;

      int* symbolRates;
      int symbolRateCnt;

      QBDVBChaseChannelsMethod readBATMode;             // Decides if BAT is to be read and how (recursively or not)
      QBDVBChaseChannelsMethod readSDTOtherMode;        // Decides if SDT other is to be read and how (recursively or not)
      QBDVBChaseChannelsMethod readNITOtherMode;        // Decides if NIT other is to be read and how (recursively or not)
  } chasing;

  /// Detect channels existence based only on SDT-actual, do not
  /// trust NIT as channel source (although still use it as
  /// source of channel metadata like LCN and network name)
  bool onlyActual;

  bool scanOtherSatellites;

  bool recordMuxesFromNIT;

  /// Ignore SDT/NIT, just get channels from PAT.
  /// Warning: this is only for purposes of handling weird networks in which
  /// we are unable to scan based on NIT/SDT. It shouldn't be used in any production
  bool onlyPAT;

  bool dropNotFoundInNIT;
  bool seekUntilFirstNITFound;

  /// -1 - not set
  int chosenNID;
  int chosenONID;
  int bouquetID;

  QBTunerType tunerType;

  struct {
    bool ignoreTsid;
    bool ignoreOnid;
  } key;

  int *additionalTVTypes;
  int additionalTVTypeCnt;

  int *additionalRadioTypes;
  int additionalRadioTypeCnt;

  SvString satelliteID;

  QBDVBSatellitesDB satellitesDB;

  SvArray additionalMuxes;

  QBTunerReserver *reserver;
  int priority;
  bool needName;
  bool needLcn;
  SvArray lcnSource;

  //For plugins
  SvHashTable metadata;
  /// do not untune the tuner if set to true
  bool keepTuned;

  /// Id true all channels with private nid or onid will be saved
  bool doNotDropPrivateIds;

  /// If true requirements from country availability descriptor will be ignored
  bool ignoreCountryAvailability;
};
typedef struct QBDVBScannerParams_s  QBDVBScannerParams;

SvType
QBDVBChannelKey_getType(void);

#endif
