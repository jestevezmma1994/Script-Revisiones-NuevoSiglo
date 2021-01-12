/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef CUIT_TYPES_H_
#define CUIT_TYPES_H_

/**
 * @file SMP/CUIT/types.h CUIT types.
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Application handle.
 * @ingroup SvApplication
 **/
typedef struct SvApplication_t *SvApplication;


/**
 * @addtogroup SvWidget
 * @{
 **/

/** Widget identifier. */
typedef unsigned int SvWidgetId;

/** Widget handle. */
typedef struct SvWidget_t *SvWidget;

/** Window handle. */
typedef SvWidget SvWindow;

/**
 * @}
 **/

/**
 * An abstract base class for windows implementation.
 *
 * @ingroup QBWindow
 * @class QBWindow QBWindow.h <CUIT/Core/QBWindow.h>
 * @extends SvObject
 *
 * @note If only class declaration is needed, include <CUIT/Core/types.h>
 **/
typedef struct QBWindow_ *QBWindow;

/**
 * QBWidget.
 * @ingroup QBWidget
 * @class QBWidget
 * @extends SvObject
 **/
typedef struct QBWidget_ *QBWidget;

/**
 * QBCUITWidget.
 * @ingroup QBCUITWidget
 * @class QBCUITWidget
 * @extends QBWidget
 **/
typedef struct QBCUITWidget_ *QBCUITWidget;


/** Event handle. */
typedef struct SvEvent_t *SvEvent;

/** Input event handle. */
typedef struct SvInputEvent_t *SvInputEvent;

/** Focus event handle. */
typedef struct SvFocusEvent_t *SvFocusEvent;

/** Effect event handle. */
typedef struct SvEffectEvent_t *SvEffectEvent;

/** User event handle. */
typedef struct SvUserEvent_t *SvUserEvent;

/** Timer event handle. */
typedef struct SvTimerEvent_t *SvTimerEvent;


/**
 * Timer identifier.
 * @ingroup SvApplicationTimers
 **/
typedef unsigned int SvTimerId;


/**
 * Effect identifier.
 * @ingroup SvEffect
 **/
typedef unsigned int SvEffectId;

/**
 * Effect handle.
 * @ingroup SvEffect
 **/
typedef struct SvEffect_t *SvEffect;


#ifdef __cplusplus
}
#endif

#endif
