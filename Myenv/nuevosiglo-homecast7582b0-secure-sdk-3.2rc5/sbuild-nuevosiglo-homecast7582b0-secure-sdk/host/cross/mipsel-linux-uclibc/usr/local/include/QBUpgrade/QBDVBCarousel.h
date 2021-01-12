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

#ifndef QB_DVB_CAROUSEL_H_
#define QB_DVB_CAROUSEL_H_

/**
 * @file QBDVBCarousel.h
 * @brief DVB data carousel
 * @private
 **/

/**
 * @defgroup QBDVBCarousel DVB data carousel
 * @ingroup QBUpgrade
 * @{
 **/

#include <stdint.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvURI.h>
#include <QBTunerTypes.h>
#include <SvFoundation/SvString.h>

/**
 * Cubiware DVB data carousel packet (MPEG-TS private section) header.
 **/
struct QBDVBCarouselHeader_ {
    /// FIXME
    uint8_t type;
    /// FIXME
    uint8_t version;
    /// FIXME
    uint16_t SID;
    /// FIXME
    uint32_t offset;
    /// FIXME
    uint32_t totalSize;
};


/**
 * Parse DVB carousel URI.
 *
 * @param[in] uri       DVB carousel URI
 * @param[out] type     tuner type
 * @param[out] params   tuning parameters
 * @param[out] SID      DVB carousel SID
 * @param[out] allowAnyFirmwareVersion   if it is forced update - regardless of the new firmware version
 * @param[out] errorOut error info
 **/
extern void
QBDVBCarouselParseURI(SvURI uri,
                      QBTunerType *type,
                      struct QBTunerParams *params,
                      unsigned int *SID,
                      bool *allowAnyFirmwareVersion,
                      SvErrorInfo *errorOut);

/**
 * Creates DVB cable carousel URI which says on what mux upgrade is available.
 * @param[in] tunerStandard tuner DVB-C standard
 * @param[in] freq          mux frequency
 * @param[in] modulation    mux modulation
 * @param[in] symbol_rate   mux symbol rate
 * @param[in] SID           upgrade file SID
 * @param[in] selector      upgrade selector
 * @param[in] allowAnyFirmwareVersion   true iff downgrades or same version upgrades are allowed.
 * @param[in] band          bandwidth
 * @return DVB cable upgrade URI
 */
SvString QBDVBCarouselCreateCableURI(QBTunerStandard tunerStandard, int32_t freq, QBTunerModulation modulation, int symbol_rate, unsigned int SID,
        SvString selector, bool allowAnyFirmwareVersion, int band);

/**
 * @}
 **/


#endif
