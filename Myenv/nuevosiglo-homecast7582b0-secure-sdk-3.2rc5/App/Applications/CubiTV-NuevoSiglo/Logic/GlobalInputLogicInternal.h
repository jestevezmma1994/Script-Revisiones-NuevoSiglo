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

#ifndef GLOBALINPUTLOGICINTERNAL_H_
#define GLOBALINPUTLOGICINTERNAL_H_

/**
 * @file GlobalInputLogicInternal.h Protected API for global input logic
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <Logic/GlobalInputLogic.h>

#include <Widgets/countdownDialog.h>
#include <QBApplicationController.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvCore/SvErrorInfo.h>

#if HomecastFactory == 1
#include <Utils/magicCode.h>
#endif

/**
 * @addtogroup GlobalInputLogic
 * @{
 **/

/**
 * GlobalInputLogic class internals.
 * @protected
 **/
struct QBGlobalInputLogic_ {
    /**
     * super class
     **/
    struct SvObject_ super_;

    /**
     * CubiTV application state
     **/
    AppGlobals appGlobals;

    /**
     * standby dialog
     **/
    QBCountdownDialog standbyDialog;

    /**
     * popup timeout in seconds
     **/
    unsigned int popupTimeout;

    /**
     * time in seconds when stb go to standby
     **/
    unsigned int keyPowerTimeout;

#if HomecastFactory == 1
    /**
     * factory magic code
     **/
    QBMagicCode *factoryMagicCode;
#endif

    /**
     * QBApplication Controller
     **/
    QBApplicationController applicationController;
    
#if HomecastFactory == 1
    QBMagicCode* hdcp_disabling_code; // AMERELES BEGIN #1853 Desactivación a demanda y por STB de la protección HDCP
#endif
};

/**
 * Handle power button.
 *
 * @protected @memberof QBGlobalInputLogic
 *
 * @param[in] self global input logic handle
 * @param[in] e    input event
 * @return         @c true if @a e was handled, @c false otherwise
 *
 **/
bool
QBGlobalInputLogicHandlePowerButton(QBGlobalInputLogic self,
                                    const QBInputEvent *e);

/**
 * Handle homecast factory.
 *
 * @protected @memberof QBGlobalInputLogic
 *
 * @param[in] self global input logic handle
 * @param[in] e    input event
 * @return         @c true if @a e was handled, @c false otherwise
 *
 **/
bool
QBGlobalInputLogicHandleHomecastFactory(QBGlobalInputLogic self,
                                        const QBInputEvent *e);

/**
 * Handle EPG key.
 *
 * @protected @memberof QBGlobalInputLogic
 *
 * @param[in] self global input logic handle
 * @param[in] e    input event
 * @return         @c true if @a e was handled, @c false otherwise
 *
 **/
bool
QBGlobalInputLogicHandleEPGKey(QBGlobalInputLogic self,
                               const QBInputEvent *e);


/**
 * @}
 **/

#endif /* GLOBALINPUTLOGICINTERNAL_H_ */
