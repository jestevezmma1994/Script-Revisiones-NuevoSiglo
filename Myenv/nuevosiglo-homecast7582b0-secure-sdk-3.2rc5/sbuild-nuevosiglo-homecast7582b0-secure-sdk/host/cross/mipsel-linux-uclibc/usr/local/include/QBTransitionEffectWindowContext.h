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

#ifndef QB_TRANSITION_EFFECT_WINDOW_CONTEXT_H_
#define QB_TRANSITION_EFFECT_WINDOW_CONTEXT_H_

/**
 * @file QBTransitionEffectWindowContext.h QBTransitionEffectWindowContext class API
 **/

#include "QBWindowContext.h"

#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBTransitionEffectWindowContext Window context with transition effects class
 * @ingroup QBApplicationControllerLibrary
 * @{
 *
 * An abstract base class for top-level window with transition effects context implementations.
 **/

/**
 * Abstract window context class.
 * @class QBTransitionEffectWindowContext
 * @extends QBWindowContext
 **/
typedef struct QBTransitionEffectWindowContext_ {
    struct QBWindowContext_t super_;
} *QBTransitionEffectWindowContext;

/**
 * Virtual methods of the window context with transition effects class.
 **/
typedef const struct QBTransitionEffectWindowContextVTable_ {
    /// virtual methods of the base class
    struct QBWindowContextVTable_ super_;

    /**
     * Create show effect for window context.
     *
     * @param[in] self                  transition effect window context handle
     * @param[in] defaultDuration       default switch duration
     * @return                          created effect, may be @c NULL
     **/
    SvEffect (*createShowEffect)(QBTransitionEffectWindowContext self,
                                 double defaultDuration);

    /**
     * Create hide effect for window context.
     *
     * @param[in] self                  transition effect window context handle
     * @param[in] defaultDuration       default switch duration
     * @return                          created effect, may be @c NULL
     **/
    SvEffect (*createHideEffect)(QBTransitionEffectWindowContext self,
                                 double defaultDuration);
} *QBTransitionEffectWindowContextVTable;

/**
 * Get runtime type identification object representing window context class.
 * @return Window context with transition effects type identification object
 **/
SvType
QBTransitionEffectWindowContext_getType(void);

/**
 * Create show effect for window context.
 *
 * @memberof QBTransitionEffectWindowContext
 *
 * @param[in] self                  transition effect window context handle
 * @param[in] defaultDuration       default switch duration
 * @return                          created effect, may be @c NULL
 **/
extern SvEffect
QBTransitionEffectWindowContextCreateShowEffect(QBTransitionEffectWindowContext self,
                                                double defaultDuration);

/**
 * Create hide effect for window context.
 *
 * @memberof QBTransitionEffectWindowContext
 *
 * @param[in] self                  transition effect window context handle
 * @param[in] defaultDuration       default switch duration
 * @return                          created effect, may be @c NULL
 **/
extern SvEffect
QBTransitionEffectWindowContextCreateHideEffect(QBTransitionEffectWindowContext self,
                                                double defaultDuration);

/**
 * @}
 **/

#endif
