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

#ifndef QB_GLOBAL_INPUT_LOGIC_H
#define QB_GLOBAL_INPUT_LOGIC_H

/**
 * @file GlobalInputLogic.h Global input logic class API
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvObject.h>
#include <QBInput/QBInputEvent.h>
#include <main_decl.h>

/**
 * @file GlobalInputLogic.h
 * @brief GlobalInput logic API
 */

/**
 * @defgroup GlobalInputLogic Global input logic class
 * @ingroup CubiTV_logic
 * @{
 **/

/**
 * Global input logic class.
 *
 * @class QBGlobalInputLogic GlobalInputLogic.h <Logic/GlobalInputLogic.h>
 * @extends SvObject
 *
 * This is an application logic class implementing functionality related to
 * global input handling. An instance of this class can be created by using
 * @ref QBLogicFactory.
 **/
typedef struct QBGlobalInputLogic_ *QBGlobalInputLogic;

/**
 * Global input logic virtual methods table.
 **/
typedef const struct QBGlobalInputLogicVTable_ {
    /**
     * virtual methods table of the base class
     **/
    struct SvObjectVTable_ super_;

    /**
     * Initialize standby logic instance.
     *
     * @param[in]  self       global input logic handle
     * @param[in]  appGlobals CubiTV application state
     * @param[out] errorOut   error info
     * @return                @a self or @c NULL in case of error
     **/
    QBGlobalInputLogic (*init)(QBGlobalInputLogic self,
                               AppGlobals appGlobals,
                               SvErrorInfo *errorOut);

    /**
     * Handle input event.
     *
     * @param[in] self global input logic handle
     * @param[in] e    input event
     * @return         @c true if @a e was handled, @c false otherwise
     **/
    bool (*handleInputEvent)(QBGlobalInputLogic self,
                             const QBInputEvent *e);

    /**
     * Update main menu position.
     *
     * @param[in] self global input logic handle
     **/
    void (*updateMainMenuPosition)(QBGlobalInputLogic self);
} *QBGlobalInputLogicVTable;

/**
 * Create an instance of global input logic using @ref QBLogicFactory.
 *
 * @public @memberof QBGlobalInputLogic
 *
 * @param[in]  appGlobals CubiTV application state
 * @param[out] errorOut   error info
 * @return                global input logic instance,
 *                        or @c NULL in case of error
 **/
QBGlobalInputLogic
QBGlobalInputLogicCreate(AppGlobals appGlobals,
                         SvErrorInfo *errorOut);

/**
 * Handle input event.
 *
 * @public @memberof QBGlobalInputLogic
 *
 * @param[in] self global input logic handle
 * @param[in] e    input event
 * @return         @c true if @a e was handled, @c false otherwise
 **/
static inline bool
QBGlobalInputLogicHandleInputEvent(QBGlobalInputLogic self,
                                   const QBInputEvent *e)
{
    return SvInvokeVirtual(QBGlobalInputLogic, self, handleInputEvent, e);
}

/**
 * Update main menu and all submenus position.
 *
 * @public @memberof QBGlobalInputLogic
 *
 * @param[in] self handle to global input logic
 **/
static inline void
QBGlobalInputLogicUpdateMainMenuPosition(QBGlobalInputLogic self)
{
    return SvInvokeVirtual(QBGlobalInputLogic, self, updateMainMenuPosition);
}

/**
 * @relates QBGlobalInputLogic
 *
 * Get runtime type identification object representing the GlobalInputLogic class.
 *
 * @return GlobalInputLogic type identification object
 **/
SvType
QBGlobalInputLogic_getType(void);

/**
 * @}
 **/

#endif // #ifndef QB_GLOBAL_INPUT_LOGIC_H
