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

#ifndef QB_DVB_SCANNER_H
#define QB_DVB_SCANNER_H

/**
 * @file QBDVBScanner.h QBDVBScanner API
 */

/**
 * @defgroup QBDVBScanner QBDVBScanner classes
 * @ingroup QBDVB
 * @{
 *
 */

#include <QBDVBScannerTypes.h>
#include <QBDVBSatellitesDB.h>

#include <SvFoundation/SvType.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvHashTable.h>

#include <fibers/c/fibers.h>

#include <QBTunerTypes.h>
#include <QBTunerLNBConfig.h>
#include <tunerReserver.h>

#include <stdbool.h>


///----------------------------------
/// QBDVBScanFilterSettings
///----------------------------------

struct QBDVBScanFilterSettings_s
{
    bool ignoreNIT;
    bool ignoreSDT;
    bool ignorePAT;
};
typedef struct QBDVBScanFilterSettings_s QBDVBScanFilterSettings;


/// SD or HD
bool QBDVBChannelTypeIsTV(QBDVBChannelType type);
/// HD only
bool QBDVBChannelTypeIsHDTV(QBDVBChannelType type);
bool QBDVBChannelTypeIsRadio(QBDVBChannelType type);

extern QBDVBScanner*
QBDVBScannerCreate(SvScheduler scheduler, SvArray plugins);

extern void
QBDVBScannerDestroy(QBDVBScanner* scanner);

void
QBDVBScannerAddPlugin(QBDVBScanner* scanner, SvObject plugin);

extern void
QBDVBScannerSetPassiveMode(QBDVBScanner* scanner, bool enable, struct QBTunerMuxId mux);

extern void
QBDVBScannerStart(QBDVBScanner* scanner, const QBDVBScannerParams* params);

/**
 * QBDVBScannerStartWithTunerResvParam starting scanning with scanner and tuner reserver params.
 *
 * @param[in] scanner instance of dvb scanner
 * @param[in] params scanning of params
 * @param[in] resvParams params of tuner reservation
 **/
extern void
QBDVBScannerStartWithTunerResvParam(QBDVBScanner * scanner, const QBDVBScannerParams *params, const QBTunerResvParams *resvParams);

extern void
QBDVBScannerStop(QBDVBScanner* scanner);

/**
 * Safe stop scanner.
 * This method wait for a NIT package and after that stop scanning.
 * @param[in] scanner handler to scanner
 */
extern void
QBDVBScannerSafeStop(QBDVBScanner* scanner);

extern void
QBDVBScannerAddAdditionalMuxes(QBDVBScanner* scanner, SvArray params);

extern void
QBDVBScannerAddMux(QBDVBScanner *scanner, struct QBTunerParams *freq, const QBDVBScanFilterSettings *filterSettings);

extern const QBDVBScannerParams*
QBDVBScannerGetParams(const QBDVBScanner* scanner);

extern SvArray
QBDVBScannerGetChannels(const QBDVBScanner* scanner, bool fullOnly);

extern SvHashTable
QBDVBScannerGetNetworks(const QBDVBScanner* scanner);

/**
 * Return visited muxes hash table.
 *
 * @param[in] scanner   handler to scanner
 * @return hash table with visited muxes
 **/
extern SvHashTable
QBDVBScannerGetVisitedMuxId(const QBDVBScanner *scanner);

extern SvHashTable
QBDVBScannerGetMuxes(const QBDVBScanner* scanner);

extern SvArray
QBDVBScannerListMuxes(const QBDVBScanner* scanner);

extern int
QBDVBScannerGetSymbolRate(const QBDVBScanner* scanner);

extern int
QBDVBScannerGetFreq(const QBDVBScanner* scanner);

extern QBTunerModulation
QBDVBScannerGetModulation(const QBDVBScanner* scanner);

extern QBTunerPolarization
QBDVBScannerGetPolarization(const QBDVBScanner* scanner);

extern int
QBDVBScannerGetTunerNum(const QBDVBScanner* scanner);

/**
 * Creates string representation of given QBDVBChaseChannelsMethod.
 *
 * @param[in] type  chase channels type
 * @return          string representation of QBDVBChaseChannelsMethod
 **/
extern SvString
QBDVBScannerChaseMethodsToString(QBDVBChaseChannelsMethod type);

extern QBDVBChaseChannelsMethod
QBDVBScannerChaseMethodsFromString(const SvString chaseChannelsStr);

extern QBDVBScannerParams
QBDVBScannerParamsGetEmpty(void);

/**
 * Return number of channels. Number of all types of channels - TV channels (HD, SD) and radio channels.
 *
 * @param[in] scanner    handler to scanner
 *
 * @return   number of channels
 **/
extern int
QBDVBScannerGetNumberOfChannels(const QBDVBScanner *scanner);

/**
 * Return number of radio channels.
 *
 * @param[in] scanner    handler to scanner
 *
 * @return  number of radio channels
 **/
extern int
QBDVBScannerGetNumberOfRadioChannels(const QBDVBScanner *scanner);

/**
 * Return number of TV channels (SD + HD).
 *
 * @param [in] scanner   handler to scanner
 *
 * @return   number of TV channels
 **/
extern int
QBDVBScannerGetNumberOfTVChannels(const QBDVBScanner *scanner);

/**
 * @}
 **/

#endif // QB_DVB_SCANNER_H
