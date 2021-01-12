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

#ifndef QBSTANDARDRASTERSCONFIG_H_
#define QBSTANDARDRASTERSCONFIG_H_

/**
 * @file QBStandardRastersConfig.h Standard rasters configuration API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvArray.h>
#include <QBTunerTypes.h>

typedef SvArray QBStandardRastersConfig;

/**
 * Create empty standard rasters configuration object.
 * @qb_allocator
 *
 * @return created object or @c NULL in case of error
 **/
QBStandardRastersConfig QBStandardRastersConfigCreate(void);

/**
 * Load standard rasters configuration from file and fill configuration object. If you call more then once this function, the previous loaded configuration will be removed.
 *
 * @note The @a configFile has configuration written in JSON. Sample configuration file looks this:
 * @verbatim
{
"comment": [
    "standardRasters file.",
    "'Elements' array contains an array of standard rasters. Fields of each",
    "element are described below:",
    "[REQUIRED] id - string",
    "[REQUIRED] type - string - tuner type 'cab', 'sat', 'ter'",
    "[REQUIRED] band - integer - frequency jump of raster",
    "[REQUIRED] modulation - array of modulation strings",
    "[REQUIRED] freq_min - integer - frequency minimum of raster",
    "[REQUIRED] freq_max - integer - frequency maximum of raster",
    "[OPTIONAL] useHalfFrequencies - boolean - do we use half frequencies"
],
"elements" : [
    {
        "id": "DVBT_7",
        "type": "ter",
        "band": 7,
        "modulation": ["QAM16", "QAM64", "QAM256", "QPSK"],
        "freq_min": 107500,
        "freq_max": 296500,
        "useHalfFrequencies" : true
    },
    {
        "id": "DVBC_8",
        "type": "cab",
        "standard": "dvbc",
        "band": 8,
        "modulation": ["QAM16", "QAM64", "QAM128", "QAM256"],
        "freq_min": 114000,
        "freq_max": 858000
    }
]
}
   @endverbatim
 *
 * @param self [in]          handle to configuration object
 * @param configFile [in]    full path to file which store rasters configuration
 **/
void QBStandardRastersConfigLoad(QBStandardRastersConfig self, const char* configFile);

/**
 * Create array of QBStandardRastersParams objects which has parameters equal to specified @a tunerType, @a standard and @a band.
 * @qb_allocator
 *
 * @param [in] self         handle to configuration file
 * @param [in] tunerType    tuner type which we are interested
 * @param [in] standard     tuner standard which we are interested
 * @param [in] band         bandwidth which we are interested. If bandwidth is equal @c 0 it means that we are interested any bandwidth.
 *
 * @return Array of QBStandardRastersParams. Order of elements in array is not specified. If we don't find any interested parameters we receive empty array.
 **/
SvArray QBStandardRastersConfigCreateParams(QBStandardRastersConfig self, QBTunerType tunerType, QBTunerStandard standard, int band);

#endif // QBSTANDARDRASTERSCONFIG_H_
