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

#ifndef QB_LEDBlink_FILTER_H_
#define QB_LEDBlink_FILTER_H_

/**
 * @file QBLEDBlinkFilter.h
 * @brief LED Blink Input Filter Class API.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBLEDBlinkFilter LED Blink input filter device class
 * @ingroup QBInput
 **/

#include <SvFoundation/SvCoreTypes.h>

/**
 * LED Blink input filter class.
 * @class QBLEDBlinkFilter
 * @extends SvObject
 **/
typedef struct QBLEDBlinkFilter_ *QBLEDBlinkFilter;

/**
 * Get runtime type identification object representing
 * LED blink input filter class.
 * @return LED blink input filter class
 **/
extern SvType QBLEDBlinkFilter_getType(void);

/**
 * Initialize LED Blink input filter object.
 *
 * This method initializes LED Blink filter. The filter will not
 * work until configured using QBLEDBlinkFilterSetup() method
 * and enabled by QBLEDBlinkFilterSetEnabled().
 *
 * @memberof QBLEDBlinkFilter
 *
 * @param[in] self      LED Blink input filter handle
 * @param[out] errorOut error info
 * @return              @a self, @c NULL in case of error
 **/
extern QBLEDBlinkFilter QBLEDBlinkFilterInit(QBLEDBlinkFilter self, SvErrorInfo *errorOut);

/**
 * Set LED Blink filter parameters.
 *
 * @memberof QBLEDBlinkFilter
 *
 * @param[in] self              LED Blink input filter handle
 * @param[in] ledID             ID of led to be managed
 * @param[in] idleBrightness    idle brightness of LED
 * @param[in] activeBrightness  active brightness of LED
 * @param[in] blinkTime         time of blink in ms
 * @param[out] errorOut         error info
 **/
extern void QBLEDBlinkFilterSetup(QBLEDBlinkFilter self,
                                  int ledID, int idleBrightness,
                                  int activeBrightness, int blinkTime,
                                  SvErrorInfo *errorOut);

/**
 * Get enabled state of LED Blink filter.
 *
 * @memberof QBLEDBlinkFilter
 *
 * @param[in] self      LED Blink input filter handle
 * @param[out] errorOut error info
 * @return              state
 **/
extern bool QBLEDBlinkFilterGetEnabled(QBLEDBlinkFilter self, SvErrorInfo *errorOut);

/**
 * Set enabled state of LED Blink filter.
 *
 * @memberof QBLEDBlinkFilter
 *
 * @param[in] self      LED Blink input filter handle
 * @param[in] enabled   desired state
 * @param[out] errorOut error info
 **/
extern void QBLEDBlinkFilterSetEnabled(QBLEDBlinkFilter self, bool enabled, SvErrorInfo *errorOut);

/**
 * @}
 **/

#endif
