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

#ifndef CUIT_EVENT_H_
#define CUIT_EVENT_H_

/**
 * @file SMP/CUIT/event.h CUIT events
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <QBInput/QBInputEvent.h>
#include <CAGE/Core/Sv2DRect.h>
#include <CUIT/Core/types.h>
#include <CUIT/Core/effect.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvEvent CUIT events
 * @ingroup CUIT
 * @{
 */

/**
 * Event types handled by CUIT.
 **/
typedef enum {
  SV_EVENT_INPUT,  ///< generated on user input (remote control, keyboard etc.)
  SV_EVENT_QUIT,   ///< generated to quit application
  SV_EVENT_FOCUS,  ///< generated when widget or window got/lost focus
  SV_EVENT_USER,   ///< user defined event
  SV_EVENT_EFFECT, ///< generated on the end of effect
  SV_EVENT_TIMER   ///< generated on timer tick (see svAppTimerStart())
} SvEventType;


/**
 * Translates SvEventType enum values into strings. The return value cannot be freed.
 * \param[in] type  value to be translated
 * \return string represeting type
 **/
extern const char *
svEventGetTypeName(SvEventType type);

/**
 * Input event type.
 **/
struct SvInputEvent_t {
  /// code of character, valid only if type is QBInputEventType_keyTyped, @c 0 otherwise
  uint16_t ch;
  /// type of input event, one of QBInputEventType_* values
  int16_t type;
  /// full input event
  QBInputEvent fullInfo;
};

/**
 * Focus event kind.
 **/
typedef enum {
  SvFocusEventKind_GET,   ///< widget got focus
  SvFocusEventKind_LOST,  ///< widget lost focus
  SvFocusEventKind_IN,    ///< child widget got focus (see @ref svWidgetSetFocusMonitor)
  SvFocusEventKind_OUT,   ///< child widget lost focus (see @ref svWidgetSetFocusMonitor)
} SvFocusEventKind;

/**
 * Focus event type.
 **/
struct SvFocusEvent_t {
  SvFocusEventKind kind; ///< kind of the focus event
  bool window; ///< @c true if focus was passed to a window, @c false if passed to a widget
  Sv2DRect prev_focus_pos; ///< position of the previously focused widget relative to widget that receives this event
  size_t parents_cnt; ///< size of parents array
  /// parents of widget which just got or lost focus
  /// (up to the first child of the first common ancestor of both of them)
  /// @note the first parent is available at index @c 0 and the last at <tt>parents_cnt - 1</tt>
  SvWidget parents[];
};

/**
 * User event type.
 *
 * User event may be routed statically (with target specified) or
 * dynamically (if target == @c 0) based on the widget event registration.
 */
struct SvUserEvent_t {
  uint32_t code;   ///< code of event
  void* prv; ///< private data of event
  void (*clean)(void*); ///<   function wich will be called when destroing event. It should free \a prv. If not set, and \a prv is set, it will be removed using standard \a free call.
};

/**
 * Effect event type.
 *
 * This kind of event is sent to target widget when effect is terminated
 * either by canceling or it's duration passes naturally.
 **/
struct SvEffectEvent_t {
  SvEffectId id; ///< id of affected effect
  SvEffectCompletionType reason; ///< completion information
  bool isPermanent; ///< @c true for permanent effects, @c false otherwise
};

/**
 * Timer event type.
 **/
struct SvTimerEvent_t {
  SvTimerId id;  ///< id of affected timer
  double t;      ///< value of timer when the event was triggered
};


struct SvEvent_t {
  /// actual type of event
  SvEventType type;
  /// sender widget ID, @c 0 if unknown or generated by CUIT itself
  SvWidgetId sender;
  /// target widget ID, @c 0 for events sent to widgets registered via svAppEventRegister()
  SvWidgetId target;
  /// previous event on the list
  SvEvent prev;
  /// next event on the list
  SvEvent next;
  /// actual information carried by the event
  union {
    /// input event information
    struct SvInputEvent_t input;
    /// focus event information
    struct SvFocusEvent_t focus;
    /// user event information
    struct SvUserEvent_t user;
    /// effect event information
    struct SvEffectEvent_t effect;
    /// timer event information
    struct SvTimerEvent_t timer;
  } ev;
};


/**
 * Creates a new input event that represents that the user
 * has pressed a key.
 * \param[in] ch   key that has been pressed
 * \return instance of SvEvent representing a input event
 **/
extern SvEvent
svInputEventCreate(int ch);

/**
 * Extends svInputEventCreate() by providing more
 * detailed information about pressed key. That includes information
 * whether the key has been typed/pressed/released and which device
 * has provided it.
 * \param[in] ke    structure desribing details of key event
 * \return instance of SvEvent representing a input event
 **/
extern SvEvent
svInputEventCreateExtended(QBInputEvent ke);

/**
 * Creates a new focus event used to notify widgets about change of their
 * focus status or change of focus status of their children. Focus status
 * is described by SvFocusEventKind.
 *
 * \param[in] kind          focus event kind
 * \param[in] window        true iff target is a window
 * \param[in] target        widget whose focus status has changed
 *                          and will be notified
 * \param[in] parents       parents of widget which just got or lost focus,
 *                          set to @c NULL when event kind is @ref SvFocusEventKind_GET
 *                          or @ref SvFocusEventKind_LOST
 * \param[in] parents_cnt   size of parents array
 * \return instance of SvEvent representing a focus event
 **/
extern SvEvent
svFocusEventCreate(SvFocusEventKind kind, bool window, SvWidgetId target,
                   SvWidget *parents, size_t parents_cnt);

/**
 * Creates a new timer event used to notify a widget that timer has fired.
 * \param[in] id        the timer that fired
 * \param[in] t         how much time passed since timer has been created
 * \param[in] target    which widget should be notified
 * \return SvEvent instance representing a timer fired event.
 **/
extern SvEvent
svTimerEventCreate(SvTimerId id, double t, SvWidgetId target);

/**
 * Creates a new effect event used to notify about effect completion.
 * \param[in] id            effect that has been completed
 * \param[in] reason        reason/how an effect has been completed
 * \param[in] target        widget that will be notified
 * \param[in] isPermanent   @c true for permanent effects, @c false otherwise
 * \return SvEvent instance representing a effect event
**/
extern SvEvent
svEffectEventCreate(SvEffectId id, SvEffectCompletionType reason, SvWidgetId target, bool isPermanent);

/**
 * Creates a new effect representing SvApplication quit.
 * \return SvEvent instance representing quit event.
**/
extern SvEvent
svQuitEventCreate(void);


/**
 * Create user event.
 *
 * \param[in] code code of the event
 * \param[in] target id of target widget (may be \a 0, event will be delivered to all widgets that are registered)
 * \param[in] prv private data of event
 * \param[in] clean cleanup function. If not specified, the prv will be freed ba standard \a free call.
 * \return created event
*/
extern SvEvent
svUserEventCreate(uint32_t code, SvWidgetId target,
                  void *prv, void (*clean)(void *));

/**
 * Destroy an event.
 *
 * \param[in] event event to be destroyed
 */
extern void
svEventDestroy(SvEvent event);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif