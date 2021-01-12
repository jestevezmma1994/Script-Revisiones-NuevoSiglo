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

#ifndef QBCHANNELSCANNINGCONFUSERPARAMS_H_
#define QBCHANNELSCANNINGCONFUSERPARAMS_H_

/**
 * @file QBChannelScanningConfUserParams.h
 * @brief User channel scanning configuration
 **/

#include <SvFoundation/SvCoreTypes.h> // SvObject
#include <QBTunerTypes.h>             // QBTunerModulation

/**
 * Describes a range of frequencies expressed by formulas:
 * x = freqMin + freqJump * i && x < freqMax
 **/
struct QBChannelScanningConfRange_t {
    int freqMin;    ///< Start of the range
    int freqMax;    ///< End of the range
    int freqJump;   ///< Selector of the range
    int band;       ///< Bandwidth size
};

typedef struct QBChannelScanningConfRange_t* QBChannelScanningConfRange;


/**
 * Describes last user preferences.
 **/
struct QBChannelScanningConfUserParams_t {
    struct SvObject_ super_;
    struct QBChannelScanningConfRange_t range;
    int symbolRate;
    QBTunerModulation modulation;
};

typedef struct QBChannelScanningConfUserParams_t* QBChannelScanningConfUserParams;

QBChannelScanningConfUserParams QBChannelScanningConfUserParamsCreate(int freqMin, int freqMax, int freqJump, int symbolRate, QBTunerModulation modulation);

/**
 * @}
 **/

#endif // QBCHANNELSCANNINGCONFUSERPARAMS_H_
