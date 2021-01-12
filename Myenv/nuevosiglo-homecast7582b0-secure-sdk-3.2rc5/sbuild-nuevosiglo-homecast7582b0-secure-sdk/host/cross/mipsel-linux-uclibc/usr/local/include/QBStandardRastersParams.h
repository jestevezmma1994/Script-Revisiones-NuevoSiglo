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

#ifndef QBSTANDARDRASTERSPARAMS_H_
#define QBSTANDARDRASTERSPARAMS_H_

/**
 * @file QBStandardRastersParams.h Standard Rasters Parameters API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvArray.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBPlatformHAL/QBPlatformTypes.h>
#include <QBTunerTypes.h>

struct QBStandardRastersParams_t {
    struct SvObject_ super_; ///< super class
    SvString id;                     ///< ID name which represents parameters, for example: 'DVBC_8', 'ANNEXB_6'.
                                     ///< Each raster parameter has to have unique ID
    QBTunerType type;                ///< type of tuner
    QBTunerStandard standard;        ///< tuner standard
    int band;                        ///< bandwidth - frequency jump of raster
    SvArray modulations;             ///< array of SvValues (string) allowed modulations.
                                     ///< Example values: 'QAM16', 'QAM256'
    int freq_min;                    ///< frequency minimum of raster
    int freq_max;                    ///< frequency maximum of raster
    bool useHalfFrequencies;         ///< do we use half frequencies.
};

typedef struct QBStandardRastersParams_t* QBStandardRastersParams;

/**
* Create new empty QBStandardRastersParams object.
* @note If you are interested how to get filled parameters you should see class QBStandardRastersConfig.
*
* @return   created QBStandardRastersParams object
*/
QBStandardRastersParams QBStandardRastersParamsCreate(void);

/**
 * Get runtime type identification object representing QBStandardRastersParams class.
 *
 * @return rasters class
 **/
extern SvType
QBStandardRastersParams_getType(void);

#endif // QBSTANDARDRASTERSPARAMS_H_
