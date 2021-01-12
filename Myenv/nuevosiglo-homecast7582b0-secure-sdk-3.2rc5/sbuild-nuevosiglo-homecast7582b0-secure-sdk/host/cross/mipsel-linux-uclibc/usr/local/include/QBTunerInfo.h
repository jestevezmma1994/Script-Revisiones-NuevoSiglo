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

#ifndef QB_TUNER_INFO_H
#define QB_TUNER_INFO_H

#include "QBTunerTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void QBTunerPropertiesGenerate(QBTunerType type, struct QBTunerProperties* prop);

void QBTunerPropertiesGenerateCable(      struct QBTunerProperties* prop);
void QBTunerPropertiesGenerateSatellite(  struct QBTunerProperties* prop);
void QBTunerPropertiesGenerateTerrestrial(struct QBTunerProperties* prop);
/**
 * Returns a text string with tuner parameters in human-readable form.
 *
 * Returned text string is composed of name=value pairs separated by colons.
 * No other fields than present in the following example can be returned.
 * Resulting string can be parsed back by QBTunerParamsFromString().
 * Example:
 *
 * `standard=dvbc:freq=663000:mod=QAM256:symbol=6875:lnbCfg=eutelsat_3a:symbol=27500:pol=v:band=1:custom0=1:plp=1:satMode=dvbs:tunExtra=1:specInv=1:freqOff=1`
 *
 * @param[in] params       handle of @ref QBTunerParams to be printed to string
 * @return                  a pointer to a string with correct URI
 * @see QBTunerParams
 **/
char *QBTunerParamsToString(const struct QBTunerParams *params);
/**
 * Reads tuner parameters from a human-readable string.
 *
 * A mirror function to QBTunerParamsToString(), where the correct text format is specified.
 *
 * @param[in] p             source string that tuner parameters have to be read from
 * @param[out] params       target object for read tuner parameters
 * @return                  0 on success, otherwise -1 (wrong/unreadable string)
 **/
int QBTunerParamsFromString(const char *p, struct QBTunerParams *params);
/**
 * Finds standard length of a parameter in string containing tuner parameters.
 *
 * @param[in] p             pointer to a place in string where parameter to be checked starts
 * @return                  length of that parameter
 **/
unsigned int QBTunerParamsGetParamLength(const char* p);

#ifdef __cplusplus
}
#endif

#endif // #ifndef QB_TUNER_INFO_H
