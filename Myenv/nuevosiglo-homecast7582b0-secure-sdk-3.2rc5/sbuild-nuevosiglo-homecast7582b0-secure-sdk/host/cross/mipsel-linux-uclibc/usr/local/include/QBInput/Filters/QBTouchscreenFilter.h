/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_TOUCHSCREEN_FILTER_H_
#define QB_TOUCHSCREEN_FILTER_H_

/**
 * @file QBTouchscreenFilter.h
 * @brief Touchscreen input filter class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBTouchscreenFilter Touchscreen input filter class
 * @ingroup QBInputEventFilters
 * @{
 **/

/**
 * Touchscreen input filter class.
 * @class QBTouchscreenFilter
 * @extends SvObject
 **/
typedef struct QBTouchscreenFilter_ *QBTouchscreenFilter;

/**
 * Touchscreen calibration data.
 *
 * This structure describes mapping between touchscreen coordinates
 * and screen coordinates.
 **/
typedef struct {
   int offsetX;   ///< correction in X direction (applied before scale)
   float scaleX;  ///< scale factor in X direction
   int offsetY;   ///< correction in Y direction (applied before scale)
   float scaleY;  ///< scale factor in Y direction
} QBTouchscreenCalibration;


/**
 * Get runtime type identification object representing
 * touchscreen input filter class.
 *
 * @return touchscreen input filter class
 **/
extern SvType QBTouchscreenFilter_getType(void);

/**
 * Initialize touchscreen input filter object.
 *
 * @memberof QBTouchscreenFilter
 *
 * @param[in] self         touchscreen input filter handle
 * @param[in] calibration  touchscreen calibration data
 * @param[out] errorOut    error info
 * @return                 self, NULL in case of error
 **/
extern QBTouchscreenFilter QBTouchscreenFilterInit(QBTouchscreenFilter self,
                                                   const QBTouchscreenCalibration *calibration,
                                                   SvErrorInfo *errorOut);


/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
