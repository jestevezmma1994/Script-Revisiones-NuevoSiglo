/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_FSM_H
#define QB_FSM_H

/**
 * @file QBFsm.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 * @brief Finite State Machine implementation
 **/

#include <QBAppKit/QBFsmDefs.h>
#include <SvFoundation/SvString.h>
#include <stdbool.h>

/**
 * @defgroup QBFsm Finite State Machine Implementation that allows registering callback functions upon state entry, state leave and state transitions
 * @ingroup QBAppKit
 * @{
 */

/**
 * Create new empty finite state machine
 *
 * @return empty QBFsm object
 **/
QBFsm QBFsmCreate(void);

/**
 * Register new state in FSM.
 *
 * @param[in] self - this FSM
 * @param[in] stateName - name of created state
 * @param[in] stepInCallback - callback function which will be called each time FSM enters this state
 * @param[in] stepInArg - callback function parameter that will be passed back to stepInCallback upon call
 * @param[in] stepOutCallback - callback function which will be called each time FSM leaves this state
 * @param[in] stepOutArg - callback function parameter that will be passed back to stepOutCallback upon call
 *
 **/
void QBFsmAddState(QBFsm self,
                   const char *stateName,
                   QBFsmCallbackFn stepInCallback,
                   void *stepInArg,
                   QBFsmCallbackFn stepOutCallback,
                   void *stepOutArg);

/**
 * Change state callbacks.
 *
 * @param[in] self - this FSM
 * @param[in] stateName - name of created state
 * @param[in] stepInCallback - callback function which will be called each time FSM enters this state
 * @param[in] stepInArg - callback function parameter that will be passed back to stepInCallback upon call
 * @param[in] stepOutCallback - callback function which will be called each time FSM leaves this state
 * @param[in] stepOutArg - callback function parameter that will be passed back to stepOutCallback upon call
 **/
void QBFsmSetStateCallbacks(QBFsm self,
                            const char *stateName,
                            QBFsmCallbackFn stepInCallback,
                            void *stepInArg,
                            QBFsmCallbackFn stepOutCallback,
                            void *stepOutArg);

/**
 * Register new transition between states in a FSM
 *
 * @param[in] self - this FSM
 * @param[in] transitionSignal - signal which will trigger transition between states
 * @param[in] beginStateName - name of state registered in the machine that the transition will be leading out of
 * @param[in] endStateName - name of state registered in the machine that the transition will be leading into
 * @param[in] transitionCallback - callback function which will be called each time FSM performs this transition
 * @param[in] transitionArg - callback function parameter that will be passed back to transitionCallback upon call
 *
 **/
void QBFsmAddTransition(QBFsm self,
                        int transitionSignal,
                        const char *beginStateName,
                        const char *endStateName,
                        QBFsmCallbackFn transitionCallback,
                        void *transitionArg);

/**
 * Paste copy of another FSM (template) into this FSM.
 * First we create a template FSM, then we create a concrete FSM
 * and instantiate a template inside concrete FSM as many times
 * as we want (by copying it with this function).
 *
 * It's worth noting, that template FSM is just another FSM.
 * It is created just like any other FSM. The difference between
 * concrete FSM and template FSM is that we do not intend to
 * use template FSMs. We create them only to copy then into other
 * FSMs (possibly multiple times) and then destroy them.
 *
 * The effect of this function is best described on an example.
 * Lets say we have concrete FSM cfsm, with states (SA, SB and SC)
 * and transitions (1 and 2) as follows:
 *
 * SA --1--> SB --2--> SC
 *
 * Lets also assume we have template FSM tfsm, with states and transitions as follows:
 *
 * TA --1--> TB --2--> TC
 *
 * if we now call
 * QBFsmAddFsmCopyAsState(cfsm, tfsm, "Instance1/");
 * QBFsmAddFsmCopyAsState(cfsm, tfsm, "Instance2/");
 *
 * tfsm will not be modified and cfsm will look like this:
 *
 * SA --1--> SB --2--> SC
 * Instance1/TA --1--> Instance1/TB --2--> Instance1/TC
 * Instance2/TA --1--> Instance2/TB --2--> Instance2/TC
 *
 * Notice that pasting template FSM does not rewire anything and
 * it does not create any new connections. Thus one may want to
 * call
 * QBFsmAddTransition(cfsm, 3, "SC", "Instance1/TA", NULL, NULL);
 * QBFsmAddTransition(cfsm, 4, "Instance1/TC", "Instance2/TA", NULL, NULL);
 * in order to obtain a nice chain FSM like this:
 * SA --1--> SB --2--> SC --3--> Instance1/TA --1--> Instance1/TB --2--> Instance1/TC --4--> Instance2/TA --1--> Instance2/TB --2--> Instance2/TC
 *
 *
 * @param[in] self - this FSM
 * @param[in] template - another FSM whose copy we want to insert into this FSM
 * @param[in] stateName - prefix which will be prepended to each state of template upon copying.
 *
 **/
void QBFsmAddFsmCopyAsState(QBFsm self,
                            QBFsm template,
                            const char *stateName);

/**
 * Set starting state for the FSM. Notice that this function must be called
 * before any signal processing by the FSM can be performed. Notice also that
 * if the start state have an In callback registered this callback will be called by this function
 *
 *  @param[in] self - this FSM
 *  @param[in] stateName - name of a state that will be marked as start state.
 */
void QBFsmSetStartState(QBFsm self,
                        const char *stateName);

/**
 * Pass a signal to the FSM for processing. If the signal causes FSM to transit
 * from state A to state B via transition T then callbacks order will be:
 * - A out callback
 * - T callback
 * - B in callback
 *
 * WARNING! callbacks are called AFTER the transition happened, so A out callback is called when
 * FSM is in state B. This prevents ambiguity of signal processing by the FSM.
 *
 * WARNING! callbacks are allowed to pass signals to FSM making it transit some more.
 * Therefore it is not safe to assume that when a state B "inCallback" is called, the FSM is in state B.
 * Notice that the FSM could transit already to some other state because transition T callback passed another signal.
 * However the callbacks order is guaranteed to be preserved, so if we have FSM like this:
 *
 * A --1--> B --2--> C
 *
 * The FSM is currently in state A, transition 1 callback is passing signal 2 to the FSM, and we start
 * by passing signal 1 to the FSM then the callbacks order will be as follows:
 *
 * FSM is in state B
 * - A outCallback
 * - 1 callback (which passes signal 2 to the FSM for processing)
 * FSM is in state C
 * - B inCallback
 * - B outCallback
 * - 2 callback
 * - C inCallback
 *
 * @param[in] self - this FSM
 * @param[in] transitionSignal - signal to be processed by the machine
 * @return true if the signal caused transition and false otherwise.
 */
bool QBFsmProcessSignal(QBFsm self,
                        int transitionSignal);

/**
 * get the name of state the FSM is currently in
 *
 * @param[in] self - this FSM
 * @return name of current state
 */
SvString QBFsmGetCurrentState(QBFsm self);

/**
 * @}
 **/

#endif //QB_FSM_H
